/**
 * This is the second motor experiment for my robot.
 * Change the loop() function so that the ramp up and ramp down loops
 * are state machines
 * 
 */
 /**
  * Motor n 
  * MOTORn_PWM_PIN is the pin to which one writes the pwm value that controlls the motor speed.
  * The value written to this pin is in the range 0 .. 255
  * 
  * MOTORn_DIRECTION_SELECT_PIN is the mode pin which determines the direction of rotation of the motor.
  */
#include "Arduino.h"
#define FDEBUG_ON
#define FTRACE_ON
// #include "avr8-stub.h"
// #include "app_api.h"

#include "config.h"
#include "utils.h"
#include "log.h"
#include "task.h"
#include "cli.h"
#include "./motor.h"
#include "./encoder.h"
#include "motion_control.h"
#include "motion_publisher.h"
#include "exercise_motors.h"
#include "remote_pid.h"
#include "reporter.h"
#include "encoder_message.h"
/**
 * Define the two motors and their pins
 */
Motor motor_left;
Motor motor_right;
ExerciseMotors exerciser(&motor_left, &motor_right);
Encoder* encoder_left_ptr;
Encoder* encoder_right_ptr;
MotionControl motion_control(motor_left, motor_right);
Reporter report_maker;
RemotePid remote_pid(motor_left, motor_right);
// MotionPublisher motion_publisher(Serial, motion_control);

void execute_command();
void collect_report();

Cli cli;
CommandBuffer command_buffer;
Task cli_task(10, &execute_command);
Task report_task(50, &collect_report);
Task motion_control_task(50, &motion_control);
Task exerciser_task(1000, &exerciser);
Task remote_pid_task(1500, &remote_pid);

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5
int mode;

void setup() {
	pinMode(LED_BUILTIN, OUTPUT);
	Serial.begin(115200);
	
	while(!Serial) {  ;   } // wait for Serial to be ready
	delay(2000);

	printf("Serial is active\n");
	// motor_left.begin(MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_DIRECTION_SELECT_PIN, MOTOR_LEFT_PWM_PIN);
	// motor_right.begin(MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_LEFT_DIRECTION_SELECT_PIN, MOTOR_RIGHT_PWM_PIN);
	// encoder_left_ptr = Encoder::make_encoder_left();
	// encoder_right_ptr = Encoder::make_encoder_right();
	// encoder_left_ptr->begin(&motor_left);
	// encoder_right_ptr->begin(&motor_right);
	// // report_maker.begin(encoder_left_ptr, encoder_right_ptr);
	// motion_control.begin(encoder_left_ptr, encoder_right_ptr);
	// remote_pid.begin(encoder_left_ptr, encoder_right_ptr);
	// mode = MODE_REMOTE_PID;
	printf("Setup complete\n");
}

