#ifndef H_encoder_sample_h
#define H_encoder_sample_h
#include "config.h"
#include <cstdint>
#include "transport/buffers.h"

struct EncoderSample {
    bool        s_contains_data;
    long        s_timestamp_musecs{};
    long        s_sample_sum{};
    const char* s_pin_state{};
    bool        s_apin_state{};
    bool        s_bpin_state{};
    bool        s_available{};

    // the remaining fields are set in the Encoder.run function not in the common interrupt handler
    // and hence are derived from the raw sample values above
    const char* s_name{};
    void*       s_encoder_addr{};
    double      s_musecs_per_interrupt;
    double      s_musecs_per_motor_revolution;
    double      s_motor_rpm;
    double      s_wheel_rpm;
    double      s_speed_mm_per_second;
    EncoderSample() 
    {
        s_contains_data = false;
        s_motor_rpm = 0.0;
        s_wheel_rpm = 0.0;
        s_speed_mm_per_second = 0.0;
        s_musecs_per_interrupt = 0.0;
        s_musecs_per_motor_revolution = 0.0;
    }
    void reset()
    {
        s_contains_data = false;
        s_motor_rpm = 0.0;
        s_wheel_rpm = 0.0;
        s_speed_mm_per_second = 0.0;
        s_musecs_per_interrupt = 0.0;
        s_musecs_per_motor_revolution = 0.0;
    }
    void dump()
    {
        print_fmt("EncoderSampel addr: %p\n", (void*)this);

        print_fmt("   s_contains_data: %d\n", (int)s_contains_data);
        print_fmt("   s_sample_sum   : %ld\n", s_sample_sum);
        print_fmt("   s_motor_rpm: %f\n", s_motor_rpm);
        print_fmt("   s_wheel_rpm: %f\n", s_wheel_rpm);
    }
};
void tojson_one_encoder_sample(transport::buffer::Handle buffer_h, EncoderSample* sample);
void tojson_two_encoder_samples(transport::buffer::Handle buffer_h, EncoderSample* left_sample, EncoderSample* right_sample);


#endif