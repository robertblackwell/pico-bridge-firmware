
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
static double robot_velocity_meters_per_second;
static double robot_heading_degrees;
static double robot_position_x;
static double robot_position_y;
static double robot_left_rpm_target;
static double robot_right_rpm_target;
static double robot_left_wheel_velocity_target_ms;
static double robot_right_wheel_velocity_target_ms;
static double robot_velocity_target_ms;
static double robot_heading_degrees_target;
static MotionControl motion_controller{};

namespace robot {
void init()
{
	encoder_left_ptr = Encoder::get_encoder_left();
	encoder_right_ptr = Encoder::get_encoder_right();
    motion_controller.begin(&dri0002, encoder_left_ptr, encoder_right_ptr);

    encoder_left_ptr->start_handling_interrupts();
    encoder_right_ptr->start_handling_interrupts();
    robot_velocity_meters_per_second = 0.0;
    robot_heading_degrees = 0.0;
    robot_position_x = 0.0;
    robot_position_y = 0.0;
    robot_left_rpm_target = 0.0;
    printf("init leaving encoder_left_ptr: %p encoder_right_ptr:%p \n", encoder_left_ptr, encoder_right_ptr);
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
bool set_wheel_velocity_ms(const double left_velocity_target_ms, const double right_velocity_target_ms)
{
    robot_left_wheel_velocity_target_ms = left_velocity_target_ms;
    robot_right_wheel_velocity_target_ms = right_velocity_target_ms;
    return true;
}
    bool set_rpm(const double left_rpm, const double right_rpm)
{
    robot_left_rpm_target = left_rpm;
    robot_right_rpm_target = right_rpm;
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
struct PiContext {
    double kp;
    double ki;
    double integral;
    double delta_time_secs;
};
static PiContext left_pi_ctx;
static PiContext right_pi_ctx;
void   pi_ctx_init(PiContext& ctx, double kp, double ki)
{
    ctx.kp = kp;
    ctx.ki = ki;
    ctx.integral = 0;
    ctx.delta_time_secs = 1.00;

}
double pi_wheel_speed_control_next_pwm_estimate(PiContext& ctx, const double target, const double latest)
{
    auto error = target - latest;
    ctx.integral = ctx.integral + (error * ctx.delta_time_secs);
    auto newpwm = ctx.kp * error + ctx.ki * ctx.integral;
    printf("pi_wheel_speed_control_next_pwm_estimate: target %f\n\t latest: %f\n\terror: %f\n\tEi: %f\n\tnewpwm: %f \n", target, latest, error, ctx.integral, newpwm);
    return (newpwm > 100.0) ? 100.0 : newpwm;
}

void start()
{
    const auto abs_time = get_absolute_time();
    last_poll_time_ms = to_ms_since_boot(abs_time);
    encoder_left_ptr->m_previous_sample_time_usecs = to_us_since_boot(abs_time);
    encoder_right_ptr->m_previous_sample_time_usecs = encoder_left_ptr->m_previous_sample_time_usecs;
    pi_ctx_init(left_pi_ctx, 90.0, 650.0);
    pi_ctx_init(right_pi_ctx, 90.0, 650.0);
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
        const EncoderSample& sleft = encoder_left_ptr->m_sample;
        const EncoderSample& sright = encoder_right_ptr->m_sample;

        robot_velocity_meters_per_second = 0.5 * (sleft.s_speed_mm_per_second + sright.s_speed_mm_per_second);
        robot_heading_degrees = (sright.s_speed_mm_per_second - sleft.s_speed_mm_per_second) / ISR_AXLE_LENGTH_MM;
        double left_new_pwm = pi_wheel_speed_control_next_pwm_estimate(left_pi_ctx, robot_left_wheel_velocity_target_ms, (sleft.s_speed_mm_per_second/1000.0));
        double right_new_pwm = pi_wheel_speed_control_next_pwm_estimate(right_pi_ctx, robot_right_wheel_velocity_target_ms, (sright.s_speed_mm_per_second/1000.0));
        printf("left_new_pwm: %f\n", left_new_pwm);
        printf("right_new_pwm: %f\n", right_new_pwm);
        motion_controller.set_raw_pwm_percent(left_new_pwm, right_new_pwm);


        printf("Robot twist vel mm/sec: %f theta (radians/sec): %f\n", robot_velocity_meters_per_second, robot_heading_degrees);
        printf("Robot rpm_left: %f rpm_right: %f \n", sleft.s_wheel_rpm, sright.s_wheel_rpm);
        printf("Robot vel_left(m/s): %f vel_right(m/s): %f \n", (sleft.s_speed_mm_per_second/1000.0), (sright.s_speed_mm_per_second/1000.0));
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
