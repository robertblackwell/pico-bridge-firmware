// #include "Arduino.h"
#define FTRACE_ON
#include <hardware/gpio.h>
#include "utils.h"
#include "motor.h"

#define DEFAULT_DIRECTION LOW

#define ENABLE_DIRECTION 1
#include "config.h"
#include "utils.h"

const char* direction_string(Motor::DirectionEnum d) {
    return ((d == Motor::DirectionEnum::Forward) ? "Forward": "Reverse");
}

Motor::Motor(){}
void Motor::begin(int id, const char* name, int an_direction_select_pin, int a_pwm_pin) {
    m_id = id;
    m_name = name;
    printf("Motor constructor name: %s m_name %s \n", name, m_name);
    direction_select_pin = an_direction_select_pin;
    pwm_pin = a_pwm_pin;
    last_pwm_value = 0;
    m_direction = DirectionEnum::Forward;
    m_pico_pwm.begin(pwm_pin, 16000);
    gpio_init(direction_select_pin);
    gpio_set_dir(direction_select_pin, GPIO_OUT);
    gpio_put(direction_select_pin, 1);
  }
void Motor::change_direction() {
    FTRACE("Motor change_direction speed is: ", this->last_pwm_value, "\n");
    m_direction = (m_direction == Motor::DirectionEnum::Forward)? Motor::DirectionEnum::Reverse: Motor::DirectionEnum::Forward; 
}
void Motor::set_direction(Motor::DirectionEnum direction) {
    FTRACE("prev_direction: %d new direction: %d\n", m_direction, direction)
    if(direction != this->m_direction) {
        printf("Motor changing direction speed is: %d\n ", this->last_pwm_value);
        m_direction = direction; 
        int pin_low_high = (m_direction == Motor::DirectionEnum::Forward)? true: false;
        gpio_put(direction_select_pin, pin_low_high);
    }
}
void Motor::set_pwmvalue(uint16_t pwmvalue) {
    printf("motor.set_pwmvalue motor name: %s pwmvalue: %f \n", m_name, pwmvalue);
    last_pwm_value = pwmvalue;
    #ifdef ENABLE_DIRECTION

        // digitalWrite(direction_select_pin, (m_direction == 1) ? HIGH : LOW);
    #else 
        digitalWrite(direction_select_pin, DEFAULT_DIRECTION);
    #endif
    FTRACE(" set_pwmvalue - direction_select_pin: %d m_direction: %d HIGH/LOW: %s\n", 
        direction_select_pin, m_direction, direction_string(m_direction));
    // analogWrite(pwm_pin, pwmvalue);
    m_pico_pwm.set_level(pwmvalue);
  }

void Motor::set_duty_cycle(double duty_cycle_percent) {
    printf("motor.set_duty_cycle motor name: %s duty cycle: %f \n", m_name, duty_cycle_percent);
    last_duty_cycle = duty_cycle_percent;
    #ifdef ENABLE_DIRECTION

        // digitalWrite(direction_select_pin, (m_direction == 1) ? HIGH : LOW);
    #else 
        digitalWrite(direction_select_pin, DEFAULT_DIRECTION);
    #endif
    FTRACE(" set_duty_cycle - direction_select_pin: %d m_direction: %s \n", direction_select_pin, direction_string(m_direction));
    m_pico_pwm.set_duty_cycle(duty_cycle_percent);
  }
