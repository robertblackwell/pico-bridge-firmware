#ifndef H_config_h
#define H_config_h

/*
    Encodes on pololu gear motors
    Red     motor power connect to motor controller
    Black   motor power connecto to motor controller
    Green   encoder ground
    Blue    encoder Vcc (3.5 - 20V)
    Yellow  encoder A output
    White   encoder B output       

    Note the DFRobot H-Bridge version 1.4 of the L298N board does NOT adjusts motor direction of rotation
    for side. Thus a HIGH signal on both M1 and M2 will cause both motors to rotate in the same 
    direction namely a Right Hand Thumb rule - in the direction of the fingers when holding the 
    motor with the thumb pointing along the drive shaft towards the attached wheel.

    A Low value on M1 and M2 will give a left hand (LHTR) direction of rotation.

    When used on a vehicle with differential drive
    -   the right hand motor must turn LeftHandThumb
    -   the left hand wheel must turn RightHandThumb 
    in order to get forward motion, Reverse motion is the opposite.

    On a robot vehicle the question as to which motor is left(RHTR) and which is right(LHTR)
    is a design/arbitary decision. Our decision is:

    left/RHTR -> correspponds to E1 M1 on the L298N controller board and pin 5(E1) 4(M1)
                 on our Mega2560 board

    right/LHTR-> corresponds to E2 M2 on the L2298N board and 
*/
#define PIPICO
#ifdef MEDA2560
// mega2560 config
// motor1
#define MOTOR_LEFT_ID 111
#define MOTOR_LEFT_NAME "left"
#define	MOTOR_LEFT_PWM_PIN 5                //7 //9 //E2 core electronic gearmotors
#define MOTOR_LEFT_DIRECTION_SELECT_PIN 4   //6 //8   M2
#define MOTOR_LEFT_ENCODER_A_INT 20         // motor yellow lead
#define MOTOR_LEFT_ENCODER_B_INT 21         // motor white lead  
#define PID_LEFT_KP 0.05
#define PID_LEFT_KI 0.0
#define PID_LEFT_KD 0.0      
// motor2
#define MOTOR_RIGHT_ID 222
#define MOTOR_RIGHT_NAME "right"
#define MOTOR_RIGHT_PWM_PIN 7               //5 //E1
#define MOTOR_RIGHT_DIRECTION_SELECT_PIN 6  //4 //M1
#define MOTOR_RIGHT_ENCODER_A_INT 3         // motor yellow lead
#define MOTOR_RIGHT_ENCODER_B_INT 2         // motor white lead
#define PID_RIGHT_KP 0.05
#define PID_RIGHT_KI 0.0
#define PID_RIGHT_KD 0.0      

#elif defined(PIPICO)
#define MOTOR_LEFT_ID 2222
#define MOTOR_LEFT_DRI0002_SIDE 2
#define MOTOR_LEFT_NAME "left"
#define DR0002_PIN_E2 3
#define DR0002_PIN_M2 2
#define EC2_PIN_ENCODER_A 1
#define EC2_PIN_ENCODER_B 0


#define	MOTOR_LEFT_PWM_PIN                  3 //DR0002_PIN_E2
#define MOTOR_LEFT_DIRECTION_SELECT_PIN     2 //DR0002_PIN_M2
#define MOTOR_LEFT_ENCODER_A_INT 1         // motor yellow lead
#define MOTOR_LEFT_ENCODER_B_INT 0         // motor white lead  

#define MOTOR_RIGHT_ID 1111
#define MOTOR_RIGHT_DRI0002_SIDE 1
#define MOTOR_RIGHT_NAME "right"
#define DR0002_PIN_E1 15
#define DR0002_PIN_M1 14
#define EC1_PIN_ENCODER_A 13
#define EC1_PIN_ENCODER_B 12

#define MOTOR_RIGHT_PWM_PIN                 15 //DR0002_PIN_E1
#define MOTOR_RIGHT_DIRECTION_SELECT_PIN    14 //DR0002_PIN_M1
#define MOTOR_RIGHT_ENCODER_A_INT 13         // motor yellow lead
#define MOTOR_RIGHT_ENCODER_B_INT 12         // motor white lead


#define PID_LEFT_KP_DEFAULT 0.0
#define PID_LEFT_KI_DEFAULT 0.01
#define PID_LEFT_KD_DEFAULT 0.0001      

#define PID_RIGHT_KP_DEFAULT 0.0
#define PID_RIGHT_KI_DEFAULT 0.01
#define PID_RIGHT_KD_DEFAULT 0.0001    

#define PID_RPM_ZERO 100.0
#define PID_RPM_MIN 3500.0
#define PID_RPM_MAX 7500.0
#define PID_RPM_INT_ZERO 100
#define PID_RPM_INT_MIN 3500
#define PID_RPM_INT_MAX 7500

#define PID_PWM_MAX 100.0
#define PID_PWM_MIN 25.0
#define PID_PWM_INT_MAX 100
#define PID_PWM_INT_MIN 25

/**
 * Used by encoder isr and encoder_sample collection
*/
#define ISR_COMMON_FUNCTION // if defined eachs ecnoders A and B pin ISRs call a common function to
                            // do their work. If not defined the ISR body is repeated in each ISR
#define ISR_START_EXLICIT   // if defined encoder interrupts are not enabled until Encoder::start_interrupts() is called
#define ISR_ATTACH_TO_PIN_A // if defined an encoder ISR will be attached to pin A for all motors.
#undef ISR_ATTACH_TO_PIN_B // if defined an encoder ISR will be attached to pin B for all motors.

#if defined(ISR_ATTACH_TO_PIN_A) && defined(ISR_ATTACH_TO_PIN_B)
    #define ISR_INTR_PER_MOTOR_REVOLUTION 48
    #define ISR_SAMPLE_SIZE (ISR_INTR_PER_MOTOR_REVOLUTION * 6)
#elif defined(ISR_ATTACH_TO_PIN_A) || defined(ISR_ATTACH_TO_PIN_B)
    #define ISR_INTR_PER_MOTOR_REVOLUTION 24
    #define ISR_SAMPLE_SIZE (ISR_INTR_PER_MOTOR_REVOLUTION * 6)
#endif
//sample size should be a number of motor revolutions
// There is a tradeoff - 
//   longer intervals give more stability to the sample rpm and speed values, but 
//   longer intervals also mean less ability to react fast and average out acceleration
//
#define ISR_PI_VALUE 3.14159265
#define ISR_SECS_IN_MINUTE 60.0
#define ISR_GEAR_RATIO 226.76
#define ISR_WHEEL_DIAMETER_MM 70.0



#endif
#endif