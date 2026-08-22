#include "unittest.h"
#include "wheel_speed_value.h"
#include <cstdio>

/**
 * Test WheelSpeedValue constructor and clamping logic.
 */
static int test_wheel_speed_value_constructor() {
    // SCL_MIN_VELOCITY is 0.0001
    // SCL_MAX_VELOCITY is 0.1

    // Test zero/near-zero
    WheelSpeedValue v0(0.0);
    UT_TRUE(v0.is_zero);
    UT_EQUAL_DOUBLE(v0.value, 0.0);

    WheelSpeedValue v_near_zero(0.00005);
    UT_TRUE(v_near_zero.is_zero);
    UT_EQUAL_DOUBLE(v_near_zero.value, 0.0);

    // Test positive within range
    WheelSpeedValue v_pos(0.05);
    UT_TRUE(!v_pos.is_zero);
    UT_EQUAL_DOUBLE(v_pos.value, 0.05);
    UT_TRUE((v_pos.direction == MotorDirection::forward));

    // Test negative within range
    WheelSpeedValue v_neg(-0.05);
    UT_TRUE(!v_neg.is_zero);
    UT_EQUAL_DOUBLE(v_neg.value, 0.05); // value is absolute
    UT_TRUE((v_neg.direction == MotorDirection::backwards));

    // Test positive clamping
    WheelSpeedValue v_pos_clamp(0.2);
    UT_EQUAL_DOUBLE(v_pos_clamp.value, 0.1); // SCL_MAX_VELOCITY
    UT_TRUE((v_pos_clamp.direction == MotorDirection::forward));

    // Test negative clamping
    WheelSpeedValue v_neg_clamp(-0.2);
    // Instructor note: The implementation sets value = -1.0 * SCL_MAX_VELOCITY for clamped negative values.
    UT_EQUAL_DOUBLE(v_neg_clamp.value, -0.1); // -SCL_MAX_VELOCITY
    UT_TRUE((v_neg_clamp.direction == MotorDirection::backwards));

    return 0;
}

/**
 * Test raw_double_value() conversion.
 */
int test_wheel_speed_raw_double() {
    WheelSpeedValue v_pos(0.05);
    UT_EQUAL_DOUBLE(v_pos.raw_double_value(), 0.05);

    WheelSpeedValue v_neg(-0.05);
    UT_EQUAL_DOUBLE(v_neg.raw_double_value(), -0.05);

    WheelSpeedValue v_zero(0.0);
    UT_EQUAL_DOUBLE(v_zero.raw_double_value(), 0.0);

    return 0;
}

/**
 * Test update_permitted() logic.
 * Rule: Change of direction must go through zero speed.
 */
int test_wheel_speed_update_permitted() {
    WheelSpeedValue v_pos(0.05);
    WheelSpeedValue v_pos_faster(0.08);
    WheelSpeedValue v_neg(-0.05);
    WheelSpeedValue v_zero(0.0);

    // Same direction: permitted
    UT_TRUE(v_pos.update_permitted(v_pos_faster));
    
    // To zero: permitted
    UT_TRUE(v_pos.update_permitted(v_zero));
    
    // From zero: permitted
    UT_TRUE(v_zero.update_permitted(v_pos));
    UT_TRUE(v_zero.update_permitted(v_neg));

    // Opposite direction NOT through zero: NOT permitted
    UT_TRUE(!v_pos.update_permitted(v_neg));
    UT_TRUE(!v_neg.update_permitted(v_pos));

    return 0;
}

/**
 * Test WheelSpeedRequest::update() logic.
 */
int test_wheel_speed_request_update() {
    WheelSpeedRequest req;
    // Initial state is zero
    UT_TRUE(req.left_wheel_target_speed.is_zero);
    UT_TRUE(req.right_wheel_target_speed.is_zero);

    WheelSpeedValue v_pos(0.05);
    WheelSpeedValue v_neg(-0.05);

    // Valid update from zero
    UT_TRUE(req.update(v_pos, v_neg));
    UT_EQUAL_DOUBLE(req.left_wheel_target_speed.raw_double_value(), 0.05);
    UT_EQUAL_DOUBLE(req.right_wheel_target_speed.raw_double_value(), -0.05);

    // Invalid update: left wheel tries to jump from positive to negative
    UT_TRUE(!req.update(v_neg, v_neg));
    // Values should not have changed
    UT_EQUAL_DOUBLE(req.left_wheel_target_speed.raw_double_value(), 0.05);

    // Valid update to zero
    WheelSpeedValue v_zero(0.0);
    UT_TRUE(req.update(v_zero, v_zero));
    UT_TRUE(req.left_wheel_target_speed.is_zero);
    UT_TRUE(req.right_wheel_target_speed.is_zero);

    return 0;
}

int main() {
    UT_ADD(test_wheel_speed_value_constructor);
    UT_ADD(test_wheel_speed_raw_double);
    UT_ADD(test_wheel_speed_update_permitted);
    UT_ADD(test_wheel_speed_request_update);

    return UTRun();
}
