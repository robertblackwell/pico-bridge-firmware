#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "cli.h"
#include "commands.h"
void execute_command();

// #define CLI_STDIO_SERIAL
// #if defined CLI_STDIO_SERIAL
// #include <pico/error.h>
// static bool get_char_if_available(int* char_received) {
//     int ch = getchar_timeout_us(0);
//     if(ch == PICO_ERROR_TIMEOUT) {
//         return false;
//     }
//     *char_received = ch;
//     return true;
// }
// #else
// #include <tusb.h>
// static bool get_char_if_available(int* char_received) {

//     if(tud_cdc_available()) {
//         int ch = getchar();
//         *char_received = ch;
//         return true;
//     }
//     return false;
// }
// #endif


Cli cli;
CommandBuffer command_buffer;
int main()
{
    stdio_init_all();
	sleep_ms(5000);
	printf("We are ready enter a command \n");
    cli.begin();
    while(true) {
        sleep_ms(500);
        execute_command();
    }
}
void execute_command()
{
	cli.run();
	if(cli.available()) {
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
				// motion.set_pwm_percent(c.left_pwm_percent_value, c.right_pwm_percent_value);
				break;
			}
			case CliCommands::MotorsRpm:
			{
				MotorsRpmCommand &c = cref.motors_rpm_command;
				printf("Motors Rpw Command m1.rpm: %f m2.rpm %f \n",
						c.m_left_rpm, c.m_right_rpm);
				// motion_control.set_pwm_direction(sc.m_left.pwm_value, (int)sc.m_left.direction, sc.m_right.pwm_value, (int)sc.m_right.direction);
				break;
			}
			case CliCommands::MotorsHalt:
			{
				printf("StopCommand\n");
				// motion.stop_all();
				break;
			}
			case CliCommands::EncodersRead:
			{
				EncodersReadCommand &rec = cref.encoders_read_command;
				printf("ReadEncodersCommand number: %d\n", rec.m_number);
				// reporter.request(rec.m_number);
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
