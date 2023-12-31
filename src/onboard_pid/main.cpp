#undef TRACE_ON
#include <functional>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "trace.h"
#include "dri0002.h"
#include "config.h"
#include "encoder.h"
#include "task.h"
#include "motion.h"
#include "reporter.h"
#include "commands.h"

#include "robot.h"

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5

void sample_collect();
void execute_commands();
void config_dump();

int mode;
Cli cli;
CommandBuffer command_buffer;

int main()
{
	stdio_init_all();
	trace_init();
	sleep_ms(5000);
	printf("stdio started\n");
	robot_init();

	printf("After begin calls\n");
	Task cli_task(20, execute_commands);
	printf("Starting loop %s\n", "onboard_pid");
	while (1)
	{
		cli_task();
		robot_tasks_run();
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
				robot_set_pwm_percent(c.left_pwm_percent_value, c.right_pwm_percent_value);
				break;
			}
			case CliCommands::MotorsRpm:
			{
				MotorsRpmCommand &c = cref.motors_rpm_command;
				printf("Motors Rpw Command m1.rpm: %f m2.rpm %f\n",
						c.m_left_rpm, c.m_right_rpm);
				robot_set_rpm(c.m_left_rpm, c.m_right_rpm);
				break;
			}
			case CliCommands::MotorsHalt:
			{
				printf("StopCommand\n");
				robot_stop_all();
				break;
			}
			case CliCommands::EncodersRead:
			{
				EncodersReadCommand &rec = cref.encoders_read_command;
				printf("ReadEncodersCommand number: %d\n", rec.m_number);
				robot_request(rec.m_number);
				break;
			}
			case CliCommands::PidArgsUpdate: {
				PidArgsUpdateCommand& c = cref.pid_args_update_command;
				print_fmt("UpdatePidCommand kp: %f ki: %f kd: %f \n", c.kp, c.ki, c.ki);
				robot_update_pid(c.kp, c.ki, c.kd);
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
