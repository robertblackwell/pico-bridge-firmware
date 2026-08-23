#undef FTRACE_ON
#undef FDEBUG_ON
#include <cstdio>
#include <cstdint>
#include <cinttypes>
#include "encoder_debug.h"
void dump_encoder_sample(const char* tag,Encoder& encoder, EncoderSample& sample)
{
    printf("Dump of tag: %s encoder: %p sample %p\n", tag, &encoder, &sample);
    printf("sample_interval: %lu \n", sample.s_elapsed_usecs);
    printf("sample_tick_count: %f \n", static_cast<double>(sample.s_sample_tick_count));
    printf("motor_rpm: %f\nwheel_rpm: %f\n", sample.s_motor_rpm, sample.s_wheel_rpm);
    printf("wheel_speed_mm_per_sec: %f\n", sample.s_speed_mm_per_second);
    printf("End Dump of tag: %s encoder: %p sample %p\n", tag, &encoder, &sample);
}
void trace_encoder(const char* tag, const Encoder& encoder)
{
    printf("Encoder %s Encoder\n", tag);
    printf("\tsample_tick_count: %u sample_interval: %lu\n",
        (encoder.m_a_pin_saved_count - encoder.m_a_pin_isr_previous_count) + (encoder.m_b_pin_saved_count - encoder.m_b_pin_isr_previous_count),
        encoder.m_current_sample_time_usecs - encoder.m_previous_sample_time_usecs
        );
    const auto x = encoder.m_a_pin_saved_count + encoder.m_b_pin_saved_count;
    printf("\ta_pin_saved_count: %" PRIu32 " b_pin_saved_count: %" PRIu32 " SUM: %" PRIu32 "\n",
        encoder.m_a_pin_saved_count, encoder.m_b_pin_saved_count, x);
    printf("\ta_prev_count: %" PRIu32 " \tb_prev_count: %" PRIu32 " sum: %" PRIu32 "\n",
        encoder.m_a_pin_isr_previous_count, encoder.m_b_pin_isr_previous_count, encoder.m_a_pin_isr_previous_count + encoder.m_b_pin_isr_previous_count);
    printf("\tm_previous_sample_time_usecs: %" PRId64 " \n", encoder.m_previous_sample_time_usecs);
    printf("\tm_current_sample_time_usecs: %" PRId64 "\n", encoder.m_current_sample_time_usecs);
    printf("\telapsed: %" PRId64 " us\n", encoder.m_current_sample_time_usecs - encoder.m_previous_sample_time_usecs);
}

void trace_sample(const char* tag, const EncoderSample& sample)
{
    printf("%s EncoderSample\n", tag);
    printf("\ts_elapsed_usecs: %lu \n", sample.s_elapsed_usecs);
    printf("\ts_sample_tick_count: %u \n", sample.s_sample_tick_count);
    printf("\ts_direction %s\n", to_string(sample.s_direction));
    printf("\ts_motor_rpm %f\n", sample.s_motor_rpm);
    printf("\ts_wheel_rpm %f\n", sample.s_wheel_rpm);
    printf("\ts_speed_mm_per_second %f\n", sample.s_speed_mm_per_second);
    printf("\ts_wheel_travel_mm %f\n", sample.s_wheel_travel_mm);
}
