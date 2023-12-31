#include "pid.h"
#include <config.h>
#include "trace.h"
Pid::Pid()
{
    m_target = 0.0;
    m_kp = 0.0;
    m_ki = 0.0;
    m_kd = 0.0;
    m_count = 0;
    m_delta_t = 0.0; 
    m_error_previous = 0.0;
    m_integral_previous = 0;
    m_first_time = false;

}
void Pid::begin(float kp, float ki, float kd, float target, float delta_t)
{
    m_target = target;
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
    m_count = 0;
    m_delta_t = delta_t; 
    m_error_previous = 0.0;
    m_integral_previous = 0;
}
void Pid::change_target(float new_target)
{
    printf("PID change target: %f\n", new_target);
    m_target = new_target;
}
double Pid::get_target()
{
    return m_target;
}
void Pid::change_constants(double kp, double ki, double kd)
{
    printf("Pid::change_constants kp: %f ki: %f kd: %f\n", kp, ki, kd);
    m_kp = kp;
    m_ki = ki;
    m_kd = kd;
}

double Pid::update(double latest_output)
{
    if(m_target < PID_RPM_ZERO) {
        printf("Pid::update m_target: %f\n", m_target);
        return 0.0;
    }
    float error = m_target - latest_output;
    float delta_error = (error - m_error_previous);
    float derivative = delta_error;
    float integral = m_integral_previous + error;
    float new_input = m_kp * error + m_ki * integral + m_kd * derivative;
    m_error_previous = error;
    m_integral_previous = integral;
    double bounded_new_input = (new_input > PID_PWM_MAX) ? PID_PWM_MAX : (new_input < PID_PWM_MIN) ? PID_PWM_MIN : new_input;
    printf("update_input this: %p m_target: %f latest_output: %f\n", this, m_target, latest_output);
    printf("    error: %f delta_error: %f derivative: %f integral: %f \n", error, delta_error, derivative, integral); 
    printf("    m_kp %f m_ki %f m_kd %f new_input: %f bounded_new_input: %f\n", m_kp, m_ki, m_kd, new_input, bounded_new_input);

    if(bounded_new_input < 0.0 || bounded_new_input > 100.0) {
        FATAL_ERROR_PRINTF("new_input is out of range %f", bounded_new_input);
    }
    return bounded_new_input;
}
void Pid::dump()
{
    print_fmt("pid this: ", (long)this, " m_target: ", m_target, 
        " m_error_previous: ", m_error_previous, 
        " m_integral_previous: ", m_integral_previous,
    "\n");

}
