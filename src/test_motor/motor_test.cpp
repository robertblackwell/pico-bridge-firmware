#undef TRACE_ON
#include <stdio.h>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>
#include <utils.h>
#include <dri0002.h>
#include "config.h"
// historical record
#if 0
void pwm_pin(uint pin, uint wrap, uint level)
{
    // Tell GPIO 0 and 1 they are allocated to the PWM
    gpio_set_function(pin, GPIO_FUNC_PWM);
	// gpio_set_function(1, GPIO_FUNC_PWM);

    // Find out which PWM slice is connected to GPIO 0 (it's slice 0)
    uint slice_num = pwm_gpio_to_slice_num(pin);
	uint chan_num = pwm_gpio_to_channel(pin);
	// pwm_config config = pwm_get_default_config();
	// pwm_config_set_clkdiv(&config, 64.f);
	// pwm_config_set_wrap(&config, 39062.f);
	// pwm_init(;
	FTRACE("pin: %u wrap:%u level:%u slice_num %u channel: %u\n", pin, wrap, level, slice_num, chan_num);
    // Set period of 4 cycles (0 to 3 inclusive)
    pwm_set_wrap(slice_num, wrap);
    // Set channel A output high for one cycle before dropping
    pwm_set_chan_level(slice_num, chan_num, level);
    // Set initial B output high for three cycles before dropping
    // pwm_set_chan_level(slice_num, PWM_CHAN_B, 3);
    // Set the PWM running
    pwm_set_enabled(slice_num, true);
    /// \end::setup_pwm[]
}
void direction(uint pin, bool d)
{
	gpio_init(pin);
    gpio_set_dir(pin, GPIO_OUT);
	gpio_put(pin, d);
}
#endif
void test01(DRI0002V1_4& dri, uint percent)
{
	uint level = (uint)65534 * percent / 100;
	FTRACE("test01 DRTI0002 percent: %u level %u\n", percent, level);
	dri.set_pwm_percent(MotorSide::left, (double)percent);
	dri.set_pwm_percent(MotorSide::right, (double)percent);
	// dri.set_pwm_percent(1, (double)(100 - percent));
	sleep_ms(3000);
}
void test01(PwmPiPico& pwm, uint percent)
{
	uint level = (uint)65534 * percent / 100;
	FTRACE("test01 RobotPwm percent: %u level %u\n", percent, level);
	pwm.set_level(level);
	// dri.set_pwm_percent(1, (double)(100 - percent));
	sleep_ms(3000);
}
void test_updown(DRI0002V1_4& dri0002)
{
	for(int x = 0; x < 3; x++) {
		for(uint p = 100; p >= 10; p=p-20) {
			test01(dri0002, p);
		}
		dri0002.set_pwm_percent(MotorSide::left, 0.0);
		dri0002.set_direction_pin_state(MotorSide::left, reverse_motor_direction(dri0002.get_direction_pin_state(MotorSide::left)));
		dri0002.set_pwm_percent(MotorSide::right, 0.0);
		dri0002.set_direction_pin_state(MotorSide::right, reverse_motor_direction(dri0002.get_direction_pin_state(MotorSide::right)));
	}
}

int main()
{
	DRI0002V1_4 dri0002;

    stdio_init_all();
    while(! tud_cdc_connected()) {
        sleep_ms(100);
    }
    /// \tag::setup_pwm[]
	const int pin_e1 = 15;
	const int pin_m1 = 14;
	const int pin_e2 = 3;
	const int pin_m2 = 2;
	// sleep_ms(5000);
	FTRACE("Start pwm 14 \n");
	// sleep_ms(5000);
	dri0002.begin(
		MOTOR_RIGHT_DRI0002_SIDE, MOTOR_RIGHT_PWM_PIN, MOTOR_RIGHT_DIRECTION_SELECT_PIN, 
		MOTOR_LEFT_DRI0002_SIDE, MOTOR_LEFT_PWM_PIN, MOTOR_LEFT_DIRECTION_SELECT_PIN);
	sleep_ms(3000);
	FTRACE("At this point the motor should not be running\n");
	sleep_ms(3000);
	test_updown(dri0002);
}
