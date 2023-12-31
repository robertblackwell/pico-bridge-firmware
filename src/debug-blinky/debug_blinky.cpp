#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include <pico/unique_id.h>
#include "hardware/gpio.h"

int count = 0;
const int LED_PIN = 25;
int main()
{
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    while(1) {
        count++;
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        gpio_put(LED_PIN,1);
        sleep_ms(100);
    }
}
