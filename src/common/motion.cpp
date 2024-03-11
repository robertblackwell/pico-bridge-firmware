#undef FTRACE_ON
#include <cmath>
#include "config.h"
#include <enum.h>
#include "trace.h"
#include "motion.h"
#include "task.h"
#include "dri0002.h"
#include "encoder.h"
#include "pid.h"


void MotionControl::set_rpm_one_side(DriveSide side, RpmValue new_request)
{
    /**
     * @TODO - this function can be simplified
     */

	RpmValue trpm = get_rpm_target(side);
	if((! new_request.is_zero()) && (! trpm.is_zero())) {
        if (new_request.direction() != trpm.direction()) {
            FATAL_ERROR_MSG("changing direction when target is not zero and wheel is in motion- for the moment fatal error")
        } else {
			set_rpm_target(side, new_request);
			RpmValue ss = get_rpm_target(side);
            /**
             * Pass the new target to the controller somehow
             */
		}
	} else if((! new_request.is_zero()) && (trpm.is_zero())) {
        set_rpm_target(side, new_request);
        RpmValue ss = get_rpm_target(side);
        // ensure direction pins are corrent
        m_dri0002_ptr->set_direction_pin_state(side, new_request.direction());

        /**
         * tell the closed loop controller start
         */
		return;
	} else if((new_request.is_zero()) && (! trpm.is_zero())) {
        set_rpm_target(side, new_request);
        RpmValue ss = get_rpm_target(side);
        /**
         * Tell the closed loop controller stop
         */
		m_dri0002_ptr->set_pwm_percent( side, 0.0);
		m_dri0002_ptr->set_direction_pin_state(side, new_request.direction());
	} else {
        FATAL_ERROR_MSG("should not get herer")
	}
}


MotionControl::MotionControl()
{
    m_dri0002_ptr = nullptr;
    m_left_encoder_ptr = nullptr;
    m_right_encoder_ptr = nullptr;
    // m_left_closed_loop_controller_ptr = nullptr;
    // m_right_closed_loop_controller_ptr = nullptr;
};
MotionControl::MotionControl( //NOLINT
        DRI0002V1_4 *dri0002,
        Encoder *encoder_left_ptr, Encoder *encoder_right_ptr
        // CustomController* left_closed_loop_controller,
        // CustomController* right_closed_loop_controller
        )
{
    m_dri0002_ptr = nullptr;
	begin(
        dri0002,
        encoder_left_ptr, encoder_right_ptr
        // left_closed_loop_controller,
        // right_closed_loop_controller
        );
}


