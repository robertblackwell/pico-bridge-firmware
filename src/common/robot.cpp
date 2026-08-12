
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
#include "transport/transport.h"
#include "transport/buffers.h"
#include "transport/transmit_buffer_pool.h"


static DRI0002V1_4 dri0002{
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN	    // E2
};
static Encoder* encoder_left_ptr;
static Encoder* encoder_right_ptr;
static MotionControl motion_controller{};

namespace robot {
void init()
{
	encoder_left_ptr = Encoder::get_encoder_left();
	encoder_right_ptr = Encoder::get_encoder_right();
    motion_controller.begin(&dri0002, encoder_left_ptr, encoder_right_ptr);

    encoder_left_ptr->start_handling_interrupts();
    encoder_right_ptr->start_handling_interrupts();
    printf("init leaving encoder_left_ptr: %x encoder_right_ptr:%x \n", encoder_left_ptr, encoder_right_ptr);
}
Encoder* get_encoder(DriveSide side)
{
    return (side == ::MotorSide::left) ? encoder_left_ptr: encoder_right_ptr;
}
inline MotionControl::RpmValue get_current_rpm(DriveSide side)
{
    if(side == DriveSide::left) return motion_controller.m_left_rpm_target;
    return motion_controller.m_right_rpm_target;
}
inline MotionControl::PwmValue get_current_pwm(DriveSide side)
{
    if(side == DriveSide::left) return motion_controller.m_left_current_pwm;
    return motion_controller.m_right_current_pwm;
}
inline bool verify_side_rpm_settable(DriveSide side, float rpm)
{
    // MotionControl::RpmValue new_request{rpm};
    // auto current_rpm = get_current_rpm(mc, side);
    // if((! new_request.is_zero()) && (! current_rpm.is_zero())) {
    //     return (new_request.direction() == current_rpm.direction());
    // }
    return true;
}
inline bool verify_side_pwm_settable(DriveSide side, float pwm)
{
    // MotionControl::RpmValue new_request{pwm};
    // auto current_pwm = get_current_pwm(mc, side);
    // if((! new_request.is_zero()) && (! current_pwm.is_zero())) {
    //     return (new_request.direction() == current_pwm.direction());
    // }
    return true;
}
void set_raw_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_raw_pwm_percent(left_pwm_percent, right_pwm_percent);
}
void set_pwm_percent(double left_pwm_percent, double right_pwm_percent)
{
    motion_controller.set_pwm_percent(left_pwm_percent, right_pwm_percent);
}
bool set_rpm(double left_rpm, double right_rpm)
{
    if(!motion_controller.verify_left_rpm_settable((float)left_rpm)) {
//        error_msg = "left rpm value invalid probably trying to change direction without stopping";
        return false;
    }
    if(!motion_controller.verify_right_rpm_settable((float)right_rpm)) {
//        error_msg = "left rpm value invalid probably trying to change direction without stopping";
        return false;
    }
    motion_controller.pid_set_rpm(left_rpm, right_rpm);
    return true;
}
void stop_all()
{
    motion_controller.stop_all();
}
void tojson_encoder_samples(transport::buffer::Handle buffer_h)
{
    Encoder::unsafe_collect_two_encoder_samples(*encoder_left_ptr, (encoder_left_ptr->m_sample), *encoder_right_ptr, encoder_right_ptr->m_sample);
    tojson_two_encoder_samples(buffer_h, &encoder_left_ptr->m_sample, &encoder_right_ptr->m_sample);
}

static uint64_t last_poll_time_ms;
static uint64_t poll_interval_ms = 1000;
void start()
{
    const auto abs_time = get_absolute_time();
    last_poll_time_ms = to_ms_since_boot(abs_time);
    encoder_left_ptr->m_previous_sample_time_usecs = to_us_since_boot(abs_time);
    encoder_right_ptr->m_previous_sample_time_usecs = encoder_left_ptr->m_previous_sample_time_usecs;
}
void poll()
{   
    const uint64_t now = to_ms_since_boot(get_absolute_time());
    if(now >= last_poll_time_ms + poll_interval_ms) {
        Encoder::unsafe_collect_two_encoder_samples(
            *encoder_left_ptr, 
            encoder_left_ptr->m_sample, 
            *encoder_right_ptr,
            encoder_right_ptr->m_sample);

        last_poll_time_ms = now;
        // transport::buffer::Handle h = transport::buffer::tx_pool::allocate();
        // tojson_two_encoder_samples(h, &encoder_left_ptr->m_sample, &encoder_right_ptr->m_sample);
        // transport::send_json_response(&h);
    }
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
void start_encoder_sample_collection(uint64_t sample_interval_us)
{
    //printf("start_encoder_sample_collection\n");
    static repeating_timer_t timer;
    add_repeating_timer_us(+sample_interval_us, &timer_callback, NULL, &timer);
    // start encoder interrupts here
}
void collect_encoder_samples()
{
	FTRACE("robot::collect_encoder_samples\n", "");
    Encoder::unsafe_collect_two_encoder_samples(*encoder_left_ptr, encoder_left_ptr->m_sample, *encoder_right_ptr,
                                       encoder_right_ptr->m_sample);
}
#endif
} // namespace
