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

// Cli cli;
transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	#if 1
	trace_init();
	#else
	trace_init_full(uart1, 115200, 8, 9);
	#endif
	sleep_ms(5000);
	printf("stdio started\n");
	// print_fmt("bridge starting ... \n");
	#if 0
	int count = 0;
	while(1) {
		sleep_ms(2000);
		printf("stdio looping %d\n", count);
		// print_fmt("bridge looping %d ... \n", count);
		// uart_puts(uart1, "this is from uart1 \r\n");
		count++;
	}
	#else
	robot_init();
	Task cli_task(20, execute_commands);
	Task encoder_samples_task(20, &robot_collect_encoder_samples);
	while (1)
	{
		cli_task();
		// encoder_samples_task();	
	}
	#endif
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
		FTRACE("This is what treader got [%s]", transport::buffer::sb_buffer_as_cstr(bh));
		FDUMP_TOKENS(args, "Dump tokens message")
		treader.return_buffer(bh);
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
			default: {
				FTRACE("Execute command case default\n");
				transport::send_command_error("Unknowncommand");
				break;
			}
		}
		#endif
	}
}