// the loop function runs over and over again forever
void loop() {
	delay(2000);
	printf("In loop function \n");
	// for(;;) {
	// 	cli_task();
	// 	switch(mode){
	// 		case MODE_REPORT:
	// 			report_task();
	// 			break;
	// 		case MODE_MOTION:
	// 			motion_control_task();
	// 			break;
	// 		case MODE_REMOTE_PID:
	// 			remote_pid_task();
	// 			break;
	// 		case MODE_EXERCISE:
	// 			// exerciser_task();
	// 			break;
	// 		case MODE_COMMANDS_ONLY:
	// 			// exerciser_task();
	// 			break;
	// 	}
	// }
}
void apply_pid()
{
	encoder_right_ptr->run();
	encoder_left_ptr->run();

}
void remote_pid_func()
{

}
void collect_report()
{
	encoder_right_ptr->run();
	encoder_left_ptr->run();
	report_maker.run(encoder_left_ptr, encoder_right_ptr);
}
void execute_command()
{
	cli.run();
	if(cli.available()) {
		CommandBuffer& cref = command_buffer;
		Argv& argv = cli.consume();
		command_buffer.fill_from_tokens(argv);
		print_fmt("Case cref.identity: %d\n", cref.identity()); 
		switch(cref.identity()) {
			case CLI_COMMAND_TAG_NONE:
				break;
			case CLI_COMMAND_TAG_ERROR:
				print_fmt("Error: % %s\n", cref.error_command.msg);
				break;

			case CLI_COMMAND_MOTOR_RAW_PWM:
			case CLI_COMMAND_TAG_SPEED: {
				SpeedCommand& sc = cref.speed_command;
				print_fmt("SpeedCommand m1.pwm: %d m2.pwm:%d m1.direction %d m2.direction %d\n", 
					sc.m_left.pwm_value, sc.m_right.pwm_value, (int)sc.m_left.direction, (int)sc.m_right.direction);
				motion_control.set_pwm_direction(sc.m_left.pwm_value, (int)sc.m_left.direction, sc.m_right.pwm_value, (int)sc.m_right.direction);
				break;
			}
			case CLI_COMMAND_TAG_STOP:{
				print_fmt("StopCommand\n");
				motion_control.stop_all();
				break;
			}
			case CLI_COMMAND_TAG_AHEAD:{
				print_fmt("AheadCommand\n");
				AheadCommand& ac = cref.ahead_command;
				motion_control.ahead(ac.pwm);
				break;
			}
			case CLI_COMMAND_MOTOR_SPEED: {
				MotorSpeedCommand& sc = cref.motor_speed_command;
				print_fmt("MotorSpeedCommand m1.rpm: %f m1.direction %d m2.rpm %f m2.direction %d\n", 
					sc.m_left_rpm, (int)sc.m_left_direction, sc.m_right_rpm, (int)sc.m_right_direction);
				// motion_control.set_pwm_direction(sc.m_left.pwm_value, (int)sc.m_left.direction, sc.m_right.pwm_value, (int)sc.m_right.direction);
				break;
			}
			case CLI_COMMAND_PID_RPM: {
				PidRpmCommand& sc = cref.pid_rpm_command;
				print_fmt("PidRpmCommand m_left.rpm_value: %d m_right.rpm_value: %d \n", 
					sc.m_left.rpm_value, sc.m_right.rpm_value);
				motion_control.pid_set_rpm(sc.m_left.rpm_value, sc.m_right.rpm_value);
				break;
			}
			case CLI_COMMAND_REMOTE_PID: {
				RemotePidCommand& sc = cref.remote_pid_command;
				print_fmt("RemotePidCommand left_pwm: %d right_pwm: %d \n", sc.m_left_pwm, sc.m_right_pwm);
				remote_pid.set_pwm(sc.m_left_pwm, sc.m_right_pwm);
				break;
			}
			case CLI_COMMAND_UPDATE_PID: {
				UpdatePidCommand& sc = cref.update_pid_command;
				print_fmt("UpdatePidCommand kp: %f ki: %f kd: %f \n", sc.kp, sc.ki, sc.ki);
				break;
			}
			case CLI_COMMAND_READ_ENCODERS: {
				PidRpmCommand& sc = cref.pid_rpm_command;
				print_fmt("ReadEncodersCommand \n");

				motion_control.pid_set_rpm(sc.m_left.rpm_value, sc.m_right.rpm_value);
				break;
			}
			case CLI_COMMAND_RESET_ENCODERS: {
				PidRpmCommand& sc = cref.pid_rpm_command;
				print_fmt("ReadEncodersCommand \n");
				motion_control.pid_set_rpm(sc.m_left.rpm_value, sc.m_right.rpm_value);
				break;
			}
			default:{
				print_fmt("Execute command case default\n");
				break;
			}
		}
	}
}
void read_encoder_command(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr) {
	log_print("read_encoder_command\n");
    encoder_left_ptr->run();
    encoder_right_ptr->run();
    EncoderSample left_sample;
    EncoderSample right_sample;
	EncoderMessage message;
    if(encoder_left_ptr->available()) {
        encoder_left_ptr->consume(left_sample);
    }
    if(encoder_right_ptr->available()) {
        encoder_right_ptr->consume(right_sample);
    }
	message.format_samples(left_sample, right_sample);
	Serial.print(message.m_buffer);
}