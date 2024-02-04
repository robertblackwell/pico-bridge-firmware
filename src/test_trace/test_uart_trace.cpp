#define FTRACE_ON
#include <stdio.h>
#include <stdint.h>
#include <pico.h>
#include <pico/types.h>
#include <pico/stdlib.h>
#include <trace.h>

#include "hardware/gpio.h"
void function_01()
{
    FTRACE("This some stuff from function_01 %d\n", 42);
}

int count = 0;
const int LED_PIN = 25;

int main()
{
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    sleep_ms(3000);
    printf("test_uart stdio init called\n");
    // // uart_init(uart1, 115200);
    // // gpio_set_function(8, GPIO_FUNC_UART);
    // // gpio_set_function(9, GPIO_FUNC_UART);
    // // sleep_ms(3000);
    // // uart_puts(uart1, "This is from uart1 before the loop \n\r");
    // while(1) {
    //     count++;
    //     sleep_ms(2000);
    //     printf("Loop %d\n", count);
    //     uart_puts(uart1, "This is from uart1 inside the loop \n\r");
    // }
    // #if 0
    trace_init_full(uart1, 115200, 8, 9);
    sleep_ms(2000);
    print_fmt("print_fmt after init %d uart0: %p uart1: %p\n", 999, uart0, uart1);

    while(1) {
        count++;
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
        gpio_put(LED_PIN,1);
        sleep_ms(1000);
        function_01();
        printf("This is stdio %d\n", count);
        FTRACE("end of loop count :%d\n", count)
    }
    // #endif
}