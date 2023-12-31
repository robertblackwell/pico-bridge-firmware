/**
 * This is the second motor experiment for my robot.
 * Change the loop() function so that the ramp up and ramp down loops
 * are state machines
 * 
 */
 /**
  * Motor n 
  * MOTORn_PWM_PIN is the pin to which one writes the pwm value that controlls the motor speed.
  * The value written to this pin is in the range 0 .. 255
  * 
  * MOTORn_DIRECTION_SELECT_PIN is the mode pin which determines the direction of rotation of the motor.
  */
#include "exercise_motors.h"

void ExerciseMotors::begin(DRI0002V1_4 * dri0002) 
{
	m_dri0002_ptr = dri0002;
	m_min_pwm = 30; //min_pwm;
	m_pwm_step_size = 10;
	m_interval_between_steps_ms = 3000; //interval_between_steps_ms;
	m_timeof_last_action_ms = millis();
	m_pwm_value = m_min_pwm;
	m_state = STATE_UP;
	m_between_max = 4;
	m_endwait_max = 4;


}
void ExerciseMotors::run() {
	printf("ExerciseMotors::run\n");
	printf("ExerciseMotors time to do something state: %d\n", m_state);
	switch(m_state) {
		case STATE_UP: {
			printf("STATE_UP pwm % value : %f \n", (float)m_pwm_value);
			m_dri0002_ptr->set_pwm_percent(MotorSide::left, (float)(m_pwm_value));
			m_dri0002_ptr->set_pwm_percent(MotorSide::right, (float)(m_pwm_value));
			m_pwm_value += m_pwm_step_size;
			if(m_pwm_value > 100) {
				printf("state change to between\n");
				m_state = STATE_BETWEEN;
				m_count = 0;
			}
		} 
		break;
		case STATE_BETWEEN: {
			printf("STATE_BETWEEN count: %d \n", m_count);
			m_count += 1;
			if(m_count > m_between_max) {
				m_dri0002_ptr->set_pwm_percent(MotorSide::left, 0.0);
				m_dri0002_ptr->set_pwm_percent(MotorSide::right, 0.0);
				m_state = STATE_DOWN;
				m_pwm_value = 100;
			}
		}
		break;
		case STATE_DOWN: {
			printf("STATE_DOWN pwm % value : %f \n", (float)m_pwm_value);
			m_dri0002_ptr->set_pwm_percent(MotorSide::left, (float)(m_pwm_value));
			m_dri0002_ptr->set_pwm_percent(MotorSide::right, (float)(m_pwm_value));
			m_pwm_value -= m_pwm_step_size;
			if(m_pwm_value < m_min_pwm) {
				printf("state change to endwait\n");
				m_state = STATE_ENDWAIT;
				m_count = 0;
				m_pwm_value = m_min_pwm;
			}
		}
		break;
		case STATE_ENDWAIT: {
			printf("STATE_ENDWAIT count: %d\n", m_count);
			m_count += 1;
			if(m_count > m_endwait_max) {
				m_dri0002_ptr->set_pwm_percent(MotorSide::left, 0.0);
				m_dri0002_ptr->set_pwm_percent(MotorSide::right, 0.0);
				m_state = STATE_UP;
				m_pwm_value = 0;
			}
		}
		break;
		default:
			while(1)
				printf("bad state\n");

	}
}

