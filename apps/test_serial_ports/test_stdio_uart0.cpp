#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include "trace.h"

#include "hardware/gpio.h"
#define UART_ID uart0
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1

int count = 0;

int main()
{
    // uart_init(UART_ID, UART_BAUD_RATE);
    // gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    stdio_init_all();
    sleep_ms(3000);

    while(1) {
        count++;
        sleep_ms(1000);
        printf("This is stdio via uart0 %d\n\r", count);
    }
    // #endif
}