#ifndef H_dri0002v1_4_h
#define H_dri0002v1_4_h
#include <stdio.h>
#include <stdint.h>
#include <pico/types.h>
#include <config.h>
#include <enum.h>

/**
 * This class interfaces to the pwm functions on a raspberry pi pico
*/
struct PwmPiPico 
{

	uint m_pwm_pin;
	uint m_wrap;
	uint m_slice_num;
	uint m_channel;
	uint m_direction_pin;
	bool m_direction;
	void begin(uint pwm_pin, uint direction_pin, uint wrap);
	void set_level(uint level);
	/**
	 * index is either 1 or 2
	 * percent is a floating point number -100.0 .. 100.0
	 * -ve pwm_percent means rotate 'backwards' m-pin low
	 * +ve pwm_percent means rotate 'forwards' m-pin high
	 * direction of rotation cannot be changes via a pwm setting using an instance of DRI0002V1_4
	*/
	void set_pwm_percent(double percent);
	void set_direction(bool dir_pin_state);
	void set_duty_cycle_percent(double percent);
};
/**
 * This class is a controller for the DFRobot part DRI0002 V1.4 a dual motor H-Bridge motor controller.
 * This part has 4 control pins E1, M1, E2, M2 which are described in the following link.
 * https://wiki.dfrobot.com/MD1.3_2A_Dual_Motor_Controller_SKU_DRI0002
 * 
 * E1, M1 control the dc motor attached to the pair of output terminals on the same side of the board as E1 M1
 * E2, M2 control a second dc motor attached to the output terminals on the other side opf the board
 * 
 * Mn controls whether the motor will turn forwards or backwards - that is direction
 * En is driven by a pwm signal to control motor speed
 * 
*/
class DRI0002V1_4
{
	public:
	
	DRI0002V1_4();
	DRI0002V1_4(uint e1m1_side_index, int pwm_E1_pin, int direction_M1_pin, uint e2m2_side_index, int pwm_E2_pin, int direction_M2_pin);
	void begin(uint e1m1_side_index, int pwm_E1_pin, int direction_M1_pin, uint e2m2_side_index, int pwm_E2_pin, int direction_M2_pin);
	/**
	 * index is either 1 or 2
	 * percent is a floating point number -100.0 .. 100.0
	 * -ve pwm_percent means rotate 'backwards' m-pin low
	 * +ve pwm_percent means rotate 'forwards' m-pin high
	 * direction of rotation cannot be changes via a pwm setting using an instance of DRI0002V1_4
	*/
	void set_pwm_percent(MotorSide side, double percent);
	/**
	 * direction of the motor is determined by the state of the DRI0002  m1 or m2 pin.
	 * This code mapds the value of 'bool direction' tp m pin state as:
	 * true -> m pin high
	 * false-> m pin low
	*/
	void set_direction_pin_state(MotorSide side, bool direction_pin_state);
	bool get_direction_pin_state(MotorSide side);
	PwmPiPico* get_pwmpipico(MotorSide side);

	private:
	int m_pin_E1;
	int m_pin_M1;

	int m_pin_E2;
	int m_pin_M2;
	PwmPiPico m_pwm_1;
	PwmPiPico m_pwm_2;
	PwmPiPico* m_sides[2];
	bool m_direction[2];


};
#endif