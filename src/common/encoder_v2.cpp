#undef FTRACE_ON
#undef FDEBUG_ON
#include <cstdio>
#include <cstdint>
#include <cinttypes>
#include <pico/stdlib.h>
#include <print>
#include <hardware/sync.h>
#include <pico_gpio_irq_dispatcher.h>
#include "config.h"
#include "trace.h"
#include "encoder_v2.h"
#define isr_testing

static void dump_encoder_sample(const char* tag,Encoder& encoder, EncoderSample& sample);

static Encoder encoder_left{ MotorSide::left, MOTOR_LEFT_NAME, MOTOR_LEFT_ENCODER_A_INT, MOTOR_LEFT_ENCODER_B_INT};
static Encoder encoder_right{ MotorSide::right, MOTOR_RIGHT_NAME, MOTOR_RIGHT_ENCODER_A_INT, MOTOR_RIGHT_ENCODER_B_INT};

static bool isr_debug = false;

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

static void encoder_isr(const uint pin, const uint32_t event)
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
        default: ;
    }
}
static void local_attach_interrupts(int gpio_pin, uint32_t events_of_interest, void(*handler)(uint pin, uint32_t event))
{
    gpio_init(gpio_pin);
	gpio_set_dir(gpio_pin, GPIO_IN);
	gpio_set_irq_enabled_with_callback(gpio_pin, events_of_interest, true, handler);
}

