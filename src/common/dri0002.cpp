#undef FTRACE_ON
#include <enum.h>
#include <dri0002.h>
#include <cstdio>
#include <pico/types.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>

#include "trace.h"
#include "config.h"

static int side2index(const MotorSide side) {
	const int ix = side == MotorSide::left ? MOTOR_LEFT_DRI0002_SIDE - 1 : MOTOR_RIGHT_DRI0002_SIDE - 1;
	return ix;
}

void PwmPiPico::begin(const uint pwm_pin, const uint direction_pin, const uint wrap) {
	printf("DRI0002.begin\n");
	FTRACE("begin this: %p  pwm_pin %u direction_pin: %u wrap: %u\n", this, pwm_pin, direction_pin, wrap);
	m_direction_pin = direction_pin;
	m_pwm_pin = pwm_pin;
	m_wrap = wrap;
	gpio_init(m_direction_pin);
	gpio_set_dir(m_direction_pin, GPIO_OUT);
	m_direction = MotorDirection::forward;
	/**
	 * Not this. For a diff drive robot to go forward one motor must turn forward(clockwise) and the other
	 * must turn backwards(counter clockwise). In the robot controlled by this code this "opposite" direction
	 * requirement is handled in the polarity of the wiring between the motor and the DRI0002.
	 * 
	 * The software should set both direction pins to '1' to go forward and both direction pins to '0'
	 * to go backwards
	*/
	gpio_put(m_direction_pin, m_direction == MotorDirection::forward?true:0);

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
void PwmPiPico::set_level(const uint level) const
{
	// print_fmt("PicoPwm::set_level addr: %p pin: %u wrap:%u level:%u slice_num %u channel: %u direction: %d\n", 
	// 	this, m_pwm_pin, m_wrap, level, m_slice_num, m_channel, (int)m_direction);
	gpio_put(m_direction_pin, m_direction == MotorDirection::forward?true:0);
	pwm_set_chan_level(m_slice_num, m_channel, level);
	pwm_set_enabled(m_slice_num, true);
}
void PwmPiPico::set_pwm_percent(const double percent) const
{
	FTRACE("set_pwm_percent index:%d percent : %f\n", percent);
	ASSERT_PRINTF(((0.00 <= percent) && percent <= 100.00), "PiPicoPwm - set_pwm_percent percent out of range 0.00 .. 100.00 %f ", percent);
	const uint level = static_cast<uint>(65534.0 * percent / 100.0);
	this->set_level(level);
}
void PwmPiPico::set_direction(const MotorDirection direction)
{
	FTRACE("PwmPiPico::set_direction old dir: %d new dir %d ", m_direction, dir);
	// if(m_direction != dir) {
		FTRACE("PwmPiPico::set_diredction settig old dir: %d new dir %d ", m_direction, dir);
		m_direction = direction;
		const int bit = (m_direction == MotorDirection::forward)? 1: 0;
		FTRACE("PwmPiPico::set_direction bit %d", bit);
		gpio_put(m_direction_pin, bit);
	// }
}


DRI0002V1_4::DRI0002V1_4(){ /*NOLINT*/ }
DRI0002V1_4::DRI0002V1_4(const uint e1m1_side_index, const int pwm_E1_pin, int pin_M1, const uint e2m2_side_index, const int pwm_E2_pin, const int direction_M2_pin) {	//NOLINT
	begin(e1m1_side_index, pwm_E1_pin, pin_M1, e2m2_side_index, pwm_E2_pin, direction_M2_pin);
}

void DRI0002V1_4::begin(const uint e1m1_side_index, const int pwm_E1_pin, int direction_M1_pin, const uint e2m2_side_index, const int pwm_E2_pin, const int direction_M2_pin)
{
	ASSERT_PRINTF((((1 <= e1m1_side_index) && (e1m1_side_index <= 2))), "DRI0002 - e1m1_side_index out of range %d ", e1m1_side_index);
	ASSERT_PRINTF((((1 <= e2m2_side_index) && (e2m2_side_index <= 2))), "DRI0002 - e2m2_side_index out of range %d ", e2m2_side_index);
	ASSERT_PRINTF((((e1m1_side_index != e2m2_side_index))), "DRI0002 - e1m1_side_index must not equal e2m2_side_index %d %d ", e1m1_side_index, e2m2_side_index);

	m_pin_E1 = pwm_E1_pin;
	m_pin_M1 = direction_M1_pin;
	m_pin_E2 = pwm_E2_pin;
	m_pin_M2 = direction_M2_pin;
	// m_direction[0] = true;
	// m_direction[1] = true;
	m_pwm_1.begin(m_pin_E1, m_pin_M1, 65534);
	m_pwm_2.begin(m_pin_E2, m_pin_M2, 65534);
	m_sides[e1m1_side_index-1] = (e1m1_side_index-1 == 0) ? &m_pwm_1: &m_pwm_2;
	m_sides[e2m2_side_index-1] = (e2m2_side_index-1 == 0) ? &m_pwm_1: &m_pwm_2;
	m_sides[0]->set_direction(MotorDirection::forward);
	m_sides[1]->set_direction(MotorDirection::forward);
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
void DRI0002V1_4::set_pwm_percent(const MotorSide side, const double percent) const {
	FTRACE("set_pwm_percent index:%d percent : %f\n", side2value(side), percent);
	uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_pwm_percent index out of range %d ", local_index);
	ASSERT_PRINTF((((0.00 <= percent) && (percent <= 100.00))), "DRI0002 - set_pwm_percent percent out of range 0.00 .. 100.00 %f ", percent);
	const uint level = static_cast<uint>(65534) * percent / 100;
	m_sides[local_index]->set_level(level);
}
/**
 * direction of the motor is determined by the state of the DRI0002  m1 or m2 pin.
 * This code maps the value of 'MotorDirection direction' to m pin state as:
 * MotorDirection::forward   -> m pin high
 * MotorDirection::backwards -> m pin low
*/
void DRI0002V1_4::set_direction_pin_state(const MotorSide side, const MotorDirection direction) const {
	const uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - set_direction index out of range %d ", local_index);
	FTRACE("DRI0002 set_direction index: %d old direction: %d new direction: %d\n", side2value(side), (int)m_sides[local_index]->m_direction, (int)direction);
	m_sides[local_index]->set_direction(direction);
	FTRACE("DRI0002 set_direction index: %d direction: %d \n", side2value(side), (int)m_sides[local_index]->m_direction);
}

MotorDirection DRI0002V1_4::get_direction_pin_state(const MotorSide side) const {
	const uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - get_direction_pin_state %d ", local_index);
	return m_sides[local_index]->m_direction;
}
MotorDirection DRI0002V1_4::get_direction(const MotorSide side) const {
	const uint local_index = side2index(side);
	ASSERT_PRINTF((((0 <= local_index) && (local_index <= 1))), "DRI0002 - get direction %d ", local_index);
	return m_sides[local_index]->m_direction;
}
