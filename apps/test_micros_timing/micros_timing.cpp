#undef FTRACE_ON
#include <functional>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5
#include "pico/stdlib.h"
#include <pico/error.h>

using namespace transport::buffer;

int main()
{

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	sleep_ms(5000);
	printf("stdio started\n");
	while (1)
	{
        int n = 1000000;
        auto start_m = micros();
        for(int i = 0; i < n; i++) {
            auto m = micros();
        }
        auto et = micros() - start_m;
        printf("loop of %d elapsed time micro-secs: %llu millsecs: %f secs: %f average: %10.10f micro-secs\n",
                  n, et, (float)et / 1000.00, ((float)et/1000000.0), ((float)(et)) / ((float)(n)));
	}
}
