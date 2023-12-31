#ifndef H_pid_h
#define H_pid_h
#include "utils.h"
class Pid
{
    public:
    Pid();
    void begin(float kp, float ki, float kd, float target, float delta_t);
    void change_target(float new_target);
    double get_target();
    void change_constants(double kp, double ki, double kd);
    double update(double latest_output);
    void dump();
    private:    
    float m_target;
    public:
    float m_kp;
    float m_ki;
    float m_kd;
    float m_delta_t;
    float m_error_previous;
    float m_integral_previous;
    long  m_count;
    double m_bounded_new_input;
    bool m_first_time;
};

#endif