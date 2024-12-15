
#undef FTRACE_ON
#include "robot.h"
#include <cstdio>
#include <pico/platform.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "trace.h"
#include "dri0002.h"
#include "config.h"
#include "encoder_sample.h"
#include "encoder_v2.h"
#include "task.h"
#include "motion.h"
#include "transport/buffers.h"


void robot_set_raw_pwm_percent(MotionControl& motion_controller, double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_raw_pwm_percent(left_pwm_percent, right_pwm_percent);
}
void robot_set_pwm_percent(MotionControl& motion_controller, double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_pwm_percent(left_pwm_percent, right_pwm_percent);
}
bool robot_set_rpm(MotionControl& motion_controller, double left_rpm, double right_rpm)
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
void robot_stop_all(MotionControl& motion_controller)
{
    motion_controller.stop_all();
}


void tojson_encoder_samples(MotionControl& mc, transport::buffer::Handle buffer_h)
{
    Encoder* encoder_left_ptr = mc.m_left_encoder_ptr;
    Encoder* encoder_right_ptr = mc.m_right_encoder_ptr;
    //printf("robot::tojson_encoder_samples\n");
    Encoder::unsafe_collect_two_encoder_samples(*encoder_left_ptr, (encoder_left_ptr->m_sample), *encoder_right_ptr, encoder_right_ptr->m_sample);
    // encoder_left_ptr->m_sample.dump();
    // encoder_right_ptr->m_sample.dump();

    tojson_two_encoder_samples(buffer_h, &encoder_left_ptr->m_sample, &encoder_right_ptr->m_sample);
}
#if 0
bool timer_callback(repeating_timer_t* timer)
{
    //printf("timer_callback\n");
    Encoder::unsafe_collect_two_encoder_samples(
        *encoder_left_ptr, 
        encoder_left_ptr->m_sample, 
        *encoder_right_ptr,
        encoder_right_ptr->m_sample);
    // update closed loop controller
    return true;
}
void robot_start_encoder_sample_collection(uint64_t sample_interval_us)
{
    //printf("robot_start_encoder_sample_collection\n");
    static repeating_timer_t timer;
    add_repeating_timer_us(+sample_interval_us, &timer_callback, NULL, &timer);
    // start encoder interrupts here
}
void robot_collect_encoder_samples()
{
	FTRACE("robot::collect_encoder_samples\n", "");
    Encoder::unsafe_collect_two_encoder_samples(*encoder_left_ptr, encoder_left_ptr->m_sample, *encoder_right_ptr,
                                       encoder_right_ptr->m_sample);
}
#endif