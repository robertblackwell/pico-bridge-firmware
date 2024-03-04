#ifndef H_motion_h
#define H_motion_h
#undef TRACE_ON
#include <task.h>
#include <dri0002.h>
#include <encoder.h>
#include <pid.h>

class MotionControl;
void dump_config(MotionControl* mp);

/**
 * NOTE: left and right motors need to rotate in oposite direction to go straight.
 */
class MotionControl : public Taskable
{
    public:
    struct RpmValue
    {
        double value{};
        MotorDirection direction;
        bool is_zero{};
        RpmValue(){}
        explicit RpmValue(double raw_value){
            
            if(((-1.0*PID_RPM_MIN) < raw_value) && (raw_value < PID_RPM_MIN)) {
                value = 0.0;
                is_zero = true;
                direction = MotorDirection::forward;
            } else if(raw_value < 0.0) {
                value = -1.0 * raw_value;
                is_zero = false;
                direction = MotorDirection::backwards;
            } else {
                value = raw_value;
                is_zero = false;
                direction = MotorDirection::forward;
            }
        }
        [[nodiscard]] double raw_double_value() const{
            return (double)(direction) * value;
        }
    };
    struct Side
    {
        Encoder        *m_encoder_ptr;
        Pid             m_pid{};
        RpmValue        m_rpm_value;
        MotorSide       m_dri0002_motor_side;
        MotorDirection  m_default_direction;
        double          m_rpm_target;
        bool            m_pid_active;
    };

public:

    MotionControl();
    MotionControl(DRI0002V1_4 *dri0002, Encoder *encoder_left_ptr, Encoder *encoder_right_ptr);
    MotionControl(DRI0002V1_4 &dri0002, Encoder &encoder_left_ptr, Encoder &encoder_right_ptr);
    void begin(DRI0002V1_4 *dri0002, Encoder *encoder_left_ptr, Encoder *encoder_right_ptr);
    PwmPiPico* dri0002_left_side();
    PwmPiPico* dri002_right_side(); 
    void set_pwm_direction(double pwm_motor_left, MotorDirection direction_left, double pwm_motor_right, MotorDirection direction_right);
    void pid_set_rpm(double left_rpm, double right_rpm);
    void set_raw_pwm_percent(double percent_1, double percent_2) const;
    void set_pwm_percent(double percent_1, double percent_2) const;
    void update_pid(double kp, double ki, double kd);
    void stop_all() const;
    // void read_encoders(); // not sure we need this

    void apply_pid();
    void set_rpm_one_side(Side& side, RpmValue request);
    void run();

    DRI0002V1_4 *m_dri0002_ptr;
    Side m_left_side;
    Side m_right_side;
};

#endif