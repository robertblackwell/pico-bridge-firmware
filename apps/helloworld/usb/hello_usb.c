/**
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>
#include "pico/stdlib.h"

int main() {
    int counter = 0;
    stdio_init_all();
    while (1) {
        printf("Hello, world - from hello_usb.c counter: %d\n", counter++);
        sleep_ms(1000);
    }
}
