#undef TRACE_ON
#include <functional>
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
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

void sample_collect();
void execute_commands();
void config_dump();


int mode;
Cli cli;
CommandBuffer command_buffer;
#define RAII_ENCODER
#ifdef RAII_ENCODER
/*
************************************************************************
* RAII initialization
************************************************************************
*/
DRI0002V1_4 dri0002{
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN	    // E2
};

/**
 * Encoders must funnel all their interrupts through a common isr handler but must pass a pointer to their
 * Encoder istance to that common isr. This is the least tricky way I can find of doing that
*/
void isr_apin_left();
void isr_bpin_left();

Encoder encoder_left{MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT, isr_apin_left, isr_bpin_left};
void isr_apin_left(){encoder_common_isr(&encoder_left);}
void isr_bpin_left(){ /*encoder_common_isr(&encoder_left)*/;} // only want interrupts on the a-pin

void isr_apin_right();
void isr_bpin_right();
Encoder encoder_right{MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT, isr_apin_right, isr_bpin_right};
void isr_apin_right(){encoder_common_isr(&encoder_right);}
void isr_bpin_right(){ /*encoder_common_isr(&encoder_right)*/;} // only want interrupts on the a-pin

Encoder* encoder_left_ptr = &encoder_left;
Encoder* encoder_right_ptr = &encoder_right;
MotionControl motion_controller{&dri0002, encoder_left_ptr, encoder_right_ptr};
Reporter reporter{encoder_left_ptr, encoder_right_ptr};
/*
************************************************************************
* RAII initialization
************************************************************************
*/
#else
DRI0002V1_4 dri0002;
Encoder *encoder_left_ptr;
Encoder *encoder_right_ptr;
#endif


int main()
{
	stdio_init_all();
	sleep_ms(5000);
	printf("stdio started\n");
#ifdef RAII_ENCODER
	encoder_left_ptr->start_interrupts();
	encoder_right_ptr->start_interrupts();
#else
	dri0002.begin(
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN	    // E2
	);
	encoder_left_ptr = make_encoder_left();
	encoder_left_ptr->begin(MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT);
	encoder_right_ptr = make_encoder_right();
	encoder_right_ptr->begin(MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT);
	MotionControl motion_controller;
	motion_controller.begin(&dri0002, encoder_left_ptr, encoder_right_ptr);
	Reporter reporter;
	reporter.begin(encoder_left_ptr, encoder_right_ptr);
#endif

	printf("After begin calls\n");
	Task cli_task(20, execute_commands);
	Task motion_task(2000, &motion_controller);
	Task report_task(20, &reporter);
	dump_config(&motion_controller);
	printf("Starting loop %s\n", "onboard_pid");
	while (1)
	{
		cli_task();
		motion_task();
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
				motion_controller.set_pwm_percent(c.left_pwm_percent_value, c.right_pwm_percent_value);
				break;
			}
			case CliCommands::MotorsRpm:
			{
				MotorsRpmCommand &c = cref.motors_rpm_command;
				printf("Motors Rpw Command m1.rpm: %f m2.rpm %f\n",
						c.m_left_rpm, c.m_right_rpm);
				motion_controller.pid_set_rpm(c.m_left_rpm, c.m_right_rpm);
				break;
			}
			case CliCommands::MotorsHalt:
			{
				printf("StopCommand\n");
				motion_controller.stop_all();
				break;
			}
			case CliCommands::EncodersRead:
			{
				EncodersReadCommand &rec = cref.encoders_read_command;
				printf("ReadEncodersCommand number: %d\n", rec.m_number);
				reporter.request(rec.m_number);
				break;
			}
			case CliCommands::PidArgsUpdate: {
				PidArgsUpdateCommand& c = cref.pid_args_update_command;
				print_fmt("UpdatePidCommand kp: %f ki: %f kd: %f \n", c.kp, c.ki, c.ki);
				motion_controller.update_pid(c.kp, c.ki, c.kd);
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
