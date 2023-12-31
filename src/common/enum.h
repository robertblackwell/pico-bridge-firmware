
#ifndef H_enum_h
#define H_enum_h
#include <config.h>

enum MotorDirection {
    forward = 1,
    backwards = -1,
};

// MotorDirection reverse_direction(MotorDirection d) 
// {
//     switch(d) {
//         case MotorDirection::forward:
//             return MotorDirection::backwards;
//             break;
//         default:
//             return MotorDirection::forward;
//             break;
//     }
// }

enum MotorSide 
{
	left = MOTOR_LEFT_DRI0002_SIDE,
	right= MOTOR_RIGHT_DRI0002_SIDE
};

MotorDirection default_motor_direction(MotorSide mside);
MotorDirection reverse_motor_direction(MotorDirection dir);
bool motor_direction_to_pin_state(MotorSide side, MotorDirection md);
const char* motor_direction_to_string(MotorDirection md);
const char* to_string(MotorSide side);
#endif