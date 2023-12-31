#ifndef H_robot_pico_pwm_h
#define H_robot_pico_pwm_h
#define TRACE_ON
#include <stdint.h>
#include <pico/stdio.h>
#include <pico/types.h>
#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include "/home/robert/pico/pico/pico-sdk/src/rp2_common/hardware_pwm/include/hardware/pwm.h"

/**
 * I got the following function from the site: https://www.i-programmer.info/programming/hardware/14849-the-pico-in-c-basic-pwm.html?start=2
 * 
 * 125,000,000 / 3 is 41
 * 
*/
static uint32_t pwm_set_freq_duty(uint slice_num, uint channel_num, uint32_t freq, int d)
{
	uint32_t clock = 125000000;
	uint32_t divider16 = clock / freq / 4096 + (clock % (freq * 4096) != 0);
	if(divider16 / 16 == 0) {
		divider16 = 16;
	}
	uint32_t wrap = clock * 16 / divider16 / freq - 1;
	pwm_set_clkdiv_int_frac(slice_num, divider16/16, divider16 & 0xF);
	pwm_set_wrap(slice_num, wrap);
	pwm_set_chan_level(slice_num, channel_num, wrap * d / 100);
	return wrap;
}

class PicoPwm
{
	public:
	uint m_gpio_pin;
	int m_clock_divider;
	int m_wrap;
	uint16_t m_last_level;
	int m_slice;
	int m_channel;
	PicoPwm(){}
	void begin(uint gpio_pin, uint16_t wrap_value) 
	{
		gpio_set_function(gpio_pin, GPIO_FUNC_PWM);
		m_gpio_pin = gpio_pin;
		m_slice = pwm_gpio_to_slice_num(gpio_pin);
		m_channel = pwm_gpio_to_channel(gpio_pin);
		// printf("PicoPwm::begin m_slice %d m_channel %d \n", m_slice, m_channel);
		pwm_set_clkdiv_int_frac(m_slice, 3, 0);
		pwm_config c = pwm_get_default_config();
		m_wrap = wrap_value;
		pwm_set_enabled(m_slice, true);
		pwm_set_chan_level(m_slice, m_channel, 0);
		pwm_set_wrap(m_slice, m_wrap);
	}
	void enable()
	{
		pwm_set_enabled(m_slice, true);
	}
	void disable()
	{
		pwm_set_enabled(m_slice, false);
	}
	void set_level(uint16_t level)
	{
		pwm_set_chan_level(m_slice, m_channel, level);
	}
	void set_duty_cycle(double duty_cycle_percentage)
	{
		m_last_level = (uint16_t)(duty_cycle_percentage * (double)m_wrap);
		pwm_set_chan_level(m_slice, m_channel, m_last_level);
	}
};
#endif