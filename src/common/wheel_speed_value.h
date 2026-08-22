#ifndef H_WHEEL_SPEED_VALUE_H
#define H_WHEEL_SPEED_VALUE_H
#include "enum.h"

/**
 * Represents the speed of a robot wheel in m/s. Applies mi and max speed limits and keeps
 * is_zero and dirction forward/backwards explicitly.
 */
struct WheelSpeedValue
{
    double value{};
    MotorDirection direction;
    bool is_zero{};
    WheelSpeedValue(): direction(MotorDirection::backwards), is_zero(true){}
    explicit WheelSpeedValue(const double raw_value){

        if(((-1.0 * SCL_MIN_VELOCITY) < raw_value) && (raw_value < SCL_MIN_VELOCITY)) {
            value = 0.0;
            is_zero = true;
            direction = MotorDirection::forward;
        } else if (raw_value < -1.0 * SCL_MAX_VELOCITY) {
            value = -1.0* SCL_MAX_VELOCITY;
            is_zero = false;
            direction = MotorDirection::backwards;
        } else if (SCL_MAX_VELOCITY < raw_value) {
            value = SCL_MAX_VELOCITY;
            is_zero = false;
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
    WheelSpeedValue(const WheelSpeedValue &other) :direction(other.direction), is_zero(other.is_zero)
    {
        value = other.value;
    }

    /**
     * Returns the wheel speed as a double positive or zero value.
     * @return
     */
    [[nodiscard]] double raw_double_value() const{
        return static_cast<double>(direction) * value;
    }

    /**
     * This function will check to see if the change of wheel speed is valid: change of direction must go
     * through zero speed. If valid will return true and apply the update. If not valid return false and
     * do ot update the wheel speed.
     * @param new_wheel_speed
     * @return
     */
    bool update_permitted(const WheelSpeedValue new_wheel_speed) const {
        if ((new_wheel_speed.is_zero)||(is_zero)) {
            // value = new_wheel_speed.value;
            // is_zero = new_wheel_speed.is_zero;
            // direction = new_wheel_speed.direction;
            return true;
        } else if (direction == new_wheel_speed.direction) {
            // value = new_wheel_speed.value;
            return true;
        } else {
            return false;
        }
    }
};

/**
 * This struct represents a request for the speed of both robot wheels. It embodies the rule that a wheel must
 * be stopped before it can change direction.
 */
struct WheelSpeedRequest {
    WheelSpeedValue left_wheel_target_speed;
    WheelSpeedValue right_wheel_target_speed;
    WheelSpeedRequest(){}
    bool update(const WheelSpeedValue& new_left_wheel_target_speed, const WheelSpeedValue &new_right_wheel_target_speed)
    {
        if (left_wheel_target_speed.update_permitted(new_left_wheel_target_speed) && right_wheel_target_speed.update_permitted(new_right_wheel_target_speed)) {
            left_wheel_target_speed = new_left_wheel_target_speed;
            right_wheel_target_speed = new_right_wheel_target_speed;
            return true;
        }
        return false;
    }
};

#endif
