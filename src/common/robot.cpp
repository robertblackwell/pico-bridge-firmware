#include "robot.h"
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

Task motion_task(2000, &motion_controller);
Task report_task(20, &reporter);

/*
************************************************************************
* RAII initialization
************************************************************************
*/
#else
#endif


void robot_init()
{
#ifdef RAII_ENCODER
	encoder_left_ptr->start_interrupts();
	encoder_right_ptr->start_interrupts();
#else
#endif

}

void robot_tasks_run()
{
    motion_task();
    report_task();
}
void robot_reporter_task_run()
{
    report_task();
}
void robot_set_raw_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_raw_pwm_percent(left_pwm_percent, right_pwm_percent);
}
void robot_set_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_pwm_percent(left_pwm_percent, right_pwm_percent);
}
void robot_set_rpm(double left_rpm, double right_rpm)
{
    motion_controller.pid_set_rpm(left_rpm, right_rpm);
}
void robot_stop_all()
{
    motion_controller.stop_all();
}
void robot_request(int n)
{
    reporter.request(n);
}
void robot_update_pid(double kp, double ki, double kd)
{
    motion_controller.update_pid(kp, ki, kd);
}
