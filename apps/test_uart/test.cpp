#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "pico/stdlib.h"
#include <pico/types.h>
#include <pico/unique_id.h>
#include <pico/time.h>
#include "hardware/gpio.h"

#if 0
#define UART_ID uart0
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 0
#define UART_RX_PIN 1
#endif
#if 0
#define UART_ID uart1
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 8
#define UART_RX_PIN 9
#endif
#define UART_ID uart0
#define UART_BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17

#undef RECEIVER_VERBOSE
#undef RECV_STATS

int count = 0;

int  recv_line_count = 0;
bool recv_first_char = true;
int  recv_buffer_len;
long recv_total_chars;
long recv_interval_line;
uint64_t recv_interval_total = 0;
absolute_time_t recv_start_time;
absolute_time_t recv_end_time;
#ifdef RECV_STATS
#endif

const int LED_PIN = 25;
char unique_id[8];
const int buffer_max_len = 2 * PICO_UNIQUE_BOARD_ID_SIZE_BYTES + 2;
char uid_buffer[buffer_max_len];
char out_buffer[512];
int out_buffer_len;
char in_buffer[512];
int in_buffer_len;
bool iam_sender;
void printf_unique_id(char* buffer, uint8_t* uid, int len) 
{
    int blen = 0;
    for(int ix = 0; ix < len; ix++){
        blen = blen + sprintf(buffer, "%2x", uid[ix]);
    } 
    buffer[blen] = '\0';

}
#undef SENDER_VERBOSE

void sender()
{
    count++;
    int len = sprintf(out_buffer, "From sender count: %d my board_id %s \n", count, uid_buffer);
    out_buffer[len] = '\0';
    uart_puts(UART_ID, out_buffer);
    gpio_put(LED_PIN, 0);
    // sleep_ms(100);
    gpio_put(LED_PIN,1);
    #ifdef SENDER_VERBOSE
    printf("Sender msg len: %d count: %d my board_id %s \n", len, count, uid_buffer);
    sleep_ms(100);
    #else
    printf(".");
    if(count % 30 == 0) {
        printf("\n");
    }
    #endif
}
void receiver()
{
    while(1) {
        in_buffer_len = 0;
        while(1) {
            while(uart_is_readable(UART_ID)) {
                #if defined RECV_STATS || 1
                if(recv_first_char) {
                    recv_start_time = get_absolute_time();
                    recv_first_char = false;
                }
                #endif
                char ch = uart_getc(UART_ID);
                // printf("got a character %d\n", (int)ch);
                if(ch == '\n') {
                    recv_line_count++;
                    #if defined  RECV_STATS || 1
                    recv_buffer_len = in_buffer_len+1;
                    recv_total_chars = recv_total_chars + in_buffer_len+1;
                    // printf("get_absolute_time\n");
                    absolute_time_t stop_time = get_absolute_time();
                    // printf("before now\n");
                    auto now = to_us_since_boot(get_absolute_time());
                    recv_end_time._private_us_since_boot = now;
                    // printf("now %llu\n", now);
                    auto then = to_us_since_boot(recv_start_time);
                    // printf("then %llu\n", now);
                    uint64_t d = to_us_since_boot(get_absolute_time()) - to_us_since_boot(recv_start_time);
                    recv_interval_total = recv_interval_total + d;
                    #endif
                    #ifdef RECIEVER_VERBOSE
                    printf("Receiver got a line [%s]\n", in_buffer);
                    #else
                        if(recv_line_count % 30 == 0) 
                            printf(".\n");
                        else
                            printf(".");
                    #endif
                    if(recv_line_count > 20000) {
                    #if defined RECV_STATS || 1
                         {
                            float elapsed = (double)to_ms_since_boot(recv_end_time) - (double)to_ms_since_boot(recv_start_time);
                            printf("Statistics line_count: %d\n", recv_line_count);
                            printf("   Start time                  : %f\n", (float)to_ms_since_boot(recv_start_time));
                            printf("   End time                    : %f\n", (float)to_ms_since_boot(recv_end_time));
                            printf("   Total interval in micro secs: %f\n", elapsed);
                            printf("   Total interval in secs      : %f\n", elapsed / 1000.0);
                            printf("   Total characters received   : %f\n", (float)(recv_total_chars));
                            printf("   Overall data rate in ch/s   : %f\n", ((float)(recv_total_chars)/(elapsed/1000.0)));
                            while(1){;}
                        }
                    #else
                        while(1) {
                            printf("recv line count");
                            sleep_ms(2000);
                        }
                    #endif
                    }
                    in_buffer_len = 0;
                    #ifdef RECV_STATS
                    recv_start_time = get_absolute_time();
                    #endif
                } else if(ch == '\r') {
                    ;
                } else {
                    in_buffer[in_buffer_len] = ch;
                    in_buffer[in_buffer_len + 1] = '\0';
                    in_buffer_len++;
                }
            }
        }
    }
}
int main()
{
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    sleep_ms(2000);
    uart_init(UART_ID, UART_BAUD_RATE);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    pico_get_unique_board_id_string(uid_buffer, buffer_max_len);
    iam_sender = (strcmp(uid_buffer, "E6614104036E8D32") == 0);
    printf("uid_buffer is : [%s] iam_%s \n", uid_buffer, (iam_sender)? " sender ": " receiver ");
    sleep_ms(5000);
    printf("after sleep uid_buffer is : [%s] iam_sender: %d \n", uid_buffer, iam_sender);
    while(1) {
        #if 0
        sleep_ms(1000);
        printf("Looping\n");
        #else
        if(iam_sender) {
            // printf("I am sender count: %d uid: %s\n", count, uid_buffer);
            // sleep_ms(5);
            sender();
        } else {
            // printf("I am receiver count: %duid: %s\n", count, uid_buffer);
            // sleep_ms(500);
            receiver();
        }
        #endif
    }
}