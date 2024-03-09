#include <stdio.h>
#include <string.h>
#include "pico/stdlib.h"
#include <pico/error.h>
#include "hardware/gpio.h"

static bool get_char_if_available(int* char_received) {
    int ch = getchar_timeout_us(0);
    if(ch == PICO_ERROR_TIMEOUT) {
        return false;
    }
    *char_received = ch;
    return true;
}

#define MAX_MESSAGE 256
const int LED_PIN = 25;

class Reader 
{
    public:
    char    msg_buffer[MAX_MESSAGE];
    int     msg_position = 0;
    bool    msg_available;
    Reader() {
        
        msg_position = 0;
        msg_available = false;
    }
    void run() {
        if (!msg_available) {
            int chin;
            while(get_char_if_available(&chin)) {
                char in_ch = (char)chin;
                if((in_ch == '\n')||(in_ch == '\r')) {
                    msg_buffer[msg_position] = '\0';
                    msg_available = true;
                    break;
                }
                msg_buffer[msg_position] = in_ch;
                msg_position ++;
                msg_buffer[msg_position] = '\0';
            }
        }
    }

    bool available() {
        return msg_available;
    }
    void consume(char* buffer, int len) {
        strncpy(buffer, msg_buffer, (len>MAX_MESSAGE)?MAX_MESSAGE: len);
        msg_position = 0;
        msg_available = false;
    }
};

int main()
{
    static char message[MAX_MESSAGE];
    static int msg_position = 0;
    Reader reader;
    stdio_init_all();
    sleep_ms(5000);
    printf("Ready to start\n");
    while(true) {
        sleep_ms(500);
        msg_position = 0;
        reader.run();
        if(reader.available()) {
            reader.consume(message, MAX_MESSAGE);
            printf("There was a message : [%s]\n", message);
        }
    }
}
