#ifndef H_robot_h
#define H_robot_h
#include "transport/buffers.h"

void robot_init();
void robot_tasks_run();
void robot_collect_encoder_samples_task_run();

// void robot_reporter_task_run();
void robot_set_raw_pwm_percent(double left_pwm_percent, double right_pwm_percent);
void robot_set_pwm_percent(double left_pwm_percent, double right_pwm_percent);
void robot_set_rpm(double left_rpm, double right_rpm);
void robot_stop_all();
void robot_request(int n);
void robot_update_pid(double kp, double ki, double kd);
void robot_collect_encoder_samples();

void tojson_encoder_samples(transport::buffer::Handle buffer_h);
#endif