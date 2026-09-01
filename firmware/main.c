#include <stdio.h>
#include "pico/stdlib.h"

#define PULSE_MS 150

static const uint GPIO_BY_INPUT[4] = {0, 2, 4, 6};

static void press_input(int input) {
    if (input < 1 || input > 4) {
        return;
    }

    uint gpio = GPIO_BY_INPUT[input - 1];
    gpio_put(gpio, 1);
    sleep_ms(PULSE_MS);
    gpio_put(gpio, 0);
}

int main(void) {
    stdio_init_all();

    for (int i = 0; i < 4; ++i) {
        gpio_init(GPIO_BY_INPUT[i]);
        gpio_set_dir(GPIO_BY_INPUT[i], GPIO_OUT);
        gpio_put(GPIO_BY_INPUT[i], 0);
    }

    sleep_ms(1000);
    printf("KVM-Switcher RP2040 ready\r\n");

    while (true) {
        int ch = getchar_timeout_us(100000);
        if (ch == PICO_ERROR_TIMEOUT) {
            continue;
        }

        if (ch >= '1' && ch <= '4') {
            int input = ch - '0';
            press_input(input);
            printf("OK %d\r\n", input);
        } else if (ch == '?' || ch == 'h' || ch == 'H') {
            printf("Commands: 1 2 3 4\r\n");
        }
    }
}
