#include <enum.h>
#include <stdio.h>

bool motor_direction_to_pin_state(MotorSide side, MotorDirection md)
{
	bool r;
	switch(side) {
		case MotorSide::right:
			r = (md == MotorDirection::forward);
			break; 
		case MotorSide::left:
			r = (md == MotorDirection::backwards);
			break;
	}
	printf("motor_direction_to_pin_state side : %s md: %s r : %d\n", to_string(side), motor_direction_to_string(md), (int)r);
	return r;
}
const char* motor_direction_to_string(MotorDirection md)
{
	return (md == MotorDirection::forward) ? "F" : "B";
}
const char* to_string(MotorDirection md)
{
	return (md == MotorDirection::forward) ? "F" : "B";
}

const char* to_string(MotorSide side)
{
	return (side == MotorSide::left) ? "L": "R";
}

MotorDirection default_motor_direction(MotorSide mside)
{
	MotorDirection md;
	switch(mside) {
		case MotorSide::left:
			md = MotorDirection::forward;
			break;
		case MotorSide::right:
			md = MotorDirection::backwards;
			break;
	}
	return md;
}
MotorDirection reverse_motor_direction(MotorDirection dir)
{
	MotorDirection md;
	switch(dir) {
		case MotorDirection::forward:
			md = MotorDirection::backwards;
			break;
		case MotorDirection::backwards:
			md = MotorDirection::forward;
			break;
	}
	return md;
}

