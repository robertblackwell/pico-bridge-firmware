#undef TRACE_ON
#include <functional>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>
#include <utils.h>
#include <dri0002.h>
#include "config.h"
#include <encoder.h>
#include <task.h>
#include <motion.h>
#include <reporter.h>
#include <commands.h>

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5

Cli cli;
CommandBuffer command_buffer;
DRI0002V1_4 dri0002;
Encoder *encoder_left_ptr;
Encoder *encoder_right_ptr;
MotionControl motion;
Reporter reporter;

void sample_collect();
void execute_commands();
void config_dump();
bool chars_available_flag = false;
int callback_count = 0;
void chars_available(void* arg) {
	chars_available_flag = true;
	callback_count++;
	char* s = (char*)(arg);
	printf("Chars available callback %s\n", s);
}
int main()
{
	// const int pin_e1 = 15;
	// const int pin_m1 = 14;
	// const int pin_e2 = 3;
	// const int pin_m2 = 2;
	stdio_init_all();
	while (!tud_cdc_connected())
	{
		sleep_ms(100);
	}
	printf("stdio started\n");
	stdio_set_chars_available_callback(&chars_available, (void*)"This is a callback arg" );
	/**
	 * This next line is making a configuration decisions.
	 * It assigns 
	 * MOTOR_LEFT to E2 and M2
	 * MOTOR_RIGHT to E1 and M1
	*/
	dri0002.begin(
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN);	// E2
	encoder_left_ptr = make_encoder_left();
	encoder_left_ptr->begin(MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT);
	encoder_right_ptr = make_encoder_right();
	encoder_right_ptr->begin(MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT);
	motion.begin(&dri0002, encoder_left_ptr, encoder_right_ptr);
	reporter.begin(encoder_left_ptr, encoder_right_ptr);
	printf("After begin calls\n");
	Task cli_task(20, execute_commands);

	Task report_task(20, &reporter);
	dump_config(&motion);
	printf("Starting loop %s\n", "bridge");
	#if 0
	while(1) {
		sleep_ms(50);
		while(chars_available_flag) {

			int c = getchar_timeout_us(5);
			if(0 <= c && c <= 255) {
				printf("got this char: %c  %d cabback count is: %d\n", (char)c, (int)c, callback_count);
			} else {
				chars_available_flag = false;
			}
		}
	}
	#endif
	while (1)
	{
		cli_task();
		report_task();
	}
}
void sample_collect(Encoder *left, Encoder *right)
{
	left->run();
	right->run();
}
void execute_commands()
{
	cli.run();
	if (cli.available())
	{
		CommandBuffer &cref = command_buffer;
		Argv &argv = cli.consume();
		command_buffer.fill_from_tokens(argv);
		// printf("Case cref.identity: %d\n", cref.identity());
		switch (cref.identity())
			{
			case CliCommands::None:
				break;
			case CliCommands::Error:
				printf("Error: % %s\n", cref.error_command.msg);
				break;

			case CliCommands::MotorsPwmPercent:
			{
				MotorsPwmPercentCommand &c = cref.motors_pwm_percent_command;
				printf("MotorsPwmPercentCommand m1.pwm: %f m2.pwm:%f\n",
					c.left_pwm_percent_value, c.right_pwm_percent_value);
				motion.set_pwm_percent(c.left_pwm_percent_value, c.right_pwm_percent_value);
				break;
			}
			case CliCommands::MotorsRpm:
			{
				MotorsRpmCommand &c = cref.motors_rpm_command;
				printf("Motors Rpw Command m1.rpm: %f m1.direction %d m2.rpm %f m2.direction %d\n",
						c.m_left_rpm, c.m_right_rpm);
				// motion_control.set_pwm_direction(sc.m_left.pwm_value, (int)sc.m_left.direction, sc.m_right.pwm_value, (int)sc.m_right.direction);
				break;
			}
			case CliCommands::MotorsHalt:
			{
				printf("StopCommand\n");
				motion.stop_all();
				break;
			}
			case CliCommands::EncodersRead:
			{
				EncodersReadCommand &rec = cref.encoders_read_command;
				printf("ReadEncodersCommand number: %d\n", rec.m_number);
				reporter.request(rec.m_number);
				// motion.pid_set_rpm(rec.m_left.rpm_value, sc.m_right.rpm_value);

				break;
			}
			case CliCommands::PidArgsUpdate: {
				PidArgsUpdateCommand& c = cref.pid_args_update_command;
				print_fmt("UpdatePidCommand kp: %f ki: %f kd: %f \n", c.kp, c.ki, c.ki);
				break;
			}
			default:
			{
				print_fmt("Execute command case default\n");
				break;
			}
		}
		printf("EndCommand\n");
	}
}
