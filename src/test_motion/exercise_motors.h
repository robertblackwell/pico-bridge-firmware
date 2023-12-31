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
#ifndef H_Exerciser_h 
#define H_Exerciser_h

#include "task.h"
#include "dri0002.h"

class ExerciseMotors: public Taskable {
	private:
	#define STATE_UP 1
	#define STATE_BETWEEN 2
	#define STATE_DOWN 3
	#define STATE_ENDWAIT 4
	int  m_state;
	
	int  m_min_pwm;
	long m_interval_between_steps_ms;
	long m_timeof_last_action_ms;

	int  m_count;
	int  m_endwait_max;
	int  m_between_max;
	int  m_pwm_step_size;
	int  m_pwm_value;

	DRI0002V1_4* m_dri0002_ptr;

	public:
	void begin(DRI0002V1_4 * dri0002);
	void run();
};

#endif