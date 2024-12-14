#ifndef H_encoder_h
#define H_encoder_h
#include "config.h"
#include "encoder_sample.h"

const char* pin_state(uint8_t apin_state, uint8_t bpin_state);

class Encoder;
void encoder_common_isr(Encoder* encoder_ptr);

void encoders_start();
Encoder* encoder_left_start();
Encoder* encoder_right_start();
/**
 * WARNING - This function turns off interrupts
*/
void unsafe_collect_two_encoder_samples(
    Encoder& left_encoder, EncoderSample& left_sample,
    Encoder& right_encoder, EncoderSample& right_sample
    );

struct Encoder
{
    public:

    Encoder();

    Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin);
    void start_interrupts() const;

    int m_id;
    const char * m_name;
    EncoderSample m_sample;
    /**
     * Following are used to save values collected by the run() function.
    */
    // long   m_interrupt_count;
    // double m_latest_speed;

    int m_state;
    int m_encoder_a_pin;
    int m_encoder_b_pin;

    /**
     * properties used by the isr
    */
    volatile uint64_t   m_isr_first_call_time;
    // time since boot in usecs when this sample was initialized
    volatile uint64_t   m_isr_sample_start_time_usecs;
    // time in usecs since boot of the latest tick in this sample
    volatile uint64_t   m_isr_sample_time_of_most_recent_tick_usecs;
    // number of ticks in this sample
    volatile uint64_t   m_isr_sample_tick_count;
    // count of ticks since the frst call to the isr
    volatile uint64_t   m_isr_lifetime_tick_count;
    // this flag allows the isr to determine the first time it is called 
    // so as to do some initialization
    volatile bool       m_isr_first_time_called_flag;

    // volatile bool       m_isr_new_sample_sum_available_flag;
    // volatile long       m_isr_last_report;
    // volatile long       m_isr_report_interval;
    // volatile long       m_isr_count;
    // volatile long       m_isr_interval_sum;
    // volatile long       m_isr_saved_sample_sum;
    // volatile long       m_isr_current_sample_sum;
    // volatile int        m_isr_interval_count;
    // volatile long       m_isr_previous_millis;
    // volatile uint32_t   m_isr_interrupt_count;
    
    // volatile uint8_t    m_apin_state;
    // volatile uint8_t    m_bpin_state;
};



#endif