#ifndef H_encoder_sample_h
#define H_encoder_sample_h
#include "config.h"
#include <cstdint>
#include "enum.h"
#include "transport/buffers.h"

struct EncoderSample
{
    double      s_motor_rpm;
    double      s_wheel_rpm;
    double      s_wheel_rps;
    double      s_speed_mm_per_second;
    double      s_wheel_travel_mm;
    uint64_t    s_elapsed_usecs; //tick
    uint32_t    s_sample_tick_count;
    MotorDirection s_direction;
    bool        s_contains_data;

    EncoderSample()
    {
        s_contains_data = false;
        s_motor_rpm = 0.0;
        s_wheel_rpm = 0.0;
        s_speed_mm_per_second = 0.0;
        s_elapsed_usecs = 0;
        s_wheel_rps = 0.0;
        s_wheel_travel_mm = 0.0;
        #if 0
        s_musecs_per_interrupt = 0.0;
        s_musecs_per_motor_revolution = 0.0;
        s_heading_change_radians = 0.0;
        #endif
    }
    void reset()
    {
        s_contains_data = false;
        s_motor_rpm = 0.0;
        s_wheel_rpm = 0.0;
        s_speed_mm_per_second = 0.0;
        s_elapsed_usecs = 0;
        s_wheel_rps = 0.0;
        s_wheel_travel_mm = 0.0;
        #if 0
        s_musecs_per_interrupt = 0.0;
        s_musecs_per_motor_revolution = 0.0;
        s_heading_change_radians = 0.0;
        #endif
    }
    void dump()
    {
        print_fmt("EncoderSampel addr: %p", (void*)this);
        print_fmt("   s_contains_data              : %d", (int)s_contains_data);
        print_fmt("   s_elapsed_usecs              : %llu", s_elapsed_usecs);
        print_fmt("   s_motor_rpm                  : %f", s_motor_rpm);
        print_fmt("   s_direction                  : %s", to_string(s_direction));
        print_fmt("   s_wheel_rpm                  : %f", s_wheel_rpm);
        print_fmt("   s_wheel_rps                  : %f", s_wheel_rps);
        print_fmt("   s_speed_mm_per_second        : %f", s_speed_mm_per_second);
    }
};
void tojson_one_encoder_sample(transport::buffer::Handle buffer_h, EncoderSample* sample);
void tojson_two_encoder_samples(transport::buffer::Handle buffer_h, EncoderSample* left_sample, EncoderSample* right_sample);


#endif