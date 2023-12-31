#undef FTRACE_ON
#include "config.h"
#include <enum.h>
#include "utils.h"
#include <motion.h>
#include <task.h>
#include <dri0002.h>
#include <encoder.h>
#include <pid.h>


void MotionControl::set_rpm_one_side(Side& side, RpmValue new_request)
{
	FTRACE("set_rpm_one_side enter side:%p side.rpm_value value: %f side.is_zero: %d side.direction: %s side.raw: %f \n",
		&side, side.m_rpm_value.value, (int)side.m_rpm_value.is_zero, 
		motor_direction_to_string(side.m_rpm_value.direction), side.m_rpm_value.raw_double_value());

	FTRACE("set_rpm_one_side enter side:%p nr.rpm_value value: %f nr.is_zero: %d nr.direction: %s nr.raw: %f \n",
		&side, new_request.value, (int)new_request.is_zero, 
		motor_direction_to_string(new_request.direction), new_request.raw_double_value());

	RpmValue result{0.0};
	
	if((! new_request.is_zero) && (! side.m_rpm_value.is_zero)) {
		if(new_request.direction == side.m_rpm_value.direction) {
			// changing speed same direction
			FTRACE("already running change speed\n");
			side.m_rpm_target = new_request.raw_double_value();
			side.m_rpm_value = new_request;
			side.m_pid.change_target(side.m_rpm_value.value);
			RpmValue& ss = side.m_rpm_value;
			FTRACE("AA set_rpm_one_side %p rpm_value value: %f is_zero: %d direction: %s raw: %f m_pid.m_target% f\n",
				&ss, ss.value, (int)ss.is_zero, 
				motor_direction_to_string(ss.direction), ss.raw_double_value(), side.m_pid.get_target());
			// call pid_run
		} else {
			// changing rpm and direction when target rpm is not zero - for the moment fatal error
			FATAL_ERROR_MSG("changing direction when target is not zero - for the moment fatal error")
		}
	} else if((! new_request.is_zero) && (side.m_rpm_value.is_zero)) {
		// current target is zero and setting non zero new speed and maybe a change
		// in direction
		FTRACE("getting started\n");
		side.m_rpm_target = new_request.raw_double_value();
		side.m_rpm_value = new_request;
		side.m_pid.change_target(side.m_rpm_value.value);
		side.m_pid_active = true;
		RpmValue& ss = side.m_rpm_value;

		// set the direction
		FTRACE("before pin state\n");
		bool direction_pin_state = motor_direction_to_pin_state(side.m_dri0002_motor_side, new_request.direction);
		FTRACE("after pin state motor_side %s  nr.dir% s pin% d\n", to_string(side.m_dri0002_motor_side), motor_direction_to_string(new_request.direction), (int) direction_pin_state);
		m_dri0002_ptr->set_direction_pin_state(side.m_dri0002_motor_side, direction_pin_state);
		
		FTRACE("BB set_rpm_one_side %p rpm_value value: %f is_zero: %d direction: %s raw: %f m_pid.m_target% f\n",
			&ss, ss.value, (int)ss.is_zero, 
			motor_direction_to_string(ss.direction), ss.raw_double_value(), side.m_pid.get_target());
		// call pid_run
		return;
	} else if((new_request.is_zero) && (! side.m_rpm_value.is_zero)) {
		//current target is not zero but going to zero
		FTRACE("going to zero\n");
		side.m_rpm_target = new_request.raw_double_value();
		side.m_rpm_value = new_request;
		side.m_pid.change_target(side.m_rpm_value.value);
		side.m_pid_active = false;
		m_dri0002_ptr->set_pwm_percent( side.m_dri0002_motor_side, 0.0);

		bool direction_pin_state = motor_direction_to_pin_state(side.m_dri0002_motor_side, new_request.direction);
		m_dri0002_ptr->set_direction_pin_state(side.m_dri0002_motor_side, direction_pin_state);

		RpmValue& ss = side.m_rpm_value;
		FTRACE("CC set_rpm_one_side %p rpm_value value: %f is_zero: %d direction: %s raw: %f m_pid.m_target% f\n",
			&ss, ss.value, (int)ss.is_zero, 
			motor_direction_to_string(ss.direction), ss.raw_double_value(), side.m_pid.get_target());
	} else {
		FTRACE("DD set_rpm_one_side fall thru \n");
	}
}


