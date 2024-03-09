#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include "trace.h"

#include "hardware/gpio.h"

int count = 0;

int main()
{
    stdio_init_all();
    sleep_ms(3000);

    while(1) {
        count++;
        sleep_ms(1000);
        printf("This is stdio via usb %d\n", count);
    }
}