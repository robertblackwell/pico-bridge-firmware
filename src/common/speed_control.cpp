
#undef FTRACE_ON
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


static uint64_t last_poll_time_ms;
static uint64_t poll_interval_ms = SCL_LOOP_INTERVAL_MS;
struct SpeedControl
{
    double m_kp;
    double m_ki;
    double m_integral;
    double m_delta_time_secs;
public:
	void   init(double kp, double ki)
	{
		m_ki = ki;
		m_integral = 0;
		m_delta_time_secs = 1.00;
	}
	double next_pwm_estimate(const double target_velocity, const double latest_velocity)
	{
		const auto error = target_velocity - latest_velocity;
		m_integral = m_integral + (error * m_delta_time_secs);
		const auto newpwm = m_kp * error + m_ki * m_integral;
		printf("pi_wheel_speed_control_next_pwm_estimate: target %f\n\t latest: %f\n\terror: %f\n\tEi: %f\n\tnewpwm: %f \n", target_velocity, latest_velocity, error, m_integral, newpwm);
		return (newpwm > 100.0) ? 100.0 : newpwm;
	}
};
