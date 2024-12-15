#ifndef H_reporter_h
#define H_reporter_h

#include "encoder_v2.h"
#include "motion.h"
#include <task.h>
void report_sample(EncoderSample& sample);

class Reporter: public Taskable
{
    public:
    Reporter();
    Reporter(MotionControl& mc);
    Reporter(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr);
    Reporter(Encoder& encoder_left, Encoder& encoder_right);
    void begin(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr);
    void request(uint interval_ms, uint number);
    void request(uint interval_ms);
    void run() override;

    private:
    Encoder* m_encoder_left_ptr;
    Encoder* m_encoder_right_ptr;
    uint m_number_required;
    uint m_count;
    uint m_interval_ms;
    uint64_t m_last_report_time_since_boot_ms;
};

#endif