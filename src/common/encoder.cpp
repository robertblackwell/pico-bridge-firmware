#undef FTRACE_ON
#undef FDEBUG_ON
#include <stdio.h>
#include <cstdint>
#include <pico/stdlib.h>

#include <pico/stdio.h>
#include <hardware/sync.h>
#include <pico_gpio_irq_dispatcher.h>
#include "config.h"
#include "trace.h"
#include "encoder.h"
#include "reporter.h"

#define ISR_INTR_PER_MOTOR_REVOLUTION 48 //24
//sample size should be a number of motor revolutions
// There is a tradeoff - 
//   longer intervals give more stability to the sample rpm and speed values, but 
//   longer intervals also mean less ability to react fast and average out acceleration
//
#define ISR_SAMPLE_SIZE (ISR_INTR_PER_MOTOR_REVOLUTION * 6)
#define ISR_PI_VALUE 3.14159265
#define ISR_SECS_IN_MINUTE 60.0
#define ISR_GEAR_RATIO 226.76
#define ISR_WHEEL_DIAMETER_MM 70.0

#define STATE_ERROR 5
#define STATE_INITIAL 4
#define STATE_A1B1  3
#define STATE_A1B0  2
#define STATE_A0B1  1
#define STATE_A0B0  0



#if 0
void irq_handler_left_apin();
void irq_handler_right_apin();
void irq_handler_left_bpin();
void irq_handler_right_bpin();
#endif

const char* pin_state(uint8_t apin_state, uint8_t bpin_state)
{
    const char* forward = "F";
    const char* backwards = "B";
    const char* r;
    if(apin_state == 1) {
        if(bpin_state == 0) {
            r = forward;
        } else {    
            r = backwards;
        }
    } else {
        if (bpin_state == 1) {
            r = forward;
        } else {
            r = backwards;
        }
    } 
    return r;
}

Encoder::Encoder(){}
Encoder::Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin, void(*local_isr_a_pin)(), void(*local_isr_b_pin)())
{
    FTRACE("Encode constructor addr: %p, id: %d name: %s, apin: %d bpin: %d a_pin_isr: %p, b_pin_isr: %p\n",
        this, id, name, encoder_a_pin, encoder_b_pin, local_isr_a_pin, local_isr_b_pin)
    m_isr_a = local_isr_a_pin;
    m_isr_b = local_isr_b_pin;
    m_id = id;
    m_name = name;
    m_encoder_a_pin = encoder_a_pin;
    m_encoder_b_pin = encoder_b_pin;
    m_isr_report_interval = 300;
    m_isr_last_report = 0;
    m_isr_new_sample_sum_available_flag = false;
    m_sample.s_available = false;
}

void Encoder::begin(int id, const char* name, int encoder_apin, int encoder_bpin)
{
    FTRACE("Encode constructor addr: %p, id: %d name: %s, apin: %d bpin: %d\n",
        this, id, name, encoder_apin, encoder_bpin)
    m_id = id;
    m_name = name;
    m_encoder_a_pin = encoder_apin;
    m_encoder_b_pin = encoder_bpin;
    m_isr_report_interval = 300;
    m_isr_last_report = 0;
    m_isr_new_sample_sum_available_flag = false;
    m_sample.s_available = false;
    attachGpioInterrupt(m_encoder_a_pin, m_isr_a);
    attachGpioInterrupt(m_encoder_b_pin, m_isr_b);
}
void Encoder::start_interrupts() const
{
    attachGpioInterrupt(m_encoder_a_pin, m_isr_a);
    attachGpioInterrupt(m_encoder_b_pin, m_isr_b);
}
#if 0
bool Encoder::available() const
{
    return m_sample.s_available;
}
void Encoder::consume(EncoderSample& sample)
{
    // log_print("Encoder::consume ", m_name, "\n");
    sample = m_sample;
    m_sample.s_available = false;
}
/**
 * void run() - called by the arduino loop function periodically to collect
 * lastest current speed and total and incremental distance travelled values.
 * These values can be accessed via
 * float get_speed() - returns output shaft speed in revolutions / sec
 * float get_distance() - total number of output shaft revolutions 
*/
void Encoder::run()
{
    // noInterrupts();
    gpio_irq_off(m_encoder_a_pin);
    gpio_irq_off(m_encoder_b_pin);

    m_sample.s_timestamp_musecs = m_isr_timestamp_musecs;
    m_sample.s_apin_state = m_apin_state;
    m_sample.s_bpin_state = m_bpin_state;
    m_sample.s_sample_sum =  m_isr_saved_sample_sum;
    m_sample.s_available = m_isr_new_sample_sum_available_flag;
    m_isr_new_sample_sum_available_flag = false;
    auto report = m_isr_last_report;
    gpio_irq_backon(m_encoder_a_pin);
    gpio_irq_backon(m_encoder_b_pin);

    // printf("after sample collection \n");
    // printf("  report          : %ld\n", report);
    // printf("  sample_sum      : %ld\n", m_sample.s_sample_sum);
    // printf("  sample available: %d\n", (int)m_sample.s_available);
    // interrupts();

    if(! m_sample.s_available) {
        m_sample.reset();
        // print_fmt("encoder.run name: ", m_name, " addr:", (long)this, "did not find a new sample - motor speed is probably : 0 mm/sec  \n");
        return;
    }
    m_sample.s_contains_data = true;
    m_sample.s_name = m_name;
    m_sample.s_pin_state = pin_state(m_sample.s_apin_state, m_sample.s_bpin_state); 
    m_sample.s_musecs_per_interrupt  =	((float)m_sample.s_sample_sum)/((long)ISR_SAMPLE_SIZE);
    m_sample.s_musecs_per_motor_revolution =	((float)m_sample.s_sample_sum * (float)ISR_INTR_PER_MOTOR_REVOLUTION)/((long)ISR_SAMPLE_SIZE);
    m_sample.s_motor_rpm = (ISR_SECS_IN_MINUTE * 1000000.0/ m_sample.s_musecs_per_motor_revolution);
    m_sample.s_wheel_rpm = m_sample.s_motor_rpm / ((float)ISR_GEAR_RATIO);
    m_sample.s_speed_mm_per_second = (m_sample.s_wheel_rpm * ISR_SECS_IN_MINUTE * (ISR_PI_VALUE)) / ISR_WHEEL_DIAMETER_MM;
}
#endif
void update_sample_from_isr(EncoderSample& sample);

