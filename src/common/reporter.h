#ifndef H_reporter_h
#define H_reporter_h

#include "encoder.h"
#include <task.h>
void report_sample(EncoderSample& sample);

class Reporter: public Taskable
{
    public:
    Reporter();
    Reporter(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr);
    Reporter(Encoder& encoder_left, Encoder& encoder_right);
    void begin(Encoder* encoder_left_ptr, Encoder* encoder_right_ptr);
    void request(int number);
    void run();

    private:
    Encoder* m_encoder_left_ptr;
    Encoder* m_encoder_right_ptr;
    uint m_number_required;
    uint m_count;
};

#endif