MotionControl::MotionControl()
{
};
MotionControl::MotionControl(DRI0002V1_4 *dri0002, Encoder *encoder_left_ptr, Encoder *encoder_right_ptr)
{
	begin(dri0002, encoder_left_ptr, encoder_right_ptr);
}
MotionControl::MotionControl(DRI0002V1_4 &dri0002, Encoder &encoder_left, Encoder &encoder_right)
{
	begin(&dri0002, &encoder_left, &encoder_right);
}

void MotionControl::begin(DRI0002V1_4 *dri0002, Encoder *encoder_left_ptr, Encoder *encoder_right_ptr)
{
    m_dri0002_ptr = dri0002;
    
	m_left_side.m_encoder_ptr = encoder_left_ptr;
	m_left_side.m_pid.begin(PID_LEFT_KP_DEFAULT, PID_LEFT_KI_DEFAULT, PID_LEFT_KD_DEFAULT, 0.0, 0.0);
	m_left_side.m_pid_active = false;
	m_left_side.m_rpm_target = 0.0;
	m_left_side.m_rpm_value = RpmValue{0.0};
	m_left_side.m_dri0002_motor_side = MotorSide::left;
	m_left_side.m_default_direction = true;
    
	m_right_side.m_encoder_ptr = encoder_right_ptr;
	m_right_side.m_pid.begin(PID_RIGHT_KP_DEFAULT, PID_RIGHT_KI_DEFAULT, PID_RIGHT_KD_DEFAULT, 0.0, 0.0);
	m_right_side.m_pid_active = false;	
	m_right_side.m_rpm_target = 0.0;
	m_right_side.m_rpm_value = RpmValue{0.0};
	m_right_side.m_dri0002_motor_side = MotorSide::right;
	m_left_side.m_default_direction = true;
}
// void begin(Motor& motor_01, Motor& motor_02, Encoder& encoder_01, Encoder& encoder_02);
void MotionControl::set_pwm_direction(double pwm_motor_left, MotorDirection direction_left, double pwm_motor_right, MotorDirection direction_right)
{
    FTRACE("motion::set_pwm_direction left_percent: %f right_percent: %f left_dir: %s right_dir: %s\n",
            pwm_motor_left, pwm_motor_right, 
			motor_direction_to_string(direction_left), motor_direction_to_string(direction_right));
}
/**
 * The rpm values are managed for 'reasonablness' and also carry direction information
 *
 * 	-	rpm > 0.0 means 'forward' rpm < 0.0 means 'backwards'
 * 	-	
 * 
*/
void MotionControl::pid_set_rpm(double left_rpm, double right_rpm)
{
	FTRACE("MotionControl::pid_set_rpm left: %f right: %f \n", left_rpm, right_rpm);
	set_rpm_one_side(m_left_side, RpmValue{left_rpm});
	set_rpm_one_side(m_right_side, RpmValue{right_rpm});
}

void MotionControl::set_pwm_percent(double percent_1, double percent_2)
{
    FTRACE("Motion::set_pwm_percent percent_1: %f percent_2: %f\n", percent_1, percent_2);
    m_dri0002_ptr->set_pwm_percent(MotorSide::left, (percent_1 >= 0.00) ? percent_1 : -percent_1);
    m_dri0002_ptr->set_pwm_percent(MotorSide::right, (percent_2 >= 0.00) ? percent_2 : -percent_2);
}
void MotionControl::stop_all()
{
    m_dri0002_ptr->set_pwm_percent(MotorSide::left, 0.0);
    m_dri0002_ptr->set_pwm_percent(MotorSide::right, 0.0);
}
void MotionControl::update_pid(double kp, double ki, double kd)
{
	m_left_side.m_pid.change_constants(kp, ki, kd);
	m_right_side.m_pid.change_constants(kp, ki, kd);
}

