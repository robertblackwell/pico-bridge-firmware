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
#include "encoder_v2.h"
#include "reporter.h"

Encoder encoder_left{MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT};
Encoder encoder_right{MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT};
void encoders_start()
{
    encoder_left.start_interrupts();
    encoder_right.start_interrupts();
}
Encoder* encoder_left_start()
{
    encoder_left.start_interrupts();
    return &encoder_left;
}
Encoder* encoder_right_start()
{
    encoder_right.start_interrupts();
    return &encoder_right;
}


/**
 * Fixes a possible bug - the sdk version does not have the memory keyword thus allowind optimization to reorder
 */
uint32_t local_save_and_disable_interrupts(){
    uint32_t status;
    __asm volatile(".syntax unified\n" "msr PRIMASK,%0"::"r" (status) : "memory" );
    return status;
}
#ifdef isr_testing
long usec_accumulator = 0;
long long previous_usecs = 0;
int count_accumulator = 0;
int tick_count_before_reporting = 600
#endif
void encoder_isr(uint pin, uint32_t event)
{
    Encoder* ptr;
    // most of the time will only be using the A pin on each encoder
    // so put those at the top of the if-else chain
    if(pin == MOTOR_LEFT_ENCODER_A_INT) {
        ptr = &encoder_left;
    } else if(pin == MOTOR_RIGHT_ENCODER_A_INT) {
        ptr = &encoder_right;
    } else if(pin == MOTOR_LEFT_ENCODER_B_INT) {
        ptr = &encoder_left;
    } else if(pin == MOTOR_RIGHT_ENCODER_A_INT) {
        ptr = &encoder_right;
    }
    if(ptr->m_isr_first_time_called_flag) {
        ptr->m_isr_first_time_called_flag = false;
        ptr->m_isr_sample_tick_count = 0;
        ptr->m_isr_lifetime_tick_count = 0;
        ptr->m_isr_sample_start_time_usecs = to_us_since_boot(get_absolute_time());
        ptr->m_isr_first_call_time = ptr->m_isr_sample_start_time_usecs;
        return;
    }
    // c++20 requirement
    auto tmp = ptr->m_isr_sample_tick_count;
    ptr->m_isr_sample_tick_count = tmp+1;
    ptr->m_isr_sample_time_of_most_recent_tick_usecs = to_us_since_boot(get_absolute_time());

    // auto y = ptr->m_isr_timestamp_musecs;
    // ptr->m_isr_interrupt_count = ptr->m_isr_interrupt_count + 1;
    // auto x = to_us_since_boot(get_absolute_time());
    // ptr->m_isr_timestamp_musecs = x;
    // auto difference = x - y;
    #ifdef isr_testing
    count_accumulator++;
    if(count_accumulator >= tick_count_before_reporting) {
        long long now = to_us_since_boot(get_absolute_time());
        long long total_usecs = now - previous_usecs;
        auto k = count_accumulator; 
        auto average_interval = ((double)total_usecs / (double)k);
        printf("common encoder_isr pin: %d ptr: %x\n", pin, ptr);
        printf("now: %lld previous: %lld\n", now, previous_usecs);
        printf("total_usecs: %lld\n", total_usecs);
        printf("count: %d\n", k);
        printf("average_interval: %f\n", average_interval);
        count_accumulator = 0;
        previous_usecs = x;
    }
    #endif
    // printf("common encoder_isr pin: %d event: %d ptr: %x real x: %ld count: %d\n", pin, event, ptr, difference, ptr->m_isr_interrupt_count);
}
void local_attach_interrupts(int gpio_pin, uint32_t events_of_interest, void(*handler)(uint pin, uint32_t event))
{
    gpio_init(gpio_pin);
	gpio_set_dir(gpio_pin, GPIO_IN);
	gpio_set_irq_enabled_with_callback(gpio_pin, events_of_interest, true, handler);
}

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
Encoder::Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin)
{
    FTRACE("Encode constructor addr: %p, id: %d name: %s, apin: %d bpin: %d a_pin_isr: %p, b_pin_isr: %p\n",
        this, id, name, encoder_a_pin, encoder_b_pin, local_isr_a_pin, local_isr_b_pin)
    m_id = id;
    m_name = name;
    m_encoder_a_pin = encoder_a_pin;
    m_encoder_b_pin = encoder_b_pin;

    m_isr_first_time_called_flag = true;
    m_isr_lifetime_tick_count = 0;
    m_isr_sample_tick_count = 0;
    m_isr_sample_start_time_usecs = 0;
    m_isr_sample_time_of_most_recent_tick_usecs = 0;
    
    m_sample.s_available = false;
}