/**
 * WARNING - This function turns off interrupts
 * Collect tick count and interval in microsecs since last collection
 * from both encoders. Reset the tick count and start time after collection.
 *
 * Important not to miss a tick as that will corrupt the odometry calculations
*/
void unsafe_collect_two_encoder_samples(
    Encoder& left_encoder, EncoderSample& left_sample,
    Encoder& right_encoder, EncoderSample& right_sample
    ) 
{
    uint32_t interrupt_status = save_and_disable_interrupts();
    uint64_t m = to_us_since_boot(get_absolute_time()); 
    {
        left_sample.s_elapsed_usecs = m - left_encoder.m_isr_sample_starttime_usecs;
        left_encoder.m_isr_sample_starttime_usecs = m; 

        left_sample.s_saved_interrupt_count = (long)left_encoder.m_isr_interrupt_count;
        left_encoder.m_isr_interrupt_count = 0;

        // left_sample.s_apin_state = left_encoder.m_apin_state;
        // left_sample.s_bpin_state = left_encoder.m_bpin_state;
    }
    {
        right_sample.s_elapsed_usecs = (m - right_encoder.m_isr_sample_starttime_usecs);
        right_sample.s_saved_interrupt_count = (long)right_encoder.m_isr_interrupt_count;
        right_encoder.m_isr_interrupt_count = 0;
        right_encoder.m_isr_sample_starttime_usecs = m; 
        right_sample.s_apin_state = right_encoder.m_apin_state;
        right_sample.s_bpin_state = right_encoder.m_bpin_state;
    }
    restore_interrupts(interrupt_status);

    update_sample_from_isr(left_sample);
    update_sample_from_isr(right_sample);
}

/**
 * Take the data collected for a single encoder in 'unsafe_collect_two_encoder_samples`
 * and perform the calcs necessary to get motor rpm, wheel rpm and wheel speed.
 * @param sample
 */
void update_sample_from_isr(EncoderSample& sample)
{
    sample.s_contains_data = true;
    if(sample.s_saved_interrupt_count == 0) {
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0;
        sample.s_musecs_per_interrupt  =	0.0;
        sample.s_musecs_per_motor_revolution =	0.0;
    } else {
        sample.s_pin_state = pin_state(sample.s_apin_state, sample.s_bpin_state); 
        sample.s_musecs_per_interrupt  =	((float)sample.s_elapsed_usecs)/((float)sample.s_saved_interrupt_count);
        sample.s_musecs_per_motor_revolution =	((float)sample.s_elapsed_usecs * (float)ISR_INTR_PER_MOTOR_REVOLUTION)/((float)sample.s_saved_interrupt_count);
        sample.s_motor_rpm = (ISR_SECS_IN_MINUTE * 1000000.0/ sample.s_musecs_per_motor_revolution);
        sample.s_wheel_rpm = sample.s_motor_rpm / ((float)ISR_GEAR_RATIO);
        sample.s_wheel_rps = sample.s_wheel_rpm / (ISR_SECS_IN_MINUTE);
        sample.s_speed_mm_per_second = sample.s_wheel_rps * (ISR_PI_VALUE) * ISR_WHEEL_DIAMETER_MM;
    }
}

void encoder_common_isr(Encoder* encoder_ptr)
{
    // encoder_ptr->m_apin_state = digital_read(encoder_ptr->m_encoder_a_pin);
    // encoder_ptr->m_bpin_state = digital_read(encoder_ptr->m_encoder_b_pin);
    encoder_ptr->m_isr_interrupt_count++;
}
