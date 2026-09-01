#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "hardware/clocks.h"
#include "hardware/gpio.h"
#include "pico/critical_section.h"
#include "pico/multicore.h"
#include "pico/stdlib.h"
#include "pio_usb.h"
#include "tusb.h"

#define HOST_PIN_DP 8
#define PULSE_MS 150
#define QUEUE_DEPTH 32

static const uint GPIO_BY_INPUT[4] = {0, 2, 4, 6};

typedef struct {
    hid_keyboard_report_t report;
} keyboard_event_t;

static keyboard_event_t report_queue[QUEUE_DEPTH];
static uint8_t queue_head = 0;
static uint8_t queue_tail = 0;
static critical_section_t state_cs;
static uint8_t pulse_request = 0;
static bool hotkey_latched = false;

static uint8_t queue_next(uint8_t value) {
    return (uint8_t)((value + 1u) % QUEUE_DEPTH);
}

static void queue_clear(void) {
    critical_section_enter_blocking(&state_cs);
    queue_head = 0;
    queue_tail = 0;
    critical_section_exit(&state_cs);
}

static void queue_report(hid_keyboard_report_t const *report) {
    critical_section_enter_blocking(&state_cs);
    uint8_t next = queue_next(queue_head);
    if (next != queue_tail) {
        report_queue[queue_head].report = *report;
        queue_head = next;
    }
    critical_section_exit(&state_cs);
}

static bool pop_report(hid_keyboard_report_t *report) {
    bool available = false;
    critical_section_enter_blocking(&state_cs);
    if (queue_tail != queue_head) {
        *report = report_queue[queue_tail].report;
        queue_tail = queue_next(queue_tail);
        available = true;
    }
    critical_section_exit(&state_cs);
    return available;
}

static void request_input(uint8_t input) {
    critical_section_enter_blocking(&state_cs);
    if (pulse_request == 0) {
        pulse_request = input;
    }
    critical_section_exit(&state_cs);
}

static bool report_has_key(hid_keyboard_report_t const *report, uint8_t keycode) {
    for (uint8_t i = 0; i < 6; ++i) {
        if (report->keycode[i] == keycode) return true;
    }
    return false;
}

static uint8_t hotkey_input(hid_keyboard_report_t const *report) {
    bool ctrl = (report->modifier & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL)) != 0;
    bool alt = (report->modifier & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT)) != 0;
    if (!ctrl || !alt) return 0;

    if (report_has_key(report, HID_KEY_1)) return 1;
    if (report_has_key(report, HID_KEY_2)) return 2;
    if (report_has_key(report, HID_KEY_3)) return 3;
    if (report_has_key(report, HID_KEY_4)) return 4;
    return 0;
}

static bool target_keys_down(hid_keyboard_report_t const *report) {
    return report_has_key(report, HID_KEY_1) || report_has_key(report, HID_KEY_2) ||
           report_has_key(report, HID_KEY_3) || report_has_key(report, HID_KEY_4);
}

static void process_keyboard_report(hid_keyboard_report_t const *report) {
    uint8_t input = hotkey_input(report);

    if (!hotkey_latched && input != 0) {
        hid_keyboard_report_t released = {0};
        queue_report(&released);
        request_input(input);
        hotkey_latched = true;
        return;
    }

    if (hotkey_latched) {
        bool ctrl = (report->modifier & (KEYBOARD_MODIFIER_LEFTCTRL | KEYBOARD_MODIFIER_RIGHTCTRL)) != 0;
        bool alt = (report->modifier & (KEYBOARD_MODIFIER_LEFTALT | KEYBOARD_MODIFIER_RIGHTALT)) != 0;
        if (!ctrl && !alt && !target_keys_down(report)) {
            hotkey_latched = false;
        }
        hid_keyboard_report_t released = {0};
        queue_report(&released);
        return;
    }

    queue_report(report);
}

static void core1_main(void) {
    sleep_ms(10);

    pio_usb_configuration_t pio_cfg = PIO_USB_DEFAULT_CONFIG;
    pio_cfg.pin_dp = HOST_PIN_DP;
    tuh_configure(1, TUH_CFGID_RPI_PIO_USB_CONFIGURATION, &pio_cfg);
    tuh_init(1);

    while (true) {
        tuh_task();
    }
}

static void service_kvm_pulse(void) {
    static uint8_t active_input = 0;
    static absolute_time_t deadline;

    if (active_input != 0) {
        if (time_reached(deadline)) {
            gpio_put(GPIO_BY_INPUT[active_input - 1], 0);
            active_input = 0;
        }
        return;
    }

    uint8_t requested = 0;
    critical_section_enter_blocking(&state_cs);
    if (pulse_request != 0) {
        requested = pulse_request;
        pulse_request = 0;
    }
    critical_section_exit(&state_cs);

    if (requested >= 1 && requested <= 4) {
        gpio_put(GPIO_BY_INPUT[requested - 1], 1);
        active_input = requested;
        deadline = make_timeout_time_ms(PULSE_MS);
    }
}

int main(void) {
    set_sys_clock_khz(120000, true);
    critical_section_init(&state_cs);

    for (int i = 0; i < 4; ++i) {
        uint gpio = GPIO_BY_INPUT[i];
        gpio_init(gpio);
        gpio_put(gpio, 0);
        gpio_set_drive_strength(gpio, GPIO_DRIVE_STRENGTH_12MA);
        gpio_set_dir(gpio, GPIO_OUT);
    }

    multicore_reset_core1();
    multicore_launch_core1(core1_main);

    tud_init(0);

    while (true) {
        tud_task();
        service_kvm_pulse();

        if (tud_hid_ready()) {
            hid_keyboard_report_t report;
            if (pop_report(&report)) {
                tud_hid_keyboard_report(0, report.modifier, report.keycode);
            }
        }
    }
}

void tud_mount_cb(void) {
    queue_clear();
    hid_keyboard_report_t released = {0};
    queue_report(&released);
}

void tud_umount_cb(void) {
    queue_clear();
}

void tuh_hid_mount_cb(uint8_t dev_addr, uint8_t instance,
                      uint8_t const *desc_report, uint16_t desc_len) {
    (void)desc_report;
    (void)desc_len;

    if (tuh_hid_interface_protocol(dev_addr, instance) == HID_ITF_PROTOCOL_KEYBOARD) {
        tuh_hid_receive_report(dev_addr, instance);
    }
}

void tuh_hid_umount_cb(uint8_t dev_addr, uint8_t instance) {
    (void)dev_addr;
    (void)instance;
}

void tuh_hid_report_received_cb(uint8_t dev_addr, uint8_t instance,
                                uint8_t const *report, uint16_t len) {
    if (tuh_hid_interface_protocol(dev_addr, instance) == HID_ITF_PROTOCOL_KEYBOARD &&
        len == sizeof(hid_keyboard_report_t)) {
        process_keyboard_report((hid_keyboard_report_t const *)report);
    }

    tuh_hid_receive_report(dev_addr, instance);
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id,
                               hid_report_type_t report_type,
                               uint8_t *buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id,
                           hid_report_type_t report_type,
                           uint8_t const *buffer, uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)bufsize;
}
