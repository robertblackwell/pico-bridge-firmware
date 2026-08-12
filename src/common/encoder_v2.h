#ifndef H_encoder_h
#define H_encoder_h
#include "config.h"
#include "encoder_sample.h"

const char* pin_state(uint8_t apin_state, uint8_t bpin_state);

struct Encoder;
void encoder_common_isr(Encoder* encoder_ptr);

struct Encoder
{
    public:
    
    // static void encoders_start();
    static Encoder* get_encoder_left();
    static Encoder* get_encoder_right();

    /**
    * WARNING - This function turns off interrupts
    */
    static void unsafe_collect_two_encoder_samples(
        Encoder& left_encoder, EncoderSample& left_sample,
        Encoder& right_encoder, EncoderSample& right_sample
        );

    Encoder();

    // Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin, void (*apin_isr)(uint, uint32_t), void(*bpin_isr)(uint, uint32_t));
    Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin);
    void start_handling_interrupts() const;

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
     * properties for revised enoder handling Aug 2026
     * Goal is to save enough information from to compute:
     * -    change since last sample - time interval and change in isr counters
     * -    validate all calculations of position and speed
     */
    volatile uint32_t m_a_pin_isr_count;        // This counter is incremented by the A  pin isr
    
    uint64_t m_previous_sample_time_usecs;      // Absolute time at which the previous sample was taken
    uint32_t m_a_pin_isr_previous_count;        // isr counter value at previous sample

    uint64_t m_sample_interval_usecs;           // 
    uint32_t m_a_pin_saved_count;               // saved value of latest a pin isr counter
    // uint32_t m_a_pin_latest_diff;
    // uint64_t m_a_pin_total;
    // void(*m_a_pin_isr)(uint, uint32_t);

    volatile uint32_t m_b_pin_isr_count;        // b pin isr counter
    uint32_t m_b_pin_isr_previous_count;        // isr counter value at previous sample
    uint32_t m_b_pin_saved_count;               // saved value of latest b pin isr counter

    // uint32_t m_b_pin_latest_diff;
    // uint64_t m_b_pin_total;
    // void(*m_b_pin_isr)(uint, uint32_t);

};



#endif