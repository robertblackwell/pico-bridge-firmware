#include <stdio.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include <pico/unique_id.h>
#include "hardware/gpio.h"

int count = 0;
const int LED_PIN = 25;
char unique_id[8];
const int buffer_max_len = 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 2;
char uid_buffer[buffer_max_len];

void printf_unique_id(char* buffer, uint8_t* uid, int len) 
{
    int blen = 0;
    for(int ix = 0; ix < len; ix++){
        blen = blen + sprintf(buffer, "%2x", uid[ix]);
    } 
    buffer[blen] = '\0';

}
int main()
{
    
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    pico_get_unique_board_id_string(uid_buffer, buffer_max_len);
    while(1) {
        count++;
        gpio_put(LED_PIN, 0);
        sleep_ms(100);
        gpio_put(LED_PIN,1);
        printf("Hello world count: %d board_id %s \n", count, uid_buffer);
        sleep_ms(100);
    }
}