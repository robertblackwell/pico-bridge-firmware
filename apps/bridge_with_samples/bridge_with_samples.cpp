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
#include "encoder_v2.h"
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
void do_commands();
void heart_beat();
#include "pico/stdlib.h"
#include <pico/error.h>
#include <cli/execute_commands.h>
#include <version.h>

using namespace transport::buffer;

static bool test_get_char_if_available(int* char_received) {
	int ch = getchar_timeout_us(0);
	if(ch == PICO_ERROR_TIMEOUT) {
		// printf("get_char_if_available: no char timedout\n");
		return false;
	}
	*char_received = ch;
	// printf("get_char_if_available: got ch: %c decimal value of ch %d\n", (char)ch, ch);
	return true;
}

// Cli cli;
transport::Reader treader;
void encoder_samples();
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
//	print_fmt("bridge (version:%s ) starting ... \n", VERSION_NUMBER);
    transport::send_boot_message("bridge_with_samplesmake (version:%s ) starting ... \n", VERSION_NUMBER);
	robot_init();
	Task cli_task(20, do_commands);
	Task heart_beat_task(1000, heart_beat);
	Task collect_samples_task(3000, &encoder_samples);
	robot_start_encoder_sample_collection((uint64_t)1000000);
	while (1)
	{
		cli_task();
		heart_beat_task();
		collect_samples_task();
	}
}
void heart_beat()
{
	printf("Heart beat \n");
}
void encoder_samples() 
{
	Handle h = tx_pool::allocate();
	tojson_encoder_samples(h);
	transport::send_json_response(&h);
}
void do_commands()
{
    treader.run();
    if (treader.available()) {
        Handle bh = treader.borrow_buffer();
        Argv args{};
        if (!args.tokenize(sb_buffer_as_cstr(bh))) {
            printf("Tokenize failed buffer: %s\n", sb_buffer_as_cstr(bh));
        } else {
            FTRACE("This is what treader got [%s]\n", sb_buffer_as_cstr(bh));
            FDUMP_TOKENS(args, "Dump tokens message")
            CommandName enumname = command_lookup(args.token_at(0));
            FTRACE("command_lookup result: [%s]", to_string(enumname))
            execute_commands(args, bh);
        }
		treader.return_buffer(bh);
    }
}
