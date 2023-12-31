#undef FTRACE_ON
#include <enum.h>
#include <dri0002.h>
#include <stdio.h>
#include <stdint.h>
#include <pico/types.h>

#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <utils.h>

#include "config.h"
int side2index(MotorSide side) {
	int ix = (side == MotorSide::left) ? MOTOR_LEFT_DRI0002_SIDE - 1 : MOTOR_RIGHT_DRI0002_SIDE - 1;
	return ix;
}
int side2value(MotorSide side) {
	int v = (side == MotorSide::left) ? MOTOR_LEFT_DRI0002_SIDE : MOTOR_RIGHT_DRI0002_SIDE ;
	return v;
}
void PwmPiPico::begin(uint pwm_pin, uint direction_pin, uint wrap) {
	printf("DRI0002.begin\n");
	FTRACE("begin this: %p  pwm_pin %u direction_pin: %u wrap: %u\n", this, pwm_pin, direction_pin, wrap);
	m_direction_pin = direction_pin;
	m_pwm_pin = pwm_pin;
	m_wrap = wrap;
	gpio_init(m_direction_pin);
	gpio_set_dir(m_direction_pin, GPIO_OUT);
	m_direction = true;
	gpio_put(m_direction_pin, (m_direction)?1:0);

	m_slice_num = pwm_gpio_to_slice_num(m_pwm_pin);
	m_channel = pwm_gpio_to_channel(m_pwm_pin);
	pwm_set_chan_level(m_slice_num, m_channel, 0);
	gpio_set_function(m_pwm_pin, GPIO_FUNC_PWM);
	
	m_slice_num = pwm_gpio_to_slice_num(m_pwm_pin);
	m_channel = pwm_gpio_to_channel(m_pwm_pin);
	pwm_set_enabled(m_slice_num, false);

	pwm_config config = pwm_get_default_config();
	FTRACE("default config csr:%u, div: %u, top: %u \n", config.csr, config.div, config.top)
	pwm_config_set_clkdiv(&config, 1.f);
	pwm_config_set_wrap(&config, 65534);
	pwm_init(m_slice_num, &config, false);
	FTRACE("default config csr:%u, div: %u, top: %u \n", config.csr, config.div, config.top)
	pwm_set_chan_level(m_slice_num, m_channel, 0);
	pwm_set_enabled(m_slice_num, true);
}
void PwmPiPico::set_level(uint level)
{
	FTRACE("PicoPwm::set_level addr: %p pin: %u wrap:%u level:%u slice_num %u channel: %u direction: %d\n", 
		this, m_pwm_pin, m_wrap, level, m_slice_num, m_channel, (int)m_direction)
	gpio_put(m_direction_pin, (m_direction)?1:0);
	pwm_set_chan_level(m_slice_num, m_channel, level);
	pwm_set_enabled(m_slice_num, true);
}
void PwmPiPico::set_pwm_percent(double percent)
{
	FTRACE("set_pwm_percent index:%d percent : %f\n", percent);
	ASSERT_PRINTF((((0.00 <= percent) && (percent <= 100.00))), "PiPicoPwm - set_pwm_percent percent out of range 0.00 .. 100.00 %f ", percent);
	uint level = (uint)65534 * percent / 100;
	this->set_level(level);
}
void PwmPiPico::set_direction(bool dir)
{
	FTRACE("PwmPiPico old dir: %d new dir %d \n", m_direction, dir);
	if(m_direction != dir) {
		FTRACE("PwmPiPico old dir: %d new dir %d \n", m_direction, dir);
		m_direction = dir;
		gpio_put(m_direction_pin, (m_direction)? 1: 0);
	}
}
void PwmPiPico::set_duty_cycle_percent(double percent)
{

}

DRI0002V1_4::DRI0002V1_4(){}
DRI0002V1_4::DRI0002V1_4(uint e1m1_side_index, int pin_E1, int pin_M1, uint e2m2_side_index, int pin_E2, int pin_M2)
{
	begin(e1m1_side_index, pin_E1, pin_M1, e2m2_side_index, pin_E2, pin_M2);
}

