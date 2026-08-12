#ifndef H_robot_h
#define H_robot_h
#include "transport/buffers.h"
#include "dri0002.h"
#include "encoder_v2.h"
#include "motion.h"
#include "speed_control.h"

namespace robot {
    /**
     * Initializes all the data structures representing the hardware. All this is hidden inside robot::init
     */
    void init();

    /**
     * Starts encoder isr processing together with speed control loop if one is configured into the build
     */
    void start();

    /**
     * Must be called frequently in the main loop so that speed control data can be collected from
     * encoder isr's and speed control loop can process updates.
     */
    void poll();

    /**
    * Sets the pwm for each motor but validates its between -100 .. +100
    * and that neither motor is being asks to change direction without going through zero.
    * @param left_pwm_percent
    * @param right_pwm_percent
    */
    void set_pwm_percent(double left_pwm_percent, double right_pwm_percent);

    /**
     * Set a target wheel velocities. Relies on the speed controller to achive the targets.
     * Action on this call may not happen until the next speed control loop
     * @param left_velocity_target_ms
     * @param right_velocity_target_ms
     * @return
     */
    bool set_wheel_velocity_ms(double left_velocity_target_ms, double right_velocity_target_ms);

    /**
    * Is the equivalent of set_raw_pwm(0.0, 0.0)
    * In addition sets wheel velocity targets to zero.
    */
    void stop_all();

    /*********************************************************************************************
     * Accessing the left and right side of the drive
     ***********************************************************************************************/
    Encoder* get_encoder(DriveSide side);
    MotionControl::RpmValue get_current_rpm(DriveSide side);
    MotionControl::PwmValue get_current_pwm(DriveSide side);

    /*********************************************************************************************
     * functions to validate that pwm and rpm values are within acceptable ranges and
     * are not asking a motor to change direction while rotating
     ***********************************************************************************************/
    bool verify_side_rpm_settable(DriveSide side, float rpm);
    bool verify_side_pwm_settable(DriveSide side, float pwm);

    /*********************************************************************************************
     * these functions execute specific command that need access to drive details
     ***********************************************************************************************/
    /**
     * Sets the pwm value for left and right motors without any validation
     * @param left_pwm_percent
     * @param right_pwm_percent
     */
    void set_raw_pwm_percent(double left_pwm_percent, double right_pwm_percent);

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
    bool set_rpm(double left_rpm, double right_rpm);

    /***************************************************************************************************************
     * json formatting
     * @TODO should probably be somewhere els
     ***************************************************************************************************************/
    void tojson_encoder_samples(transport::buffer::Handle buffer_h);

} // namespace
#endif