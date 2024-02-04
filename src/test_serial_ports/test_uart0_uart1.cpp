#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include <pico/stdio_uart.h>
#include "trace.h"

#include "hardware/gpio.h"
#include "hardware/uart.h"
#define UART_01 uart1
#define UART1_BAUD_RATE 115200
#define UART1_TX_PIN 8
#define UART1_RX_PIN 9

#define UART_00 uart0
#define UART0_BAUD_RATE 115200
#define UART0_TX_PIN 16
#define UART0_RX_PIN 17


/**
 * This program tests the used of uart0 as stdio and uart1 for Trace
 * on the robot hardware config
*/

int count = 0;

int main()
{
    stdio_init_all();
    // stdio_uart_init_full(UART_00, 115200, UART0_RX_PIN, UART0_TX_PIN);
    // uart_init(uart0, 115200);
    // gpio_set_function(UART0_RX_PIN, GPIO_FUNC_UART);
    // gpio_set_function(UART0_TX_PIN, GPIO_FUNC_UART);
    uart_init(uart1, UART1_BAUD_RATE);
    gpio_set_function(UART1_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART1_RX_PIN, GPIO_FUNC_UART);

    sleep_ms(3000);

    while(1) {
        count++;
        sleep_ms(1000);
        printf("This is stdio via uart0 %d\n\r", count);
        char buffer[256];
        sprintf(buffer, "Uart1 count: %d\n\r", count);
        uart_puts(UART_01, buffer);
    }
    // #endif
}