void DRI0002V1_4::begin(uint e1m1_side_index, int pin_E1, int pin_M1, uint e2m2_side_index, int pin_E2, int pin_M2)
{
	ASSERT_PRINTF((((1 <= e1m1_side_index) && (e1m1_side_index <= 2))), "DRI0002 - e1m1_side_index out of range %d ", e1m1_side_index);
	ASSERT_PRINTF((((1 <= e2m2_side_index) && (e2m2_side_index <= 2))), "DRI0002 - e2m2_side_index out of range %d ", e2m2_side_index);
	ASSERT_PRINTF((((e1m1_side_index != e2m2_side_index))), "DRI0002 - e1m1_side_index must not equal e2m2_side_index %d %d ", e1m1_side_index, e2m2_side_index);

	m_pin_E1 = pin_E1;
	m_pin_M1 = pin_M1;
	m_pin_E2 = pin_E2;
	m_pin_M2 = pin_M2;
	// m_direction[0] = true;
	// m_direction[1] = true;
	m_pwm_1.begin(m_pin_E1, m_pin_M1, 65534);
	m_pwm_2.begin(m_pin_E2, m_pin_M2, 65534);
	m_sides[e1m1_side_index-1] = (e1m1_side_index-1 == 0) ? &m_pwm_1: &m_pwm_2;
	m_sides[e2m2_side_index-1] = (e2m2_side_index-1 == 0) ? &m_pwm_1: &m_pwm_2;
	m_sides[0]->set_direction(true);
	m_sides[1]->set_direction(false);
	FTRACE("DRI0002:  %p \n\te1m1_index %d pin_E1: %d pin_M1: %d \n\te2m2_index: %d pin_E2: %d pin_M2: %d\n", 
		this, e1m1_side_index, m_pin_E1, m_pin_M1, e2m2_side_index, m_pin_E2, m_pin_M2)
}
/**
 * index is either 1 or 2
 * percent is a floating point number -100.0 .. 100.0
 * -ve pwm_percent means rotate 'backwards' m-pin low
 * +ve pwm_percent means rotate 'forwards' m-pin high
 * direction of rotation cannot be changes via a pwm setting using an instance of DRI0002V1_4
*/
void DRI0002V1_4::set_pwm_percent(MotorSide side, double percent)
{
	FTRACE("set_pwm_percent index:%d percent : %f\n", side2value(side), percent);
	uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_pwm_percent index out of range %d ", local_index);
	ASSERT_PRINTF((((0.00 <= percent) && (percent <= 100.00))), "DRI0002 - set_pwm_percent percent out of range 0.00 .. 100.00 %f ", percent);
	uint level = (uint)65534 * percent / 100;
	m_sides[local_index]->set_level(level);
}
/**
 * direction of the motor is determined by the state of the DRI0002  m1 or m2 pin.
 * This code mapds the value of 'bool direction' tp m pin state as:
 * true -> m pin high
 * false-> m pin low
*/
void DRI0002V1_4::set_direction_pin_state(MotorSide side, bool direction)
{
	uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_direction index out of range %d ", local_index);
	FTRACE("DRI0002 set_direction index: %d old direction: %d new direction: %d\n", side2value(side), (int)m_sides[local_index]->m_direction, (int)direction);
	m_sides[local_index]->set_direction(direction);
	FTRACE("DRI0002 set_direction index: %d direction: %d \n", side2value(side), (int)m_sides[local_index]->m_direction);
}
bool DRI0002V1_4::get_direction_pin_state(MotorSide side)
{
	uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_pwm_percent index out of range %d ", local_index);
	return m_sides[local_index]->m_direction;
}
PwmPiPico* DRI0002V1_4::get_pwmpipico(MotorSide side)
{
	uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_pwm_percent index out of range %d ", local_index);
	return m_sides[local_index];
}