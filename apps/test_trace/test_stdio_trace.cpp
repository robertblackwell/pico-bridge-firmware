#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#define FTRACE_ON
#include <trace.h>

void function_01()
{
    FTRACE("This some stuff from function_01 %d", 42);
}

int count = 0;
const int LED_PIN = 25;

int main()
{
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    trace_init_stdio();
    print_fmt("print_fmt after init %d \n", 999);
    while(1) {
        count++;
        gpio_put(LED_PIN, 0);
        sleep_ms(1000);
        gpio_put(LED_PIN,1);
        sleep_ms(1000);
        function_01();
        FTRACE("end of loop count :%d\n", count)
    }
}