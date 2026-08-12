#undef FTRACE_ON
#undef FDEBUG_ON
#include <stdio.h>
#include <cstdint>
#include <cinttypes>
#include <pico/stdlib.h>
#include <print>

#include <pico/stdio.h>
#include <hardware/sync.h>
#include <pico_gpio_irq_dispatcher.h>
#include "config.h"
#include "trace.h"
#include "encoder_v2.h"
#include "reporter.h"
#define isr_testing
void dump_encoder_sample(const char* tag,Encoder& encoder, EncoderSample& sample);


// Encoder encoder_left{MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT, &encoder_left_isr_a_pin, &encoder_left_isr_b_pin};
// Encoder encoder_right{MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT, &encoder_right_isr_a_pin, &encoder_right_isr_b_pin};

Encoder encoder_left{MOTOR_LEFT_ID, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT};
Encoder encoder_right{MOTOR_RIGHT_ID, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT};

bool isr_debug = false;

static void encoder_left_isr_a_pin(uint pin, uint32_t event, Encoder* encoder)
{
    if(isr_debug) printf("encoder_left_isr_a_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_left, encoder_left.m_a_pin_isr_count);
    const auto v = encoder_left.m_a_pin_isr_count;
    encoder_left.m_a_pin_isr_count = v + 1;
}

static void encoder_left_isr_b_pin(uint pin, uint32_t event, Encoder* encoder)
{
    if(isr_debug) printf("encoder_left_isr_b_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_left, encoder_left.m_b_pin_isr_count);
    const auto v = encoder_left.m_b_pin_isr_count;
    encoder_left.m_b_pin_isr_count = v + 1;
}

static void encoder_right_isr_a_pin(uint pin, uint32_t event,  Encoder* encoder)
{
    if(isr_debug) printf("encoder_right_isr_a_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_right, encoder_left.m_a_pin_isr_count);
    const auto v = encoder_right.m_a_pin_isr_count;
    encoder_right.m_a_pin_isr_count = v + 1;
}

static void encoder_right_isr_b_pin(uint pin, uint32_t event,  Encoder* encoder) {
    if(isr_debug) printf("encoder_right_isr_b_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_right, encoder_right.m_b_pin_isr_count);
    const auto v = encoder_right.m_b_pin_isr_count;
    encoder_right.m_b_pin_isr_count = v + 1;
}

Encoder* Encoder::get_encoder_left()
{
    return &encoder_left;
}
Encoder* Encoder::get_encoder_right()
{
    return &encoder_right;
}


/**
 * Fixes a possible bug - the sdk version does not have the memory keyword thus allowind optimization to reorder
 */
static uint32_t local_save_and_disable_interrupts(){
    uint32_t status;
    __asm volatile(".syntax unified\n" "msr PRIMASK,%0"::"r" (status) : "memory" );
    return status;
}

#if 1
static void encoder_isr(uint pin, uint32_t event)
{
    switch(pin) {
        case MOTOR_LEFT_ENCODER_A_INT: {
                if(isr_debug) printf("encoder_left_isr_a_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_left, encoder_left.m_a_pin_isr_count);
                const auto v = encoder_left.m_a_pin_isr_count;
                encoder_left.m_a_pin_isr_count = v + 1;
            }
            break;
        case MOTOR_LEFT_ENCODER_B_INT: { 
                if(isr_debug) printf("encoder_left_isr_b_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_left, encoder_left.m_b_pin_isr_count);
                const auto v = encoder_left.m_b_pin_isr_count;
                encoder_left.m_b_pin_isr_count = v + 1;
            }
            break;
        case MOTOR_RIGHT_ENCODER_A_INT: {
                if(isr_debug) printf("encoder_right_isr_a_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_right, encoder_left.m_a_pin_isr_count);
                const auto v = encoder_right.m_a_pin_isr_count;
                encoder_right.m_a_pin_isr_count = v + 1;
            }
            break;
        case MOTOR_RIGHT_ENCODER_B_INT: {
                if(isr_debug) printf("encoder_right_isr_b_pin pin: %ld event: %ld encoder:%x count:%d\n", pin,  event, &encoder_right, encoder_right.m_b_pin_isr_count);
                const auto v = encoder_right.m_b_pin_isr_count;
                encoder_right.m_b_pin_isr_count = v + 1;
            }
            break;
    }
}
#endif
#if 0
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
        // count_accumulator = 0;
        // previous_usecs = x;
    }
    #endif
    // printf("common encoder_isr pin: %d event: %d ptr: %x real x: %ld count: %d\n", pin, event, ptr, difference, ptr->m_isr_interrupt_count);
}
#endif
static void local_attach_interrupts(int gpio_pin, uint32_t events_of_interest, void(*handler)(uint pin, uint32_t event))
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
// Encoder::Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin, void (*apin_isr)(uint, uint32_t, Encoder*), void(*bpin_isr)(uint, uint32_t, Encoder*))
Encoder::Encoder(int id, const char* name, int encoder_a_pin, int encoder_b_pin)
{
    FTRACE("Encode constructor addr: %p, id: %d name: %s, apin: %d bpin: %d a_pin_isr: %p, b_pin_isr: %p\n",
        this, id, name, encoder_a_pin, encoder_b_pin, local_isr_a_pin, local_isr_b_pin)
    m_id = id;
    m_name = name;
    m_encoder_a_pin = encoder_a_pin;
    m_encoder_b_pin = encoder_b_pin;
    m_a_pin_isr_previous_count = 0;

    m_b_pin_isr_previous_count = 0;
    m_sample.s_available = false;
}
static int isrflag = 0;
void Encoder::start_handling_interrupts() const
{
    // if(isrflag != 0) {
    //     return;
    // } 
    // isrflag = 1;
    constexpr auto events_both = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    auto events_fall = GPIO_IRQ_EDGE_FALL;
    auto events_rise = GPIO_IRQ_EDGE_RISE;
    printf("Encoder::start_handling_interrupts() \n");
    if(m_encoder_a_pin == MOTOR_LEFT_ENCODER_A_INT) {
        // printf("left: %x %x %x %x\n", m_a_pin_isr, m_b_pin_isr, &encoder_left_isr_a_pin, &encoder_left_isr_b_pin);

        // local_attach_interrupts(m_encoder_a_pin, events_both, &encoder_left_isr_a_pin);
        // local_attach_interrupts(m_encoder_b_pin, events_both, &encoder_left_isr_b_pin);

        // local_attach_interrupts(m_encoder_a_pin, events_both, m_a_pin_isr);
        // local_attach_interrupts(m_encoder_b_pin, events_both, m_b_pin_isr);

        local_attach_interrupts(m_encoder_a_pin, events_both, &encoder_isr);
        local_attach_interrupts(m_encoder_b_pin, events_both, &encoder_isr);
    } else {
        // printf("right: %x %x %x %x\n", m_a_pin_isr, m_b_pin_isr, &encoder_right_isr_a_pin, &encoder_right_isr_b_pin);

        // local_attach_interrupts(m_encoder_a_pin, events_both, &encoder_right_isr_a_pin);
        // local_attach_interrupts(m_encoder_b_pin, events_both, &encoder_right_isr_b_pin);

        // local_attach_interrupts(m_encoder_a_pin, events_both, m_a_pin_isr);
        // local_attach_interrupts(m_encoder_b_pin, events_both, m_b_pin_isr);

        local_attach_interrupts(m_encoder_a_pin, events_both, &encoder_isr);
        local_attach_interrupts(m_encoder_b_pin, events_both, &encoder_isr);
    }
}

void update_sample_from_isr(EncoderSample& sample);
void fill_sample(EncoderSample& sample, uint64_t sample_time, uint64_t previous_sample_time_usecs, uint32_t sample_tick_count);

/**
 * WARNING - This function turns off interrupts
 * Collect tick count and interval in microsecs since last collection
 * from both encoders. Reset the tick count and start time after collection.
 *
 * Important not to miss a tick as that will corrupt the odometry calculations
*/
void Encoder::unsafe_collect_two_encoder_samples(
    Encoder& left_encoder, EncoderSample& left_sample,
    Encoder& right_encoder, EncoderSample& right_sample
    ) 
{
    #define EC_DEBUG
    printf("Begin========================================================================================================\n");
    uint32_t raw_left_a_count; 
    uint32_t raw_left_b_count; 
    uint32_t raw_right_a_count; 
    uint32_t raw_right_b_count;
    //https://github.com/raspberrypi/pico-sdk/issues/1644
    // see this reference for bug in save_and_disable_interrupts

    // interesting discussion on making isrs faster
    //https://forums.raspberrypi.com/viewtopic.php?t=369434

    const uint64_t sample_time = to_us_since_boot(get_absolute_time());
    // const uint32_t left_a_prev_count = left_encoder.m_a_pin_isr_previous_count;
    // const uint32_t left_b_prev_count = left_encoder.m_b_pin_isr_previous_count;
    // const uint32_t right_a_prev_count = right_encoder.m_a_pin_isr_previous_count;
    // const uint32_t right_b_prev_count = right_encoder.m_b_pin_isr_previous_count;
    #ifdef EC_DEBUG
    // printf("SAVED unsafe_collect_two_encoder_samples \n\tleft_a_prev_count: %" PRIu32 " \n\tleft_b_prev_count: %" PRIu32 " \n\tright_a_prev_count: %" PRIu32 " \n\tright_b_prev_count: %" PRIu32 "\n", 
    //     left_encoder.m_a_pin_isr_previous_count, left_encoder.m_b_pin_isr_previous_count, 
    //     right_encoder.m_a_pin_isr_previous_count, right_encoder.m_b_pin_isr_previous_count);
    // printf("\n\tleft_a_prev_count: %" PRIu32 "\n\tleft_b_prev_count: %" PRIu32 "\n\tright_a_prev_count: %" PRIu32 " \n\tright_b_prev_count: %" PRIu32 "\n", 
    //     left_a_prev_count, left_b_prev_count, right_a_prev_count, right_b_prev_count);
    #endif
    
    const uint32_t interrupt_status = local_save_and_disable_interrupts();
    {
        raw_left_a_count = left_encoder.m_a_pin_isr_count; 
        raw_left_b_count = left_encoder.m_b_pin_isr_count; 
        raw_right_a_count = right_encoder.m_a_pin_isr_count; 
        raw_right_b_count = right_encoder.m_b_pin_isr_count;
    }
    restore_interrupts(interrupt_status);

    left_encoder.m_a_pin_saved_count = raw_left_a_count;
    left_encoder.m_b_pin_saved_count = raw_left_b_count;
    right_encoder.m_a_pin_saved_count = raw_right_a_count;
    right_encoder.m_b_pin_saved_count = raw_right_b_count;

    const auto left_sample_tick_count =
            (raw_left_a_count - left_encoder.m_a_pin_isr_previous_count)
        +   (raw_left_b_count - left_encoder.m_b_pin_isr_previous_count);
    fill_sample(left_sample, sample_time, left_encoder.m_previous_sample_time_usecs, left_sample_tick_count);
    left_encoder.m_previous_sample_time_usecs = sample_time;
    left_encoder.m_a_pin_isr_previous_count = raw_left_a_count;
    left_encoder.m_b_pin_isr_previous_count = raw_left_b_count;

    const auto right_sample_tick_count =
            (raw_right_a_count - right_encoder.m_a_pin_isr_previous_count)
        +   (raw_right_b_count - right_encoder.m_b_pin_isr_previous_count);
    fill_sample(right_sample, sample_time, right_encoder.m_previous_sample_time_usecs, right_sample_tick_count);
    right_encoder.m_previous_sample_time_usecs = sample_time;
    right_encoder.m_a_pin_isr_previous_count = raw_right_a_count;
    right_encoder.m_b_pin_isr_previous_count = raw_right_b_count;

    // dump_encoder_sample("left", left_encoder, left_sample);
    // dump_encoder_sample("right", right_encoder, right_sample);
#ifdef EC_DEBUGX
    std::print("left_sample_interval: {} right_sample_interval: {}\n", left_sample_interval, right_sample_interval);
    printf("left_sample_interval: %llu right_sample_interval: %llu\n", left_sample_interval, right_sample_interval);
    printf("sample_time: %llu\n", sample_time);
    printf("left_encoder.m_previous_sample_time_usecs: %llu\n", left_encoder.m_previous_sample_time_usecs);
    printf("right_encoder.m_previous_sample_time_usecs: %llu\n", right_encoder.m_previous_sample_time_usecs);
    printf("\n\tleft_total_count: %f \n\tright_total_count: %f \n\tleft_motor_revs: %f \n\tright_motor_revs: %f\n",
        static_cast<double>(left_sample.s_sample_tick_count), static_cast<double>(right_sample.s_sample_tick_count), left_motor_revs, right_motor_revs);
    printf("\n\tleft_wheel_revs: %f \n\tright_wheel_revs: %f\n", left_wheel_revs, right_wheel_revs);
    printf("\n\tleft_motor_rpm: %f\n\tright_motor_rpm: %f\n\tleft_wheel_rpm: %f\n\tright_wheel_rpm: %f\n",
        left_sample.s_motor_rpm, right_sample.s_motor_rpm, left_sample.s_wheel_rpm, right_sample.s_wheel_rpm);
    #endif


    #ifdef EC_DEBUG
    // printf("left a_pin_saved_count: %f left b_pin_saved_count %f \n", (float)left_encoder.m_a_pin_saved_count, (float)left_encoder.m_b_pin_saved_count);
    // printf("left isr_saved_sample_tick_count: %f \n", (float)left_sample.s_isr_saved_sample_tick_count);
    // printf("right a_pin_saved_count: %f right b_pin_saved_count %f \n", (float)right_encoder.m_a_pin_saved_count, (float)right_encoder.m_b_pin_saved_count);
    // printf("right isr_saved_sample_tick_count: %f \n", (float)right_sample.s_isr_saved_sample_tick_count);
    // update_sample_from_isr(left_sample);
    // update_sample_from_isr(right_sample);
    #endif
    left_encoder.m_a_pin_isr_previous_count = raw_left_a_count;
    left_encoder.m_b_pin_isr_previous_count = raw_left_b_count;
    right_encoder.m_a_pin_isr_previous_count = raw_right_a_count;
    right_encoder.m_b_pin_isr_previous_count = raw_right_b_count;
    #ifdef EC_DEBUG
    // printf("SAVED unsafe_collect_two_encoder_samples \n\tleft_a_prev_count: %" PRIu32 " \n\tleft_b_prev_count: %" PRIu32 " \n\tright_a_prev_count: %" PRIu32 " \n\tright_b_prev_count: %" PRIu32 "\n", 
    //     left_encoder.m_a_pin_isr_previous_count, left_encoder.m_b_pin_isr_previous_count, 
    //     right_encoder.m_a_pin_isr_previous_count, right_encoder.m_b_pin_isr_previous_count);
    #endif
    printf("End========================================================================================================\n");
}
void fill_sample(EncoderSample& sample, uint64_t sample_time, uint64_t previous_sample_time_usecs, uint32_t sample_tick_count)
{
    if (sample_tick_count == 0) {
        sample.s_elapsed_usecs = sample_time - previous_sample_time_usecs;
        sample.s_sample_tick_count = sample_tick_count;
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0;
    } else {
        sample.s_elapsed_usecs = sample_time - previous_sample_time_usecs;
        sample.s_sample_tick_count = sample_tick_count;
        const double left_sample_interval_secs = static_cast<double>(sample.s_elapsed_usecs)/1000000.0;
        const double left_motor_revs = sample_tick_count / static_cast<double>(ISR_INTR_PER_MOTOR_REVOLUTION);
        const double left_wheel_revs = left_motor_revs / (double)ISR_GEAR_RATIO;
        sample.s_motor_rpm = (left_motor_revs / left_sample_interval_secs)*60.0;
        sample.s_wheel_rpm = (left_wheel_revs / left_sample_interval_secs)*60.0;
        sample.s_wheel_rps = sample.s_wheel_rpm / 60.0;
        sample.s_speed_mm_per_second = sample.s_wheel_rps * (ISR_PI_VALUE) * ISR_WHEEL_DIAMETER_MM;
        sample.s_wheel_travel_mm = ISR_WHEEL_DIAMETER_MM * (ISR_PI_VALUE) * (sample.s_sample_tick_count / (double)(ISR_INTR_PER_MOTOR_REVOLUTION * ISR_GEAR_RATIO));
    }
}
void sample_from_encoder(Encoder& encoder, EncoderSample& sample)
{
#if 0
    // printf("encoder::update_sample_from_isr ticks: %ld\n", sample.s_isr_saved_sample_tick_count);
    sample.s_contains_data = true;
    if(sample.s_isr_saved_sample_tick_count == 0) {
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0;
        sample.s_musecs_per_interrupt  =	0.0;
        sample.s_musecs_per_motor_revolution =	0.0;
    } else {
        // sample.s_pin_state = pin_state(sample.s_apin_state, sample.s_bpin_state);
        // sample.s_elapsed_usecs = sample.s_isr_endtime_us - sample.s_isr_starttime_us;
        sample.s_musecs_per_interrupt  =	((double)sample.s_elapsed_usecs)/((double)sample.s_isr_saved_sample_tick_count);
        sample.s_musecs_per_motor_revolution =	((double)sample.s_elapsed_usecs / ((double)sample.s_isr_saved_sample_tick_count)) * (double)ISR_INTR_PER_MOTOR_REVOLUTION;
        sample.s_motor_rpm = (ISR_SECS_IN_MINUTE * 1000000.0/ sample.s_musecs_per_motor_revolution);
        sample.s_wheel_rpm = sample.s_motor_rpm / ((float)ISR_GEAR_RATIO);
        sample.s_wheel_rps = sample.s_wheel_rpm / (ISR_SECS_IN_MINUTE);
        sample.s_speed_mm_per_second = sample.s_wheel_rps * (ISR_PI_VALUE) * ISR_WHEEL_DIAMETER_MM;
        // printf("isr_saved_sample_tick_count: %ld\n", sample.s_isr_saved_sample_tick_count);
        // printf("elapsed_usecs: %f\n", (float)sample.s_elapsed_usecs);
        // printf("ISR_INT_PER_MOTOR_REVOLUTION: %f\n", (float)ISR_INTR_PER_MOTOR_REVOLUTION);
        // printf("usecs_per_interrupt: %f\n", sample.s_musecs_per_interrupt);
        // printf("usecs_per_motor_revolution: %f \n", sample.s_musecs_per_motor_revolution);
        // printf("ISR_SECS_IN_MINUTE %f\n", (float)ISR_SECS_IN_MINUTE);
        // printf("motor_rpm: %f \n", sample.s_motor_rpm);
        // printf("ISR_GEAR_RATIO: %f\n", (float)ISR_GEAR_RATIO);
        // printf("wheel_rpm: %f  wheel_rps: %f\n", sample.s_wheel_rpm, sample.s_wheel_rps);
        // printf("speed_mm_per_seconds: %f\n", sample.s_speed_mm_per_second);
    }
#endif
}
/**
 * Take the data collected for a single encoder in 'unsafe_collect_two_encoder_samples`
 * and perform the calcs necessary to get motor rpm, wheel rpm and wheel speed.
 * @param sample
 */
void update_sample_from_isr(EncoderSample& sample)
{
#if 0
    // printf("encoder::update_sample_from_isr ticks: %ld\n", sample.s_isr_saved_sample_tick_count);
    sample.s_contains_data = true;
    if(sample.s_isr_saved_sample_tick_count == 0) {
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0; 
        sample.s_musecs_per_interrupt  =	0.0;
        sample.s_musecs_per_motor_revolution =	0.0;
    } else {
        // sample.s_pin_state = pin_state(sample.s_apin_state, sample.s_bpin_state); 
        // sample.s_elapsed_usecs = sample.s_isr_endtime_us - sample.s_isr_starttime_us;
        sample.s_musecs_per_interrupt  =	((double)sample.s_elapsed_usecs)/((double)sample.s_isr_saved_sample_tick_count);
        sample.s_musecs_per_motor_revolution =	((double)sample.s_elapsed_usecs / ((double)sample.s_isr_saved_sample_tick_count)) * (double)ISR_INTR_PER_MOTOR_REVOLUTION;
        sample.s_motor_rpm = (ISR_SECS_IN_MINUTE * 1000000.0/ sample.s_musecs_per_motor_revolution);
        sample.s_wheel_rpm = sample.s_motor_rpm / ((float)ISR_GEAR_RATIO);
        sample.s_wheel_rps = sample.s_wheel_rpm / (ISR_SECS_IN_MINUTE);
        sample.s_speed_mm_per_second = sample.s_wheel_rps * (ISR_PI_VALUE) * ISR_WHEEL_DIAMETER_MM;
        // printf("isr_saved_sample_tick_count: %ld\n", sample.s_isr_saved_sample_tick_count);
        // printf("elapsed_usecs: %f\n", (float)sample.s_elapsed_usecs);
        // printf("ISR_INT_PER_MOTOR_REVOLUTION: %f\n", (float)ISR_INTR_PER_MOTOR_REVOLUTION);
        // printf("usecs_per_interrupt: %f\n", sample.s_musecs_per_interrupt);
        // printf("usecs_per_motor_revolution: %f \n", sample.s_musecs_per_motor_revolution);
        // printf("ISR_SECS_IN_MINUTE %f\n", (float)ISR_SECS_IN_MINUTE);
        // printf("motor_rpm: %f \n", sample.s_motor_rpm);
        // printf("ISR_GEAR_RATIO: %f\n", (float)ISR_GEAR_RATIO);
        // printf("wheel_rpm: %f  wheel_rps: %f\n", sample.s_wheel_rpm, sample.s_wheel_rps);
        // printf("speed_mm_per_seconds: %f\n", sample.s_speed_mm_per_second);
    }
#endif
}
void dump_encoder_sample(const char* tag,Encoder& encoder, EncoderSample& sample)
{
    printf("Dump of tag: %s encoder: %p sample %p\n", tag, &encoder, &sample);
    printf("sample_interval: %llu \n", sample.s_elapsed_usecs);
    printf("sample_tick_count: %f \n", static_cast<double>(sample.s_sample_tick_count));
    printf("motor_rpm: %f\nwheel_rpm: %f\n", sample.s_motor_rpm, sample.s_wheel_rpm);
    printf("wheel_speed_mm_per_sec: %f\n", sample.s_speed_mm_per_second);
    printf("End Dump of tag: %s encoder: %p sample %p\n", tag, &encoder, &sample);
}
