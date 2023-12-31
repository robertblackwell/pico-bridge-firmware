#ifndef H_motor_h
#define H_motor_h
// #include "Arduino.h"
#include "pico_pwm.h"

class Motor{
	public:
	enum DirectionEnum {
		Forward = 11,
		Reverse = 12
	};

	Motor();
	void begin(int motor_id, const char* name, int an_direction_select_pin, int a_pwm_pin);
	void set_direction(DirectionEnum direction);
	void change_direction();
	void set_pwmvalue(uint16_t pwmvalue);
	void set_duty_cycle(double duty_cycle_percent);

	// private:

	int direction_select_pin;
	int pwm_pin;
	int last_pwm_value;
	double last_duty_cycle;
	DirectionEnum m_direction;
	const char* m_name;
	int m_id;
    PicoPwm m_pico_pwm;
	int m_pwm_wrap_value;
	int m_pwm_clock_divider;
};
#endif