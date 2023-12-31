#ifndef H_remote_pid_h
#define H_remote_pid_h
#include <Arduino.h>
#include "log.h"
#include "task.h"
#include "motor.h"
#include "encoder.h"
#include "utils.h"


class RemotePid: public Taskable
{
    public:
    RemotePid(Motor& motor_left, Motor& motor_right)
    : m_motor_left(motor_left), m_motor_right(motor_right)
    {
    }
    void begin(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr)
    {
        ASSERT_MSG((encoder_left_ptr->m_id == MOTOR_LEFT_ID), "Motor ID does not match left encoder")
        ASSERT_MSG((encoder_right_ptr->m_id == MOTOR_RIGHT_ID), "Motor ID does not match left encoder")
        m_encoder_left_ptr = encoder_left_ptr;
        m_encoder_right_ptr = encoder_right_ptr;
    }
    void set_pwm(uint8_t pwm_motor_left, uint8_t pwm_motor_right)
    {
        ASSERT_MSG((m_motor_left.m_id == MOTOR_LEFT_ID), "Motor ID does not match left encoder")
        ASSERT_MSG((strcmp(m_motor_left.m_name, MOTOR_LEFT_NAME) == 0), "Motor name does not match left encoder")
        m_motor_left.set_pwmvalue(pwm_motor_left);

        ASSERT_MSG((m_motor_right.m_id == MOTOR_RIGHT_ID), "Motor ID does not match left encoder")
        ASSERT_MSG((strcmp(m_motor_right.m_name, MOTOR_RIGHT_NAME) == 0), "Motor name does not match left encoder")
        m_motor_right.set_pwmvalue(pwm_motor_right);
        log_print("remote_pid set_pwm pwm_motor_left: ", pwm_motor_left, " pwm_motor_right: ", pwm_motor_right, "\n");
    }
    void pid_set_rpm(int left_rpm, int right_rpm)
    {
    }
    void stop_all(){}
    void ahead(uint8_t pwm){}
    void run()
    {
        // log_print("RemotePid.run\n");
        m_encoder_left_ptr->run();
        m_encoder_right_ptr->run();
        if(m_encoder_left_ptr->available()) {
            // log_print("Reporter.run - got a sample\n");
            EncoderSample sample;
            m_encoder_left_ptr->consume(sample);
            char* m_name = (char*)m_encoder_left_ptr->m_motor_p->m_name;
            float current_rpm = sample.s_motor_rpm;
            int current_pwm = m_motor_left.last_pwm_value;
            log_print(
                "Pid Start\n",
                    "{'left':{",
                        "'m_name':"    ,"'",m_name, "'", ", ",
                        "'ec_name':"   ,"'",sample.s_name,"'",", ",
                        "'pwm':"       ,current_pwm,", ",
                        "'motor_rpm': ",current_rpm, 
                    "}}","\n",
                "Pid End\n");
        }
        if(m_encoder_right_ptr->available()) {
            EncoderSample sample;
            m_encoder_right_ptr->consume(sample);
            char* m_name = (char*)m_encoder_right_ptr->m_motor_p->m_name;
            float current_rpm = sample.s_motor_rpm;
            int current_pwm = m_motor_right.last_pwm_value;
            log_print(
                "Pid Start\n",
                "{'right':{",
                    "'m_name':"    ,"'",m_name, "'", ", ",
                    "'ec_name':",  "'", sample.s_name,"'", ", ",
                    "'pwm':",           current_pwm,", ",
                    "'motor_rpm': ",    current_rpm,
                "}}\n",
            "Pid End\n");
        }
       
    }

    int m_left_rpm_target;
    int m_right_rpm_target;
    Motor& m_motor_left;
    Motor& m_motor_right;
    Encoder* m_encoder_left_ptr;
    Encoder* m_encoder_right_ptr;

};

#endif