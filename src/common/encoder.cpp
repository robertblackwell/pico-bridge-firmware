#undef FTRACE_ON
#undef FDEBUG_ON
#include <stdio.h>
#include <pico/stdio.h>

#include <pico_gpio_irq_dispatcher.h>
#include "config.h"
#include "trace.h"
#include "encoder.h"
#include "reporter.h"

#define ISR_SAMPLE_SIZE 12
#define ISR_INTR_PER_MOTOR_REVOLUTION 24
#define ISR_PI_VALUE 3.14159265
#define ISR_SECS_IN_MINUTE 60
#define ISR_GEAR_RATIO 220
#define ISR_WHEEL_DIAMETER_MM 60

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
void Encoder::start_interrupts()
{
    attachGpioInterrupt(m_encoder_a_pin, m_isr_a);
    attachGpioInterrupt(m_encoder_b_pin, m_isr_b);
}
bool Encoder::available()
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
    gpio_irq_backon(m_encoder_a_pin);
    gpio_irq_backon(m_encoder_b_pin);
    // interrupts();

    if(! m_sample.s_available) {
        // log_print("encoder.run name: ", m_name, " addr:", (long)this, "did not find a new sample - motor speed is probably : 0 mm/sec  \n");
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
void Encoder::reset_sample()
{
    m_isr_count = 0;
    // this->m_isr_interval_count = 0;
    // this->m_isr_saved_sample_sum = 0;
    // this->m_isr_current_sample_sum = 0;
    // this->m_isr_new_sample_sum_available_flag = false;
}
#if 0
// global definition of 2 encoder objects
Encoder g_encoder_left;
Encoder g_encoder_right;
#endif

void encoder_common_isr(Encoder* encoder_ptr)
{
    #if 1
    int a_value = digital_read(encoder_ptr->m_encoder_a_pin);
    int b_value = digital_read(encoder_ptr->m_encoder_b_pin);
    encoder_ptr->m_apin_state = a_value;
    encoder_ptr->m_bpin_state = b_value;
    #if 0
    state_machine(a_value, b_value);
    #endif
    #endif
    #if 1
    Encoder* ep = encoder_ptr;
	if(ep->m_isr_count == 0) {
		ep->m_isr_interval_count = 0;
		ep->m_isr_previous_millis = micros();
		ep->m_isr_current_sample_sum = 0;
		ep->m_isr_saved_sample_sum = 0;
	} else {
		long musecs = micros();
        ep->m_isr_timestamp_musecs = musecs;
		long interval = (musecs - ep->m_isr_previous_millis);
		ep->m_isr_previous_millis = musecs;
		ep->m_isr_current_sample_sum += interval;
	}
	ep->m_isr_interval_count++;
	if(ep->m_isr_interval_count >= ISR_SAMPLE_SIZE) {
		ep->m_isr_interval_count = 0;
		ep->m_isr_saved_sample_sum = ep->m_isr_current_sample_sum;
		ep->m_isr_current_sample_sum = 0;
        ep->m_isr_new_sample_sum_available_flag = true;
	}
	ep->m_isr_count++;
    #endif
}
#if 0
void irq_handler_left_apin()
{   
    // printf("irq_handler_left_apin\n");
    // return;
    Encoder* tmp = reinterpret_cast<Encoder*>(&g_encoder_left);
    encoder_common_isr(tmp);
}
void irq_handler_left_bpin()
{   
    // printf("irq_handler_left_bpin\n");
    return;
    Encoder* tmp = reinterpret_cast<Encoder*>(&g_encoder_left);
    encoder_common_isr(tmp);
}
void irq_handler_right_apin()
{
    // printf("irq_handler_right_apin\n");
    // return;
    Encoder* tmp = reinterpret_cast<Encoder*>(&g_encoder_right);
    encoder_common_isr(tmp);
}
void irq_handler_right_bpin()
{
    // printf("irq_handler_right_bpin\n");
    return;
    Encoder* tmp = reinterpret_cast<Encoder*>(&g_encoder_right);
    encoder_common_isr(tmp);
}
Encoder* make_encoder_left()
{
    g_encoder_left.m_isr_a = irq_handler_left_apin;
    g_encoder_left.m_isr_b = irq_handler_left_bpin;

    return (&g_encoder_left);
}
Encoder* make_encoder_right()
{
    g_encoder_right.m_isr_a = irq_handler_right_apin;
    g_encoder_right.m_isr_b = irq_handler_right_bpin;
    return (&g_encoder_right);
}
#endif