#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#define FTRACE_ON
#include <trace.h>
#include "transport/transport.h"

int count = 0;
const int LED_PIN = 25;

transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();
    void* x = malloc(120);
    gpio_init(LED_PIN);
    gpio_set_dir(LED_PIN, GPIO_OUT);
    stdio_init_all();
    trace_init_stdio();
    printf("print_fmt after init %d sizeof(void*): %d\n", 999, sizeof(void*));
    print_fmt("print_fmt after init %d \n", 999);
    
    while(1) {
        // printf("top of while loop reader.m_state: %d\n", treader.m_state);
        treader.run();
        if(treader.available()) {
            printf("inside available loop reader.m_state: %d\n", treader.m_state);
            transport::buffer::Handle bh = treader.borrow_buffer();
            Argv args(bh);
            transport::send_command_ok(
                "This is what treader got [%s]\n", 
                transport::buffer::sb_buffer_as_cstr(bh));
                transport::send_command_ok("command is %s", args.token_at(0));
                for(int i = 1; i < args.token_count; i++) {
                    transport::send_command_ok("      i: %d token: %s", i, args.token_at(i));
                }
            treader.return_buffer(bh);
            printf("leaving if available branch reader.m_state: %d\n", treader.m_state);
        }
        sleep_ms(1000);
        // count++;
        // gpio_put(LED_PIN, 0);
        // sleep_ms(1000);
        // gpio_put(LED_PIN,1);
        // sleep_ms(1000);
		// printf("test_02_command end of loop count :%d sizeof(void*): %d\n", count, sizeof(void*));
        // FTRACE("test_02_command end of loop count :%d\n", count)
    }
}