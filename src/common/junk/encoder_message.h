#ifndef H_encoder_message_h
#define H_encoder_message_h
#include "encoder.h"
#include "utils.h"

class EncoderMessage 
{   public:
    char m_buffer[256];
    int  m_buf_len;
    int  m_next;

    EncoderMessage(){
        m_buf_len = 0;
        m_next = 0;
    }
    void format_samples(EncoderSample& left_sample, EncoderSample& right_sample)
    {
        m_buf_len = sprintf(m_buffer, 
        "/*AA*/\n" 
            "{"
            "'left': {\n"
                "'timestamp':%ld,\n"
                "'pin_state':'%s',\n"
                // "'apin':'%d',\n"
                // "'bpin':'%d',\n"
                "'motor_rpm': %f,\n"
                // "'wheel_rpm':%f,\n"
                // "'speed_mm_sec':%f\n"
                "}\n"
            "'right': {\n"
                "'timestamp':%ld,\n"
                "'pin_state':'%s',\n"
                // "'apin':'%d',\n"
                // "'bpin':'%d',\n"
                "'motor_rpm': %f,\n"
                // "'wheel_rpm':%f,\n"
                // "'speed_mm_sec':%f\n"
                "}\n"
            "}\n"
        "/*BB*/\n",
            // left_sample.s_name,
            left_sample.s_timestamp_musecs,
            left_sample.s_pin_state, 
            // (int)left_sample.s_apin_state,
            // (int)left_sample.s_bpin_state,
            left_sample.s_motor_rpm,
            // left_sample.s_wheel_rpm,
            // left_sample.s_speed_mm_per_second,

            right_sample.s_timestamp_musecs,
            right_sample.s_pin_state, 
            // (int)right_sample.s_apin_state,
            // (int)right_sample.s_bpin_state,
            right_sample.s_motor_rpm
            // right_sample.s_wheel_rpm,
            // right_sample.s_speed_mm_per_second 
	    );
        ASSERT_MSG(m_buf_len < 256, "EncoderMessage.format_samples buffer too long");
    }
};

#endif