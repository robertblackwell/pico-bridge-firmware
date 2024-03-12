
#undef FTRACE_ON
#include "robot.h"
#include <stdio.h>
#include <pico/platform.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "trace.h"
#include "dri0002.h"
#include "config.h"
#include "encoder_sample.h"
#include "encoder.h"
#include "task.h"
#include "motion.h"
#include "transport/buffers.h"

DRI0002V1_4 dri0002{
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN	    // E2
};

/**
 * The following is a bit arcane
 * It chooses between two strategies for encoder ISRs. Each encoder needs 2 ISRs - one each for PINS A and B.
 * The code is the same for pin A and B. Infact the code is the same for all Encoder ISRs all that changes is
 * the encoder it self.
 *
 * there are currently 2 strategies for handling this:
 *
 * 1.   Encoders funnel all their interrupts through a common isr handler but must pass a pointer to their
 *      Encoder instance to that common isr. This is the least tricky way I can find of doing that
 *
 * 2.   Encoder ISR code is short enough to justify repeating it in ISR.
*/
#ifdef ISR_COMMON_FUNCTION
#define ISR_GUTS(enc) \
    encoder_common_isr(&enc);
#else
#define ISR_GUTS(enc) \
    enc.m_isr_interrupt_count = encoder_left.m_isr_interrupt_count + 1; \
    enc.m_isr_sample_most_recent_time_usecs = micros();
#endif
void isr_apin_left();
void isr_bpin_left();
Encoder encoder_left{MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT, isr_apin_left, isr_bpin_left};
void isr_apin_left() {
    ISR_GUTS(encoder_left)
}
void isr_bpin_left() {
    ISR_GUTS(encoder_left)
}

void isr_apin_right();
void isr_bpin_right();
Encoder encoder_right{MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT, isr_apin_right, isr_bpin_right};
void isr_apin_right() {
    ISR_GUTS(encoder_right)
}
void isr_bpin_right() {
    ISR_GUTS(encoder_right)
}

Encoder* encoder_left_ptr = &encoder_left;
Encoder* encoder_right_ptr = &encoder_right;
MotionControl motion_controller{&dri0002, encoder_left_ptr, encoder_right_ptr};

void robot_init()
{
	encoder_left_ptr->start_interrupts();
	encoder_right_ptr->start_interrupts();
}

void robot_set_raw_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_raw_pwm_percent(left_pwm_percent, right_pwm_percent);
}
void robot_set_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_pwm_percent(left_pwm_percent, right_pwm_percent);
}
bool robot_set_rpm(double left_rpm, double right_rpm)
{
    if(!motion_controller.verify_left_rpm_settable((float)left_rpm)) {
//        robot_error_msg = "left rpm value invalid probably trying to change direction without stopping";
        return false;
    }
    if(!motion_controller.verify_right_rpm_settable((float)right_rpm)) {
//        robot_error_msg = "left rpm value invalid probably trying to change direction without stopping";
        return false;
    }
    motion_controller.pid_set_rpm(left_rpm, right_rpm);
    return true;
}
void robot_stop_all()
{
    motion_controller.stop_all();
}


void tojson_encoder_samples(transport::buffer::Handle buffer_h)
{
#if ! defined(SAMPLE_COLLECTION_TASK)
//    unsafe_collect_two_encoder_samples(*encoder_left_ptr, (encoder_left_ptr->m_sample), *encoder_right_ptr, encoder_right_ptr->m_sample);
#endif
    // encoder_left_ptr->m_sample.dump();
    // encoder_right_ptr->m_sample.dump();

    tojson_two_encoder_samples(buffer_h, &encoder_left_ptr->m_sample, &encoder_right_ptr->m_sample);
}
bool timer_callback(repeating_timer_t* timer)
{
    unsafe_collect_two_encoder_samples(
        *encoder_left_ptr, 
        encoder_left_ptr->m_sample, 
        *encoder_right_ptr,
        encoder_right_ptr->m_sample);
    // update closed loop controller
    return true;
}
void robot_start_encoder_sample_collection(uint64_t sample_interval_us)
{
    static repeating_timer_t timer;
    add_repeating_timer_us(+sample_interval_us, &timer_callback, NULL, &timer);
    // start encoder interrupts here
}
void robot_collect_encoder_samples()
{
	FTRACE("robot::collect_encoder_samples\n", "");
    unsafe_collect_two_encoder_samples(*encoder_left_ptr, encoder_left_ptr->m_sample, *encoder_right_ptr,
                                       encoder_right_ptr->m_sample);
}
