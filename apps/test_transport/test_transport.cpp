#define FTRACE_ON
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
void execute_commands();
#include "pico/stdlib.h"
#include <pico/error.h>
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
int count = 0;
// Cli cli;
transport::Reader treader;
void read_input();
void send_periodic();

int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
	printf("stdio started\n");
	Task reader_task(10, &read_input);
	Task sender_task(0, &send_periodic);
	while (1)
	{
		// reader_task.run();
		sender_task.run();	
	}
}
void read_input() 
{
	treader.run();
	if(treader.available()) {
		transport::buffer::Handle bh = treader.borrow_buffer();
		printf("treader got [%s]\n", transport::buffer::sb_buffer_as_cstr(bh));
		// transport::send_command_ok(" - treader got [%s] ", transport::buffer::sb_buffer_as_cstr(bh));
		treader.return_buffer(bh);
	}
}
void send_periodic()
{
	printf("TEST_TRANSPORT THIS IS A MESSAGE FROM PICO aadsad kjlkj iuoiuo ASD ,MN,MN, stdio looping count[%d]\n", count);
	count++;
}