#ifndef H_motion_publisher_h
#define H_motion_publisher_h
#include "motion.h"
class MotionPublisher
{
    public:
    MotionPublisher(MotionControl& motion_control);
    void begin();
    void run();
    void publish(MotionControl& mc, Encoder& encoder_left, Encoder& encoder_right);
    MotionControl & m_motion_control;
};

#endif