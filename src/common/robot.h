#ifndef H_robot_h
#define H_robot_h
#include "transport/buffers.h"
#include "dri0002.h"
#include "encoder_v2.h"
#include "motion.h"
#if 1
/*********************************************************************************************
 * Accessing the left and right side of the drive
 ***********************************************************************************************/
inline Encoder* get_encoder(MotionControl& mc, DriveSide side)
{
    return mc.encoder_ptr(side);
}
inline MotionControl::RpmValue get_current_rpm(MotionControl& mc, DriveSide side)
{
    if(side == DriveSide::left) return mc.m_left_rpm_target;
    return mc.m_right_rpm_target;
}
inline MotionControl::PwmValue get_current_pwm(MotionControl& mc, DriveSide side)
{
    if(side == DriveSide::left) return mc.m_left_current_pwm;
    return mc.m_right_current_pwm;
}
#endif
/*********************************************************************************************
 * functions to validate that pwm and rpm values are within acceptable ranges and
 * are not asking a motor to change direction will rotating
 ***********************************************************************************************/
inline bool verify_side_rpm_settable(MotionControl& mc, DriveSide side, float rpm)
{
    MotionControl::RpmValue new_request{rpm};
    auto current_rpm = get_current_rpm(mc, side);
    if((! new_request.is_zero()) && (! current_rpm.is_zero())) {
        return (new_request.direction() == current_rpm.direction());
    }
    return true;
}
inline bool verify_side_pwm_settable(MotionControl& mc, DriveSide side, float pwm)
{
    MotionControl::RpmValue new_request{pwm};
    auto current_pwm = get_current_pwm(mc, side);
    if((! new_request.is_zero()) && (! current_pwm.is_zero())) {
        return (new_request.direction() == current_pwm.direction());
    }
    return true;
}

/*********************************************************************************************
 * these functions execute specific command that need access to drive details
 ***********************************************************************************************/
/**
 * Sets the pwm value for left and right motors without any validation
 * @param left_pwm_percent
 * @param right_pwm_percent
 */
void robot_set_raw_pwm_percent(MotionControl& motion_controller, double left_pwm_percent, double right_pwm_percent);

/**
 * Sets the pwm for each motor but validates its between -100 .. +100
 * and that neither motor is being asks to change direction without going through zero.
 * @param left_pwm_percent
 * @param right_pwm_percent
 */
void robot_set_pwm_percent(MotionControl& motion_controller, double left_pwm_percent, double right_pwm_percent);
/**
 * Set the desired rpm for each motor.
 * validation:
 *  -   motors have a min and max allowable value for rpm. The goal will be modified by
 *      those values.
 *  -   will get an error if the parameters imply the motor has to change direction and
 *      its current state is moving.
 *  -   if a closed loop controller is running these values will be presented as 'target'
 *      values.
 * @param left_rpm
 * @param right_rpm
 * @return
 */
bool robot_set_rpm(MotionControl& motion_controller, double left_rpm, double right_rpm);

/**
 * Is the equivalent of robot_set_raw_pwm(0.0, 0.0);
 */
void robot_stop_all(MotionControl& motion_controller);

#if 0
/***************************************************************************************************************
 *  Encoder sample collection - builds kinematic state of robot
 ***************************************************************************************************************/
/**
 * Setup a repeating timer to examine and save the values from the encoder isr function every sample_interval_us
 *
 * @param sample_interval_us
 */
void robot_start_encoder_sample_collection(uint64_t sample_interval_us);

/**
 * Performs the same function as the timer callback that is setup in the previous function
 * `robot_start_encoder_sample_collection()`.
 *
 * These should not both be used as they conflict.
 */
void robot_collect_encoder_samples();
#endif
/***************************************************************************************************************
 * json formatting
 * @TODO should probably be somewhere els
 ***************************************************************************************************************/
void tojson_encoder_samples(MotionControl& mc, transport::buffer::Handle buffer_h);


#endif