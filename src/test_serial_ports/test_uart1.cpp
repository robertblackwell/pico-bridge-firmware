#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include "trace.h"

#include "hardware/gpio.h"

int count = 0;
#define UART_ID uart1
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 8
#define UART_RX_PIN 9

int main()
{
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    stdio_init_all();
    sleep_ms(3000);

    while(1) {
        count++;
        sleep_ms(1000);
        char buffer[512];
        sprintf(buffer, "uart_puts(uart1) %d\n", count);
        uart_puts(uart1, buffer);
        printf("This is stdio %d\n", count);
    }
    // #endif
}