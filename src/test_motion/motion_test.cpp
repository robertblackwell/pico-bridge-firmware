#undef TRACE_ON
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>
#include <utils.h>
#include <dri0002.h>
#include "config.h"
#include <encoder.h>
#include <motion.h>
#include <reporter.h>
#include "exercise_motors.h"

void test01(DRI0002V1_4& dri, uint percent)
{
	uint level = (uint)65534 * percent / 100;
	FTRACE("test01 DRTI0002 percent: %u level %u\n", percent, level);
	dri.set_pwm_percent(MotorSide::left, (double)percent);
	dri.set_pwm_percent(MotorSide::right, (double)percent);
	// dri.set_pwm_percent(1, (double)(100 - percent));
	sleep_ms(3000);
}
void test01(PwmPiPico& pwm, uint percent)
{
	uint level = (uint)65534 * percent / 100;
	FTRACE("test01 RobotPwm percent: %u level %u\n", percent, level);
	pwm.set_level(level);
	// dri.set_pwm_percent(1, (double)(100 - percent));
	sleep_ms(3000);
}
void test_updown(DRI0002V1_4& dri0002)
{
	for(int x = 0; x < 3; x++) {
		for(uint p = 100; p >= 10; p=p-20) {
			test01(dri0002, p);
		}
		dri0002.set_pwm_percent(MotorSide::left, 0.0);
		dri0002.set_direction_pin_state(MotorSide::left, !dri0002.get_direction_pin_state(MotorSide::left));
		dri0002.set_pwm_percent(MotorSide::right, 0.0);
		dri0002.set_direction_pin_state(MotorSide::right, !dri0002.get_direction_pin_state(MotorSide::right));
	}
}
void sample_collect();
int main()
{
	// const int pin_e1 = 15;
	// const int pin_m1 = 14;
	// const int pin_e2 = 3;
	// const int pin_m2 = 2;
    stdio_init_all();
    while(! tud_cdc_connected()) {
        sleep_ms(100);
    }
	printf("stdio started\n");
	DRI0002V1_4 dri0002;
	dri0002.begin(
		MOTOR_RIGHT_DRI0002_SIDE, MOTOR_RIGHT_PWM_PIN, MOTOR_RIGHT_DIRECTION_SELECT_PIN, 
		MOTOR_LEFT_DRI0002_SIDE, MOTOR_LEFT_PWM_PIN, MOTOR_LEFT_DIRECTION_SELECT_PIN);
	ExerciseMotors exercise;
	exercise.begin(&dri0002);
	Task exercise_task(3000, &exercise);

	Encoder* encoder_left_ptr = make_encoder_left();
	encoder_left_ptr->begin(MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT);
	Encoder* encoder_right_ptr = make_encoder_right();
	encoder_right_ptr->begin(MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT);

	// MotionControl motion;
	// motion.begin(&dri0002, encoder_left_ptr, encoder_right_ptr);
	// Task motion_task(10, &motion);

	Reporter reporter;
	reporter.begin(encoder_left_ptr, encoder_right_ptr);
	Task report_task(0, &reporter);
    /// \tag::setup_pwm[]
	// sleep_ms(5000);
	printf("Stariung loop\n");
	while(1) {
		exercise_task();
		// motion_task();
		report_task();
	}
}
void sample_collect(Encoder* left, Encoder* right)
{
	left->run();
	right->run();
}