void MotionControl::run()
{
	bool got_some = false;
	if(! (m_left_side.m_pid_active || m_right_side.m_pid_active)) {
		return;
	}
	apply_pid();
}
void MotionControl::apply_pid()
{
	bool got_some = false;
	EncoderSample left_sample;
	m_left_side.m_encoder_ptr->run();
	double left_rpm = 0.0;
	if(m_left_side.m_encoder_ptr->available()) {
		FTRACE("Reporter.run - got a left sample\n"," ")
		m_left_side.m_encoder_ptr->consume(left_sample);
		got_some = true;
		left_rpm = left_sample.s_motor_rpm;
	}
	EncoderSample right_sample;
	m_right_side.m_encoder_ptr->run();
	double right_rpm = 0.0;
	if(m_right_side.m_encoder_ptr->available()) {
		FTRACE("Reporter.run - got a right sample\n", " ")
		m_right_side.m_encoder_ptr->consume(right_sample);
		got_some = true;
		right_rpm = right_sample.s_motor_rpm;
	}

	double new_left_pwm = (m_left_side.m_pid_active) ? m_left_side.m_pid.update(left_rpm): 0.0;
	double new_right_pwm = (m_right_side.m_pid_active) ? m_right_side.m_pid.update(right_rpm): 0.0;
	printf("left: pid: %p m_target: %f rpm: %f pwm: %f\n", &m_left_side.m_pid, m_left_side.m_pid.get_target(), left_rpm, new_left_pwm);
    printf("right:pid: %p m_target: %f rpm: %f pwm: %f\n", &m_right_side.m_pid, m_right_side.m_pid.get_target(), right_rpm, new_right_pwm);

	printf("pid loop left_target: %f left_rpm: %f left_pwm: %f right_target: %f right_rpm: %f right_pwm: %f\n",
		m_left_side.m_pid.get_target(), left_rpm, new_left_pwm,
		m_right_side.m_pid.get_target(), right_rpm, new_right_pwm
	);
	this->set_pwm_percent(new_left_pwm, new_right_pwm);
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
			mp->m_dri0002_ptr->get_pwmpipico(MotorSide::left)->m_pwm_pin,
			mp->m_dri0002_ptr->get_pwmpipico(MotorSide::left)->m_direction_pin);
		printf("\tm_isr_ainterupt a: %d \n\tinterrupt b: %d\n",
			mp->m_left_side.m_encoder_ptr->m_encoder_a_pin,
			mp->m_left_side.m_encoder_ptr->m_encoder_b_pin
		);
		printf("\tm_isr_a: %p \n\tm_isr_b: %p\n",
			mp->m_left_side.m_encoder_ptr->m_isr_a,
			mp->m_left_side.m_encoder_ptr->m_isr_b
		);
		// printf("\tirq_handler_left_a: %p \n\tirq_handler_left_b: %p\n",
		// 	irq_handler_left_apin,
		// 	irq_handler_left_bpin
		// );
	printf("Right Config \n");
		printf("\tE pin %d \n\tM pin: %d \n", 
			mp->m_dri0002_ptr->get_pwmpipico(MotorSide::right)->m_pwm_pin,
			mp->m_dri0002_ptr->get_pwmpipico(MotorSide::right)->m_direction_pin);
		printf("\tinterupt a: %d \n\tinterrupt b: %d\n",
			mp->m_right_side.m_encoder_ptr->m_encoder_a_pin,
			mp->m_right_side.m_encoder_ptr->m_encoder_b_pin);
		printf("\tm_isr_a: %p \n\tm_isr_b: %p\n",
			mp->m_right_side.m_encoder_ptr->m_isr_a,
			mp->m_right_side.m_encoder_ptr->m_isr_b
		);
		// printf("\tirq_handler_right_a: %p \n\tirq_handler_right_b: %p\n",
		// 	irq_handler_right_apin,
		// 	irq_handler_right_bpin
		// );
}