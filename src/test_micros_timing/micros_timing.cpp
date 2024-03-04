#undef FTRACE_ON
#include <functional>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>

#include "trace.h"
#include "dri0002.h"
#include "config.h"
#include "encoder.h"
#include "task.h"
#include "motion.h"
#include "reporter.h"
#include "cli/argv.h"
#include "cli/commands.h"
#include "transport/buffers.h"
#include "transport/transmit_buffer_pool.h"
#include "transport/transport.h"
#include "robot.h"

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5
#include "pico/stdlib.h"
#include <pico/error.h>

using namespace transport::buffer;

transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
	printf("stdio started\n");
	robot_init();
	while (1)
	{
        int n = 1000000;
        auto start_m = micros();
        for(int i = 0; i < n; i++) {
            auto m = micros();
        }
        auto et = micros() - start_m;
        print_fmt("loop of %d elapsed time micro-secs: %llu millsecs: %f secs: %f average: %10.10f micro-secs",
                  n, et, (float)et / 1000.00, ((float)et/1000000.0), ((float)(et)) / ((float)(n)));
	}
}
