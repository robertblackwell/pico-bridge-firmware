#ifndef H_encoder_h
#define H_encoder_h
#include "config.h"

const char* pin_state(uint8_t apin_state, uint8_t bpin_state);
void irq_handler_left_apin();
void irq_handler_right_apin();
void irq_handler_left_bpin();
void irq_handler_right_bpin();


class Encoder;
void encoder_common_isr(Encoder* encoder_ptr);

Encoder* make_encoder_left();
Encoder* make_encoder_right();

struct EncoderSample {
    bool        s_contains_data;
    long        s_timestamp_musecs;
    long        s_sample_sum;
    const char* s_pin_state;
    bool        s_apin_state;
    bool        s_bpin_state;
    bool        s_available;

    // the remaining fields are set in the Encoder.run function not in the common interrupt handler
    // and hence are derived from the raw sample values above
    const char* s_name;
    void*       s_encoder_addr;
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
};

class Encoder
{
    public:

    Encoder();

    Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin, void(*local_isr_a_pin)(), void(*local_isr_b_pin)());

    void begin(int id, const char* name, int encoder_a_pin, int encoder_b_pin);
    void start_interrupts();

    void run();
    bool available();
    void consume(EncoderSample& sample);
    /**
     * Reset the isr state variables so that the current sample collection is aborted and 
     * collection of a new sample will start on the next interrupt.
     * This is to prevent a sample spanning over a speed change.
     * 
     * When called interrupts must be OFF
    */
    void reset_sample();
    // protected:
    void initialize(void(*isr_a)(), void(*isr_b)());

    friend Encoder* make_left_encoder();
    friend Encoder* make_right_encoder();
    friend void common_interrupt_handler(Encoder* encoder_ptr);

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

    volatile long   m_isr_timestamp_musecs;
    volatile uint8_t m_apin_state;
    volatile uint8_t m_bpin_state;
    volatile bool m_isr_new_sample_sum_available_flag;
    volatile long m_isr_last_report;
    volatile long m_isr_report_interval;
    volatile long m_isr_previous_millis;
    volatile long m_isr_count;
    volatile int  m_isr_interval_count;
    volatile long m_isr_interval_sum;
    volatile long m_isr_saved_sample_sum;
    volatile long m_isr_current_sample_sum;
};

#endif