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
void heart_beat();
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

// Cli cli;
transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	#if 1
	trace_init();
	#else
	trace_init_full(uart1, 115200, 8, 9);
	#endif
	sleep_ms(5000);
	printf("stdio started\n");
	// print_fmt("bridge starting ... \n");
	#define TEST3
	#ifdef TEST1
	int count = 0;
	while(1) {
		sleep_ms(2000);
		char buffer[256];
		int bufix = 0;
		buffer[bufix] = '\0';
		int ch;
		while(bool x = test_get_char_if_available(&ch)){
			//printf("chars available: %d ch: %c\n", (int)x, (char)ch);
			if(ch == '\n') {
				buffer[bufix++] = '\n';
				buffer[bufix] = '\0';
				bufix = 0;
				printf("chars_available got newline buffer: [%s]", buffer);
			} else {
				buffer[bufix++] = ch;
				buffer[bufix] = '\0';
			}
		}
		printf("THIS IS A MESSAGE FROM PICO aadsad kjlkj iuoiuo ASD ,MN,MN, stdio looping count[%d]\n", count);
		// print_fmt("bridge looping %d ... \n", count);
		// uart_puts(uart1, "this is from uart1 \r\n");
		count++;
	}
	#elif defined(TEST2)
	int count = 0;

	while(1) {
		sleep_ms(200);
		printf("Test 2 \n");
		char buffer[256];
		int bufix = 0;
		buffer[bufix] = '\0';
		int ch;
		treader.run();
		if(treader.available()) {
			transport::buffer::Handle bh = treader.borrow_buffer();
			printf("treader got [%s]\n", transport::buffer::sb_buffer_as_cstr(bh));
			transport::send_command_ok(" - treader got [%s] ", transport::buffer::sb_buffer_as_cstr(bh));
			treader.return_buffer(bh);
		}
		printf("THIS IS A MESSAGE FROM PICO aadsad kjlkj iuoiuo ASD ,MN,MN, stdio looping count[%d]\n", count);
		// print_fmt("bridge looping %d ... \n", count);
		// uart_puts(uart1, "this is from uart1 \r\n");
		count++;
	}
	#else
	robot_init();
	Task cli_task(20, execute_commands);
	Task heart_beat_task(2000, heart_beat);
	Task encoder_samples_task(20, &robot_collect_encoder_samples);
	while (1)
	{
		cli_task();
		heart_beat_task();
		// encoder_samples_task();	
	}
	#endif
}
void heart_beat()
{
	printf("Heart beat \n");
}
void sample_collect(Encoder *left, Encoder *right)
{
	left->run();
	right->run();
}
void execute_commands()
{
	treader.run();
	if (treader.available())
	{
		transport::buffer::Handle bh = treader.borrow_buffer();
		Argv args{transport::buffer::sb_buffer_as_cstr(bh)};
		FTRACE("This is what treader got [%s]\n", transport::buffer::sb_buffer_as_cstr(bh));
		FDUMP_TOKENS(args, "Dump tokens message")
		CommandName enumname = command_lookup(args.token_at(0));
		FTRACE("command_lookup result: [%s]", to_string(enumname))

		#if 1
		FTRACE("switch on command name %s", to_string(enumname))
		switch (enumname) {
			case CommandName::None:
				transport::send_command_error("none");
				break;
			case CommandName::Error:
				transport::send_command_error("Invalid command %s", transport::buffer::sb_buffer_as_cstr(bh));
				break;
			case CommandName::MotorsPwmPercent: {
				double left_pwm, right_pwm;
				if(validate_pwm(args, left_pwm, right_pwm)) {
					robot_set_raw_pwm_percent(left_pwm, right_pwm);
					transport::send_command_ok("MotorPwmPercent");
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			case CommandName::MotorsRpm: {
				double left_rpm, right_rpm;
				if(validate_rpm(args, left_rpm, right_rpm)) {
					printf("%f %f\n", left_rpm, right_rpm);
					//robot_set_rpm(left_rpm, right_rpm);
					transport::send_command_ok("MotorRpmCommand");
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			case CommandName::MotorsHalt: {
				if(validate_encoder_read(args)) {
					printf("%s\n", to_string(enumname));
					robot_stop_all();
					transport::send_command_ok("StopCommand");
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			case CommandName::EncodersRead:
			{
				int number = 1;
				if(validate_encoder_read(args)) {
					printf("%s\n", to_string(enumname));
					robot_request(number);
					transport::buffer::Handle h = transport::buffer::tx_pool::allocate();
					tojson_encoder_samples(h);
					transport::send_json_response(&h);
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			case CommandName::PidArgsUpdate: {
				transport::send_command_ok("PidArgUpdate command not implemented [%s]", transport::buffer::sb_buffer_as_cstr(bh));
				break;
			}
			case CommandName::Echo: {
				if(validate_encoder_read(args)) {
					printf("%s\n", to_string(enumname));
					transport::send_command_ok("Echo command [%s]", transport::buffer::sb_buffer_as_cstr(bh));
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			case CommandName::LoadTest: {
				const char* response_source = "qwertyuiopasdfghjklzxcvbnm1234567890~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:ZXCVBNM<>?";
				int count, response_length, nbr_per_second;
				if(validate_loadtest(args, count, response_length, nbr_per_second)) {
					int interval_ms = (int)(1000.00 / (float)nbr_per_second);
					long start_time = micros();
					for(int i = 0; i < count; i++) {
						long m = micros();
						transport::send_command_ok("Loadtest time: %ld count: %i interval: %d [%s]", m, i, interval_ms, response_source);
						sleep_ms(interval_ms);
					}
					long end_time = micros();
					transport::send_command_ok("Elapsed time %ld micro seconds", (end_time - start_time));
				} else {
					transport::send_command_error("Invalid %s command %s\n", to_string(enumname), transport::buffer::sb_buffer_as_cstr(bh));
				}
				break;
			}
			default: {
				FTRACE("Execute command case default\n");
				transport::send_command_error("Unknowncommand");
				break;
			}
		}

		#endif
		treader.return_buffer(bh);
	}
}
