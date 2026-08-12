#ifndef H_SPEED_CONTROL_H
#define H_SPEED_CONTROL_H
#include <stdio.h>

struct SpeedControl
{
    double m_kp;
    double m_ki;
    double m_integral;
    double m_delta_time_secs;
public:
    void init(double kp, const double ki)
    {
        m_kp = kp;
        m_ki = ki;
        m_integral = 0;
        m_delta_time_secs = 1.00;
    }
    double next_pwm_estimate(const double target_velocity, const double latest_velocity)
    {
        const auto error = target_velocity - latest_velocity;
        m_integral = m_integral + (error * m_delta_time_secs);
        const auto newpwm = m_kp * error + m_ki * m_integral;
        printf("pi_wheel_speed_control_next_pwm_estimate: target %f\n\t latest: %f\n\terror: %f\n\tEi: %f\n\tnewpwm: %f \n", target_velocity, latest_velocity, error, m_integral, newpwm);
        return (newpwm > 100.0) ? 100.0 : newpwm;
    }
};

#endif