const char* pin_state(const uint8_t apin_state, const uint8_t bpin_state)
{
    constexpr auto forward = "F";
    constexpr auto backwards = "B";
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

Encoder::Encoder(const MotorSide side, const char* name, const int encoder_a_pin, const int encoder_b_pin)
{
    FTRACE("Encode constructor addr: %p, side: %d name: %s, apin: %d bpin: %d a_pin_isr: %p, b_pin_isr: %p\n",
        this, m_side, name, encoder_a_pin, encoder_b_pin, local_isr_a_pin, local_isr_b_pin)
    m_side = side;
    m_name = name;
    m_state = 0;
    m_encoder_a_pin = encoder_a_pin;
    m_encoder_b_pin = encoder_b_pin;
    m_a_pin_isr_count = 0;
    m_b_pin_isr_count = 0;
    m_previous_sample_time_usecs = 0;
    m_a_pin_isr_previous_count = 0;
    m_b_pin_isr_previous_count = 0;
    m_sample_interval_usecs = 0;
    m_a_pin_saved_count = 0;
    m_b_pin_saved_count = 0;
    m_sample.s_contains_data = false;
    m_current_sample_time_usecs = 0;
}
void Encoder::init(uint64_t current_time_us)
{
    m_current_sample_time_usecs = current_time_us;
    m_a_pin_isr_count = 0;
    m_a_pin_saved_count = 0;
    m_b_pin_isr_count = 0;
    m_b_pin_saved_count = 0;
}
void Encoder::prepare_next(const uint64_t current_time_us)
{
    m_previous_sample_time_usecs = m_current_sample_time_usecs;
    m_a_pin_isr_previous_count = m_a_pin_saved_count;
    m_b_pin_isr_previous_count = m_b_pin_saved_count;
    m_current_sample_time_usecs = current_time_us;
}
void Encoder::start_handling_interrupts() const
{
    constexpr auto events_both = GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE;
    auto events_fall = GPIO_IRQ_EDGE_FALL;
    auto events_rise = GPIO_IRQ_EDGE_RISE;
    printf("Encoder::start_handling_interrupts() \n");
    local_attach_interrupts(m_encoder_a_pin, events_both, &encoder_isr);
    local_attach_interrupts(m_encoder_b_pin, events_both, &encoder_isr);
}
#define EC_DEBUGX
static void print_encoder(const char* tag, const Encoder& encoder);
static void print_sample(const char* tag, const EncoderSample& sample);
#ifdef EC_DEBUG
#define PRINT_ENCODER(tag, encoder) print_encoder(tag, encoder);
#define PRINT_SAMPLE(tag, sample) print_sample(tag, sample);
#define PRINT_BEGIN  printf("Begin========================================================================================================\n");
#define PRINT_END    printf("End========================================================================================================\n");

#else
#define PRINT_ENCODER(tag, encoder)
#define PRINT_SAMPLE(tag, sample)
#define PRINT_BEGIN
#define PRINT_END
#endif

void update_sample_from_isr(EncoderSample& sample);
void fill_sample(EncoderSample& sample, uint64_t sample_time, uint64_t previous_sample_time_usecs, uint32_t sample_tick_count, MotorDirection direction);

/**
 * WARNING - This function turns off interrupts
 * Collect tick count and interval in microsecs since last collection
 * from both encoders. Reset the tick count and start time after collection.
 *
 * Important not to miss a tick as that will corrupt the odometry calculations
    //https://github.com/raspberrypi/pico-sdk/issues/1644
    // see this reference for bug in save_and_disable_interrupts

    // interesting discussion on making isrs faster
    //https://forums.raspberrypi.com/viewtopic.php?t=369434

 * The properties of left_encoder and right_encoder are state variables that are inputs to the
 * kinematic calculations that happen at the end of each sample period.
 *
 * In order that those calcuations are correct there is a required initialization step and
 * a transition step at across the transition from one sample period to the next.
 *
 * This comment attempts to codify that transition.
 *
 * Initialization:
 *
    left_encoder.m_a_pin_saved_count    <-- set to 0 or the starting value for left_encoder.m_b_pin_isr_count
    left_encoder.m_b_pin_saved_count    <-- set to 0 or the starting value for left_encoder.m_b_pin_isr_count
    left_encoder.m_current_sample_time_usecs  <-- set to current time when previous 2 values are initialized

    right_encoder.m_a_pin_saved_count   <-- set to 0 or the starting  value for right_encoder.m_a_pin_isr_count
    right_encoder.m_b_pin_saved_count   <-- set to 0 or the starting value for right_encoder.m_b_pin_isr_count
    right_encoder.m_current_sample_time_usecs <-- set to current time when previous 2 values are initialized

*
* Transition
* At the end of each sample period the following transition takes place in preparation for
* kinematic calculations:
*
    left_encoder.m_a_pin_isr_previous_count    <-- left_encoder.m_a_pin_saved_count
    left_encoder.m_b_pin_isr_previous_count    <-- left_encoder.m_b_pin_saved_count
    left_encoder.m_previous_sample_time_usecs  <-- left_encoder.m_current_sample_time_usecs
    left_encoder.m_a_pin_saved_count    <-- set to current value of left_encoder.m_b_pin_isr_count
    left_encoder.m_b_pin_saved_count    <-- set to current value of left_encoder.m_b_pin_isr_count
    left_encoder.m_current_sample_time_usecs  <-- set to current time

    right_encoder.m_a_pin_isr_previous_count   <-- right_encoder.m_a_pin_saved_count
    right_encoder.m_b_pin_isr_previous_count   <-- right_encoder.m_b_pin_saved_count
    right_encoder.m_previous_sample_time_usecs <-- right_encoder.m_current_sample_time_usecs
    right_encoder.m_a_pin_saved_count   <-- set to current value of right_encoder.m_a_pin_isr_count
    right_encoder.m_b_pin_saved_count   <-- set to current value of right_encoder.m_b_pin_isr_count
    right_encoder.m_current_sample_time_usecs <-- set to current time when previous 2 values are initialized
*/

void Encoder::unsafe_collect_two_encoder_samples(
    Encoder& left_encoder, EncoderSample& left_sample, MotorDirection left_direction,
    Encoder& right_encoder, EncoderSample& right_sample, MotorDirection right_direction
    ) 
{
    PRINT_BEGIN
    uint32_t raw_left_a_count; 
    uint32_t raw_left_b_count; 
    uint32_t raw_right_a_count; 
    uint32_t raw_right_b_count;
    const uint64_t sample_time = to_us_since_boot(get_absolute_time());
    left_encoder.prepare_next(sample_time);
    right_encoder.prepare_next(sample_time);

    const uint32_t interrupt_status = local_save_and_disable_interrupts();
    {
        /**
         *WARNING - interrupts are off in  this block
         */
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

    PRINT_ENCODER("Before left fill sample", left_encoder);
    const auto left_sample_tick_count =
            (left_encoder.m_a_pin_saved_count - left_encoder.m_a_pin_isr_previous_count)
        +   (left_encoder.m_b_pin_saved_count - left_encoder.m_b_pin_isr_previous_count);
    fill_sample(left_sample,
        left_encoder.m_current_sample_time_usecs,
        left_encoder.m_previous_sample_time_usecs,
        left_sample_tick_count,
        left_direction);
    PRINT_SAMPLE("after fill_sample left_sample", left_sample);

    PRINT_ENCODER("Before right fill sample", right_encoder);
    const auto right_sample_tick_count =
            (raw_right_a_count - right_encoder.m_a_pin_isr_previous_count)
        +   (raw_right_b_count - right_encoder.m_b_pin_isr_previous_count);
    fill_sample(right_sample,
        right_encoder.m_current_sample_time_usecs,
        right_encoder.m_previous_sample_time_usecs,
        right_sample_tick_count,
        right_direction);
    PRINT_SAMPLE("after fill_sample right_sample", right_sample);
    PRINT_END
}
void fill_sample(EncoderSample& sample, const uint64_t sample_time, const uint64_t previous_sample_time_usecs, const uint32_t sample_tick_count, MotorDirection direction)
{
    if (sample_tick_count == 0) {
        sample.s_contains_data = false;
        sample.s_elapsed_usecs = sample_time - previous_sample_time_usecs;
        sample.s_sample_tick_count = sample_tick_count;
        sample.s_motor_rpm = 0.0;
        sample.s_wheel_rpm = 0.0;
        sample.s_speed_mm_per_second = 0.0;
        sample.s_direction = direction;
    } else {
        sample.s_contains_data = true;
        sample.s_direction = direction;
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
void print_encoder(const char* tag, const Encoder& encoder)
{
    printf("Encoder %s Encoder\n", tag);
    printf("\tsample_tick_count: %lu sample_interval: %llu\n",
        (encoder.m_a_pin_saved_count - encoder.m_a_pin_isr_previous_count) + (encoder.m_b_pin_saved_count - encoder.m_b_pin_isr_previous_count),
        encoder.m_current_sample_time_usecs - encoder.m_previous_sample_time_usecs
        );
    const auto x = encoder.m_a_pin_saved_count + encoder.m_b_pin_saved_count;
    printf("\ta_pin_saved_count: %" PRIu32 " b_pin_saved_count: %" PRIu32 " SUM: %" PRIu32 "\n",
        encoder.m_a_pin_saved_count, encoder.m_b_pin_saved_count, x);
    printf("\ta_prev_count: %" PRIu32 " \tb_prev_count: %" PRIu32 " sum: %" PRIu32 "\n",
        encoder.m_a_pin_isr_previous_count, encoder.m_b_pin_isr_previous_count, encoder.m_a_pin_isr_previous_count + encoder.m_b_pin_isr_previous_count);
    printf("\tm_previous_sample_time_usecs: %" PRId64 " \n", encoder.m_previous_sample_time_usecs);
    printf("\tm_current_sample_time_usecs: %" PRId64 "\n", encoder.m_current_sample_time_usecs);
    printf("\telapsed: %" PRId64 " us\n", encoder.m_current_sample_time_usecs - encoder.m_previous_sample_time_usecs);
}

void print_sample(const char* tag, const EncoderSample& sample)
{
    printf("%s EncoderSample\n", tag);
    printf("\ts_elapsed_usecs: %llu \n", sample.s_elapsed_usecs);
    printf("\ts_sample_tick_count: %lu \n", sample.s_sample_tick_count);
    printf("\ts_direction %s\n", to_string(sample.s_direction));
    printf("\ts_motor_rpm %f\n", sample.s_motor_rpm);
    printf("\ts_wheel_rpm %f\n", sample.s_wheel_rpm);
    printf("\ts_speed_mm_per_second %f\n", sample.s_speed_mm_per_second);
    printf("\ts_wheel_travel_mm %f\n", sample.s_wheel_travel_mm);
}