void Encoder::start_interrupts() const
{
    auto events_both = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    auto events_fall = GPIO_IRQ_EDGE_FALL;
    auto events_rise = GPIO_IRQ_EDGE_RISE;
    local_attach_interrupts(m_encoder_a_pin, events_both, encoder_isr);
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
    //printf("unsafe_collect_two_encoder_samples\n");
    //https://github.com/raspberrypi/pico-sdk/issues/1644
    // see this reference for bug in save_and_disable_interrupts

    // interesting discussioin on making isrs faster
    //https://forums.raspberrypi.com/viewtopic.php?t=369434
    uint32_t interrupt_status = local_save_and_disable_interrupts();

        left_sample.s_isr_starttime_us = left_encoder.m_isr_sample_start_time_usecs;
        left_sample.s_isr_endtime_us = left_encoder.m_isr_sample_time_of_most_recent_tick_usecs;
        left_sample.s_isr_saved_lifetime_tick_count = left_encoder.m_isr_lifetime_tick_count;
        left_sample.s_isr_saved_sample_tick_count = left_encoder.m_isr_sample_tick_count;
        left_encoder.m_isr_sample_tick_count = 0;
        left_encoder.m_isr_sample_start_time_usecs = left_encoder.m_isr_sample_time_of_most_recent_tick_usecs;

        right_sample.s_isr_starttime_us = right_encoder.m_isr_sample_start_time_usecs;
        right_sample.s_isr_endtime_us = right_encoder.m_isr_sample_time_of_most_recent_tick_usecs;
        right_sample.s_isr_saved_lifetime_tick_count = right_encoder.m_isr_lifetime_tick_count;
        right_sample.s_isr_saved_sample_tick_count = right_encoder.m_isr_sample_tick_count;
        right_encoder.m_isr_sample_tick_count = 0;
        right_encoder.m_isr_sample_start_time_usecs = right_encoder.m_isr_sample_time_of_most_recent_tick_usecs;

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
    //printf("encoder::update_sample_from_isr\n");
    sample.s_contains_data = true;
    if(sample.s_isr_saved_sample_tick_count == 0) {
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0;
        sample.s_musecs_per_interrupt  =	0.0;
        sample.s_musecs_per_motor_revolution =	0.0;
    } else {
        // sample.s_pin_state = pin_state(sample.s_apin_state, sample.s_bpin_state); 
        sample.s_elapsed_usecs = sample.s_isr_endtime_us - sample.s_isr_starttime_us;
        sample.s_musecs_per_interrupt  =	((float)sample.s_elapsed_usecs)/((float)sample.s_isr_saved_sample_tick_count);
        sample.s_musecs_per_motor_revolution =	((float)sample.s_elapsed_usecs / ((float)sample.s_isr_saved_sample_tick_count)) * (float)ISR_INTR_PER_MOTOR_REVOLUTION;
        sample.s_motor_rpm = (ISR_SECS_IN_MINUTE * 1000000.0/ sample.s_musecs_per_motor_revolution);
        sample.s_wheel_rpm = sample.s_motor_rpm / ((float)ISR_GEAR_RATIO);
        sample.s_wheel_rps = sample.s_wheel_rpm / (ISR_SECS_IN_MINUTE);
        sample.s_speed_mm_per_second = sample.s_wheel_rps * (ISR_PI_VALUE) * ISR_WHEEL_DIAMETER_MM;
    }
}
