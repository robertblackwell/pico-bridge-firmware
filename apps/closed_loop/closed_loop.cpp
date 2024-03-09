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
void execute_commands();
void heart_beat();
#include "pico/stdlib.h"
#include <pico/error.h>

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

transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
	print_fmt("%s starting ... \n", __FILE__);
	robot_init();
	Task cli_task(20, execute_commands);
	Task heart_beat_task(5000, heart_beat);
	robot_start_encoder_sample_collection((uint64_t)100000);
	while (1)
	{
		cli_task();
		heart_beat_task();
		// collect_samples_task();
	}
}
void heart_beat()
{
	printf("Heart beat \n");
}
void execute_commands()
{
	treader.run();
	if (treader.available())
	{
		Handle bh = treader.borrow_buffer();
		Argv args{};
		if(!args.tokenize(sb_buffer_as_cstr(bh))) {
			printf("Tokenize failed buffer: %s\n", sb_buffer_as_cstr(bh));
		} else {
			FTRACE("This is what treader got [%s]\n", sb_buffer_as_cstr(bh));
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
					transport::send_command_error("Invalid command %s", sb_buffer_as_cstr(bh));
					break;
				case CommandName::MotorsPwmPercent: {
					double left_pwm, right_pwm;
					if(validate_pwm(args, left_pwm, right_pwm)) {
						robot_set_raw_pwm_percent(left_pwm, right_pwm);
						transport::send_command_ok("MotorPwmPercent %f  %f", left_pwm, right_pwm);
					} else {
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
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
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
					}
					break;
				}
				case CommandName::MotorsHalt: {
					if(validate_encoder_read(args)) {
						printf("%s\n", to_string(enumname));
						robot_stop_all();
						transport::send_command_ok("StopCommand");
					} else {
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
					}
					break;
				}
				case CommandName::EncodersRead:
				{
					int number = 1;
					if(validate_encoder_read(args)) {
						// printf("%s\n", to_string(enumname));
						robot_request(number);
						Handle h = tx_pool::allocate();
						tojson_encoder_samples(h);
						transport::send_json_response(&h);
						// printf("End of read encoder cmd \n");
					} else {
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
					}
					break;
				}
				case CommandName::PidArgsUpdate: {
					transport::send_command_ok("PidArgUpdate command not implemented [%s]", sb_buffer_as_cstr(bh));
					break;
				}
				case CommandName::Echo: {
					if(validate_encoder_read(args)) {
						printf("%s\n", to_string(enumname));
						transport::send_command_ok("Echo command [%s]", sb_buffer_as_cstr(bh));
					} else {
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
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
						transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
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
		treader.return_buffer(bh);
	}
}
