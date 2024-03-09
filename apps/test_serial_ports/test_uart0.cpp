#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include "trace.h"

#include "hardware/gpio.h"

int count = 0;

#define UART_BAUD_RATE 115200
#if 1
    #define UART_ID uart1
    #define UART_TX_PIN 8
    #define UART_RX_PIN 9
#else
    #define UART_ID uart0
    #define UART_TX_PIN 16
    #define UART_RX_PIN 17
#endif

int main()
{
    stdio_init_all();
    printf("test_uart0.cpp  starting \n");
    sleep_ms(3000);
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    sleep_ms(3000);

    while(1) {
        count++;
        sleep_ms(1000);
        char buffer[512];
        sprintf(buffer, "uart_puts(uart on 16,17) %d\n", count);
        uart_puts(UART_ID, buffer);
        printf("This is stdio %d\n", count);
    }
    // #endif
}