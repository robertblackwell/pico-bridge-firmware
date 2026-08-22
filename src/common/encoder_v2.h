#ifndef H_encoder_h
#define H_encoder_h
#include "config.h"
#include "enum.h"
#include "encoder_sample.h"

const char* pin_state(uint8_t apin_state, uint8_t bpin_state);

struct Encoder;
void encoder_common_isr(Encoder* encoder_ptr);

/**
 * An instance of this class represents the encoder on ONE of the robots drive motor.
 * The primary function of such an instance is to count the number of interrupts generated
 * by that motors encoder.
 *
 * The change in encoder interrupt count for both robot motors can be used to estimate
 * the speed and direction of the robot. That estimation is NOT performed by this class/module
 *
 * This implementation creates 2 instances of the Encoder class, one for each of the robots 2 drive motor, and makes
 * those instances available through static get_encoder_xxxx() functions.
 *
 * Each encoder has 2 interrupt sources; a-pin and b-pin.
 *
 * Each encoder instance attaches Interrupt Service Routines (ISR) to both interrupt sources
 * for that encoder and counts the absolute number of interrupts for each source. Here absolute
 * means that the ISR does not consider direction of rotation. When the motor is turning (in either direction)
 * the interrupt count increases.
 *
 * As noted else where the difference in the direction of rotation between the
 * left and right motor is handled in the wiring arrangement not in software.
 * Hence determining direction of rotation in the ISR (by comparing the relative state of a-pin and b-pin)
 * would produce a invalid/contradictory result.
 *
 * Thus direction of rotation of each motor must be solicited from either dri0002.h/ccp or motion.h/cpp.
 *
 * WARNING: The kinematic calculations that estimate robot velocity/position:
 * -    are performed periodically and the time interval between such estimates should be small
 * -    the period between succesive kinematic calculations is called the sample period
 * -    the direction of rotation of the motors must not change during a sample period
 * -    the speed of rotation is permitted to change during a sample period
 * -    combining the previous 2 points implies that, for each motor, there should be a
 *      period of no/zero motion between sample periods of motion in opposite directions.
 *
 * TODO: use PIO to coount interrupts.
 */
struct Encoder
{
    public:
    
    static Encoder* get_encoder_left();
    static Encoder* get_encoder_right();

    /**
    * WARNING - This function turns off interrupts
    * This function captures the current value of the a-pin and b-pin interrupt counter for both the
    * left motor and right motor encoders and uses those values to compute kinematic values for the robot.
    * The collected data is stored in left_sample and right_sample respectively.
    */
    static void unsafe_collect_two_encoder_samples(
        Encoder& left_encoder, EncoderSample& left_sample, MotorDirection left_direction,
        Encoder& right_encoder, EncoderSample& right_sample, MotorDirection right_direction
        );

    Encoder(MotorSide side, const char* name, int encoder_a_pin, int encoder_b_pin);
    void init(uint64_t current_time_us);
    void prepare_next(uint64_t current_time_us);

    void start_handling_interrupts() const;

    MotorSide m_side;
    const char * m_name;
    EncoderSample m_sample;
    /**
     * Following are used to save values collected by the run() function.
    */
    int m_state;
    int m_encoder_a_pin;
    int m_encoder_b_pin;

    /**
     * properties for revised enoder handling Aug 2026
     * Goal is to save enough information to compute:
     * -    change since last sample - time interval and change in isr counters
     * -    validate all calculations of position and speed
     */
    volatile uint32_t m_a_pin_isr_count;        // This counter is incremented by the A  pin isr

    uint64_t m_current_sample_time_usecs;

    uint64_t m_previous_sample_time_usecs;      // Absolute time at which the previous sample was taken
    uint32_t m_a_pin_isr_previous_count;        // isr counter value at previous sample

    uint64_t m_sample_interval_usecs;           // 
    uint32_t m_a_pin_saved_count;               // saved value of latest a pin isr counter

    volatile uint32_t m_b_pin_isr_count;        // b pin isr counter
    uint32_t m_b_pin_isr_previous_count;        // isr counter value at previous sample
    uint32_t m_b_pin_saved_count;               // saved value of latest b pin isr counter
};

#endif