void MotionControl::begin(
        DRI0002V1_4 *dri0002,
        Encoder *encoder_left_ptr, Encoder *encoder_right_ptr
        // CustomController* left_closed_loop_controller_ptr,
        // CustomController* right_closed_loop_controller_ptr
        )
{
    m_dri0002_ptr = dri0002;
    m_left_encoder_ptr = encoder_left_ptr;
    m_right_encoder_ptr = encoder_right_ptr;
    // m_left_closed_loop_controller_ptr = left_closed_loop_controller_ptr;
    // m_right_closed_loop_controller_ptr = right_closed_loop_controller_ptr;
}
/**
 * The rpm values are managed for 'reasonablness' and also carry direction information
 *
 * 	-	rpm > 0.0 means 'forward' rpm < 0.0 means 'backwards'
 * 	-	
 * 
*/
bool MotionControl::verify_side_rpm_settable(DriveSide side, float rpm) const
{
    MotionControl::RpmValue new_request{rpm};

    if((! new_request.is_zero()) && (! this->get_rpm_target(side).is_zero())) {
        return (new_request.direction() == get_rpm_target(side).direction());
    }
    return true;
}
bool MotionControl::verify_left_rpm_settable(float rpm_value) const
{
    return verify_side_rpm_settable(DriveSide::left, rpm_value);
}
bool MotionControl::verify_right_rpm_settable(float rpm_value) const
{
    return verify_side_rpm_settable(DriveSide::right, rpm_value);
}
void MotionControl::pid_set_rpm(double left_rpm, double right_rpm) {
	FTRACE("MotionControl::pid_set_rpm left: %f right: %f \n", left_rpm, right_rpm);
	set_rpm_one_side(DriveSide::left, RpmValue{left_rpm});
	set_rpm_one_side(DriveSide::right, RpmValue{right_rpm});
}
void MotionControl::set_raw_pwm_percent(double percent_1, double percent_2) const
{
    FTRACE("Motion::set_pwm_percent percent_1: %f percent_2: %f\n", percent_1, percent_2);
	MotorDirection dl = (percent_1 > 0.0) ? MotorDirection::forward : MotorDirection::backwards;
	MotorDirection dr = (percent_2 > 0.0) ? MotorDirection::forward : MotorDirection::backwards;
	FTRACE("dl %s dr %s \n", motor_direction_to_string(dl), motor_direction_to_string(dr))
	m_dri0002_ptr->set_direction_pin_state(DriveSide ::left, dl);
	m_dri0002_ptr->set_direction_pin_state(DriveSide ::right, dr);
    m_dri0002_ptr->set_pwm_percent(DriveSide ::left, fabs(percent_1));
    m_dri0002_ptr->set_pwm_percent(DriveSide ::right, fabs(percent_2));
}
void MotionControl::set_pwm_percent(double percent_1, double percent_2) const
{
    FTRACE("Motion::set_pwm_percent percent_1: %f percent_2: %f\n", percent_1, percent_2);
    m_dri0002_ptr->set_pwm_percent(DriveSide ::left, (percent_1 >= 0.00) ? percent_1 : -percent_1);
    m_dri0002_ptr->set_pwm_percent(DriveSide ::right, (percent_2 >= 0.00) ? percent_2 : -percent_2);
}
void MotionControl::stop_all() const
{
    m_dri0002_ptr->set_pwm_percent(DriveSide ::left, 0.0);
    m_dri0002_ptr->set_pwm_percent(DriveSide ::right, 0.0);
}
/**
 * This function dumps the config associated with pin assignments
 * and interrupt handlers.
*/
void dump_config(MotionControl* mp)
{
	printf("Config dump\n");
	printf("Left Config \n");
		printf("\tE pin %d \n\tM pin: %d \n", 
			mp->m_dri0002_ptr->get_pwmpipico(DriveSide ::left)->m_pwm_pin,
			mp->m_dri0002_ptr->get_pwmpipico(DriveSide ::left)->m_direction_pin);
		printf("\tm_isr_ainterupt a: %d \n\tinterrupt b: %d\n",
			mp->encoder_ptr(DriveSide::left)->m_encoder_a_pin,
            mp->encoder_ptr(DriveSide::right)->m_encoder_a_pin
		);
		printf("\tm_isr_a: %p \n\tm_isr_b: %p\n",
			mp->encoder_ptr(DriveSide::left)->m_isr_a,
			mp->encoder_ptr(DriveSide::right)->m_isr_b
		);
		// printf("\tirq_handler_left_a: %p \n\tirq_handler_left_b: %p\n",
		// 	irq_handler_left_apin,
		// 	irq_handler_left_bpin
		// );
	printf("Right Config \n");
		printf("\tE pin %d \n\tM pin: %d \n", 
			mp->m_dri0002_ptr->get_pwmpipico(DriveSide ::right)->m_pwm_pin,
			mp->m_dri0002_ptr->get_pwmpipico(DriveSide ::right)->m_direction_pin);
		printf("\tinterupt a: %d \n\tinterrupt b: %d\n",
			mp->encoder_ptr(DriveSide::left)->m_encoder_a_pin,
			mp->encoder_ptr(DriveSide::right)->m_encoder_b_pin);
		printf("\tm_isr_a: %p \n\tm_isr_b: %p\n",
			mp->encoder_ptr(DriveSide::left)->m_isr_a,
			mp->encoder_ptr(DriveSide::right)->m_isr_b
		);
		// printf("\tirq_handler_right_a: %p \n\tirq_handler_right_b: %p\n",
		// 	irq_handler_right_apin,
		// 	irq_handler_right_bpin
		// );
}