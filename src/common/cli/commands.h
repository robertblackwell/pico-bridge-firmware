#ifndef H_commands_h
#define H_commands_h
// #include "Arduino.h"
#include "trace.h"
// #include <log.h>
#include "argv.h"

#define CLI_COMMAND_TAG_NONE            0   // an illegal command
#define CLI_COMMAND_TAG_ERROR           4   // used internally only for commands that do not parse correctly 

#define CLI_COMMAND_TAG_SPEED           1   // sets the pwm percentage value for both motors 
                                            //  -   command has 2 arguments left-percentage-pwm right-percentage-
                                            //  -   these are floating point numbers in the range -100.00 to 100.00
                                            //  -   these are convered into 4 values for the command arg structure
#define CLI_COMMAND_MOTOR_PERCENT_PWM   11  // this is the similar to the speed command except direction is given in the sign of the pwm value
                                            // 2 arguments each are floating point numbers -100.00 .. 100.00

#define CLI_COMMAND_MOTOR_RPM           5   // set the speed of motors to an rpm value - two arguments 3500-7500
#define CLI_COMMAND_MOTOR_SPEED         10  // set the speed of each motor in rpm

#define CLI_COMMAND_TAG_STOP            2   // no arguments

#define CLI_COMMAND_UPDATE_PIDARGS      7   // update the parameters of the embedded PID algorithm

#define CLI_COMMAND_READ_ENCODERS       8   // read the latest value of the encoders
#define CLI_COMMAND_RESET_ENCODERS      9   // reset the encoder recording of ticks 
#define CLI_COMMAND_TAG_ECHO            10

enum class CommandName
{
    None = 'n',
    Error = 'x',
    MotorsPwmPercent = 's',
    MotorsRpm = 'r',
    MotorsHalt = 'h',
    PidArgsUpdate = 'u',
    EncodersRead = 'e',
    Echo = 'c',
};
inline const char* to_string(CommandName en)
{
    const char* s = "Unknown";;
    switch(en) {
        case CommandName::Echo:
            s = "Echo";
            break;
        case CommandName::None:
            s = "None";
            break;
        case CommandName::Error:
            s =  "Error";
            break;
        case CommandName::MotorsPwmPercent:
            s =  "MotorsPwmPercent";
            break;
        case CommandName::MotorsRpm:
            s =  "MotorsRpm";
            break;
        case CommandName::MotorsHalt:
            s =  "MotorsHalt";
            break;
        case CommandName::PidArgsUpdate:
            s =  "PidArgsUpdate";
            break;
        case CommandName::EncodersRead:
            s =  "EncodersRead";
            break;
    }
    return s;
}
 
CommandName command_lookup(const char* first_arg);
bool validate_pwm(Argv& args, double& left, double& right);
bool validate_rpm(Argv& args, double& left, double& right);
bool validate_stop(Argv& args);
bool validate_echo(Argv& args);
bool validate_encoder_read(Argv& args);


// inline const char* clicommand2string(CliCommands cmd)
// {
//     const char* s = "Unknown";;
//     switch(cmd) {
//         case Echo:
//             s = "Echo";
//             break;
//         case None:
//             s = "None";
//             break;
//         case Error:
//             s =  "Error";
//             break;
//         case MotorsPwmPercent:
//             s =  "MotorsPwmPercent";
//             break;
//         case MotorsRpm:
//             s =  "MotorsRpm";
//             break;
//         case MotorsHalt:
//             s =  "MotorsHalt";
//             break;
//         case PidArgsUpdate:
//             s =  "PidArgsUpdate";
//             break;
//         case EncodersRead:
//             s =  "EncodersRead";
//             break;
//     }
//     return s;
// }

#if 0

typedef int CommandTagType;

struct EchoCommand {
    char line[100];
};  

struct ErrorCommand;
struct ErrorCommand {
    void create_error_msg(const char* msg, char* cline) {
        snprintf(m_line, 100, "ERROR[%s] -- %s", msg,  cline);
    }
    void create_error_msg(char* msg, char* cline) {
        snprintf(m_line, 100, "ERROR[%s] -- %s", msg,  cline);
    }
    const char* what() {
        return m_line;
    }
    char m_line[100];
};
struct UnknownCommand{
    Argv tokens;
};
/**
 * Sets the pwm value and direction for each of two motors
*/
struct MotorsPwmPercentCommand{
    double  left_pwm_percent_value;
    double  right_pwm_percent_value;
};

/**
 * @TODO - is this the same as PidRpmCommand - probably
*/
struct MotorsRpmCommand {
    double m_left_rpm;
    double m_right_rpm;

};
/**
 * Stops both motors by setting pwm to zero
*/
struct MotorsHaltCommand{
};
/**
 * Update the parameters of the PID algorithm on the micro controller
*/
struct PidArgsUpdateCommand {
    double kp;
    double ki;
    double kd;
};
/**
 * Read the n-latest sample collected for each encoder
*/
struct EncodersReadCommand {
    int m_number;
};

struct CommandBuffer {
    CommandBuffer(Argv& tokens) {
        fill_from_tokens(tokens);
    }
    CommandName identity();
    void fill_from_tokens(Argv& tokens);
    void copyTo(CommandBuffer& to);
    CommandName m_command_enum;
    union {
        EchoCommand             echo_command;
        MotorsPwmPercentCommand motors_pwm_percent_command;
        MotorsRpmCommand        motors_rpm_command;
        MotorsHaltCommand       motors_halt_command;

        PidArgsUpdateCommand    pid_args_update_command;

        EncodersReadCommand     encoders_read_command;

        UnknownCommand          unknown_command;
        ErrorCommand            error_command;
    };
};
#endif //0
#endif