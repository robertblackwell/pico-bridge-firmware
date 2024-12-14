#ifndef H_motion_h
#define H_motion_h
#undef TRACE_ON
#include <task.h>
#include <cmath>
#include <dri0002.h>
#include <encoder_v2.h>

// using namespace closed_loop;

class MotionControl;
void dump_config(MotionControl* mp);

/**
 * NOTE: left and right motors need to rotate in oposite direction to go straight.
 */
struct MotionControl
{
    template<const int MIN, const int MAX>
    struct FloatValue
    {
        double m_value;
        FloatValue()
        {
            m_value=0.0;
        }
        explicit FloatValue(double raw_value)
        {
            m_value = raw_value;
        }
        FloatValue& operator=(const FloatValue v)
        {
            m_value = v.m_value;
            return *this;
        }
        double signed_value()
        {
            return m_value;
        }
        double abs_value()
        {
            return fabs(m_value);
        }
        [[nodiscard]] double raw_double_value() const{
            return (double) m_value;
        }
        inline MotorDirection direction()
        {
            return (m_value < 0.0) ? MotorDirection::backwards: MotorDirection::forward;
        }
        inline bool is_zero()
        {
            return (((-1.0*(double)MIN) < m_value) && (m_value < (double)MIN));
        }
    };
    using RpmValue = FloatValue<PID_RPM_INT_MIN, PID_RPM_INT_MAX>;
    using PwmValue = FloatValue<PID_PWM_INT_MIN, PID_PWM_INT_MAX>;


    MotionControl();
    MotionControl(DRI0002V1_4 *dri0002,
                  Encoder *encoder_left_ptr, Encoder *encoder_right_ptr
                //   CustomController* left_closed_loop_controller,
                //   CustomController* right_closed_loop_controller
                  );

    MotionControl(DRI0002V1_4 &dri0002, Encoder &encoder_left_ptr, Encoder &encoder_right_ptr); //NOLINT

    void begin(
            DRI0002V1_4 *dri0002,
            Encoder *encoder_left_ptr,
            Encoder *encoder_right_ptr
            // CustomController* left_closed_loop_controller_ptr,
            // CustomController* right_cloed_loop_controller_ptr
            );

    void pid_set_rpm(double left_rpm, double right_rpm);
    void set_raw_pwm_percent(double percent_1, double percent_2) const;
    void set_pwm_percent(double percent_1, double percent_2) const;
    void stop_all() const;

    void set_rpm_one_side(DriveSide side, RpmValue request);

    [[nodiscard]] bool verify_left_rpm_settable(float rpm_value) const;
    [[nodiscard]] bool verify_right_rpm_settable(float rpm_value) const;
    [[nodiscard]] bool verify_side_rpm_settable(DriveSide side, float rpm) const;

    /**
     * Get either the left or right encoder ptr
     */
    inline Encoder* encoder_ptr(DriveSide side)
    {
        return (side == DriveSide::left) ? m_left_encoder_ptr : m_right_encoder_ptr;
    }

    /**
     * Get either the left or right closed loop controller ptr
     */
    // inline CustomController* closed_loop_controller_ptr(DriveSide side)
    // {
    //     return (side == DriveSide::left) ? m_left_closed_loop_controller_ptr : m_right_closed_loop_controller_ptr;
    // }

    /**
     * Get from dri0002 the PwmPiPico instance responsible for either the left or right motor
     */
    [[nodiscard]] inline PwmPiPico* get_pwm_pipico_ptr(DriveSide side) const
    {
        return m_dri0002_ptr->get_pwm_pico(side);
    }

    /**
     * Get the current pwm value for either the left or right motor
     */
    inline PwmValue get_current_pwm_value(DriveSide side)
    {
        return (side == DriveSide::left) ? m_left_current_pwm : m_right_current_pwm;
    }

    /**
     * Set the current pwm value for either the left or right motor
     */
    inline void set_current_pwm_value(DriveSide side, PwmValue v)
    {
        if(side == DriveSide::left)
            m_left_current_pwm = v;
        else
            m_right_current_pwm = v;
    }

    /**
     * Get the current rpm target value for either the left or right motor
     */
    [[nodiscard]] inline RpmValue get_rpm_target(DriveSide side) const
    {
        return (side == DriveSide::left) ? m_left_rpm_target : m_right_rpm_target;
    }

    /**
     * Set the current rpm target value for either the left or right motor
     */
    inline void set_rpm_target(DriveSide side, RpmValue v)
    {
        if(side == DriveSide::left)
            m_left_rpm_target.m_value = v.m_value;
        else
            m_right_rpm_target.m_value = v.m_value;
    }

    DRI0002V1_4*        m_dri0002_ptr;
    Encoder*            m_left_encoder_ptr;
    Encoder*            m_right_encoder_ptr;
    PwmValue            m_left_current_pwm;
    PwmValue            m_right_current_pwm;
    RpmValue            m_left_rpm_target;
    RpmValue            m_right_rpm_target;

};

#endif