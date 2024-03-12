#ifndef H_encoder_h
#define H_encoder_h
#include "config.h"
#include "encoder_sample.h"

const char* pin_state(uint8_t apin_state, uint8_t bpin_state);
void irq_handler_left_apin();
void irq_handler_right_apin();
void irq_handler_left_bpin();
void irq_handler_right_bpin();


class Encoder;
void encoder_common_isr(Encoder* encoder_ptr);

Encoder* make_encoder_left();
Encoder* make_encoder_right();

class Encoder
{
    public:

    Encoder();

    Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin, void(*local_isr_a_pin)(), void(*local_isr_b_pin)());

    void begin(int id, const char* name, int encoder_a_pin, int encoder_b_pin);
    void start_interrupts() const;
#if 0
    void run();
    [[nodiscard]] bool available() const;
    void consume(EncoderSample& sample);
#endif
//    // protected:
//    void initialize(void(*isr_a)(), void(*isr_b)());
//
//    friend Encoder* make_left_encoder();
//    friend Encoder* make_right_encoder();
//    friend void common_interrupt_handler(Encoder* encoder_ptr);

    int m_id;
    const char * m_name;
    EncoderSample m_sample;
    /**
     * Following are used to save values collected by the run() function.
    */
    long   m_interrupt_count;
    double m_latest_speed;

    int m_state;
    int m_encoder_a_pin;
    int m_encoder_b_pin;
    /**
     * A pointer to the isr that handles pin a
    */
    void(*m_isr_a)();
    /**
     * A pointer to the isr that handles pin b
    */
    void(*m_isr_b)();

    /**
     * properties used by the isr
    */
    volatile bool    m_isr_new_sample_sum_available_flag;
    volatile long    m_isr_last_report;
    volatile long    m_isr_report_interval;
    volatile long    m_isr_count;
    volatile long    m_isr_interval_sum;
    volatile long    m_isr_saved_sample_sum;
    volatile long    m_isr_current_sample_sum;

    volatile int     m_isr_interval_count;
    volatile long    m_isr_previous_millis;

    volatile uint8_t            m_apin_state;
    volatile uint8_t            m_bpin_state;
    volatile uint32_t           m_isr_interrupt_count;
    volatile uint64_t           m_isr_sample_starttime_usecs;
    volatile uint64_t           m_isr_sample_most_recent_time_usecs;
    volatile uint64_t           m_isr_timestamp_musecs;
};

#define ENCODER_ISR_DEF(isr_name, encoder_ptr) \
void _time_critical(isr_name)() { \
    encoder_ptr->m_isr_interrupt_count = ep->m_isr_interrupt_count + 1; \
    encoder_ptr->m_isr_sample_most_recent_time_us = micros(); \
} 

/**
 * WARNING - This function turns off interrupts
*/
void unsafe_collect_two_encoder_samples(
    Encoder& left_encoder, EncoderSample& left_sample,
    Encoder& right_encoder, EncoderSample& right_sample
    );

#endif