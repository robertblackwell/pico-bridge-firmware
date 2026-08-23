#undef FTRACE_ON
#include <functional>
#include <cstdio>
#if PICO_ON_DEVICE
#include <pico/stdlib.h>
#endif
#include "hw_utils.h"
#include "trace.h"
#include "task.h"
#include "cli/argv.h"
#include "cli/commands.h"
#include "transport/buffers.h"
#include "transport/transmit_buffer_pool.h"
#include "transport/transport.h"
#include "robot.h"

using namespace transport::buffer;
void execute_commands(Argv& args, transport::buffer::Handle bh)
{
    FTRACE("This is what treader got [%s]\n", sb_buffer_as_cstr(bh));
    FDUMP_TOKENS(args, "Dump tokens message")
    CommandName enumname = command_lookup(args.token_at(0));
    FTRACE("command_lookup result: [%s]", to_string(enumname))

    #if 1
    FTRACE("switch on command name %s", to_string(enumname))
    switch (enumname) {
        case CommandName::None:
            transport::send_command_error("none");
            break;
        case CommandName::Error:
            transport::send_command_error("Invalid command %s", sb_buffer_as_cstr(bh));
            break;
        case CommandName::MotorsPwmPercent: {
            double left_pwm, right_pwm;
            if(validate_pwm(args, left_pwm, right_pwm)) {
                robot::set_raw_pwm_percent(left_pwm, right_pwm);
                transport::send_command_ok("MotorPwmPercent %f  %f", left_pwm, right_pwm);
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::MotorsRpm: {
            double left_rpm, right_rpm;
            if(validate_rpm(args, left_rpm, right_rpm)) {
                printf("%f %f\n", left_rpm, right_rpm);
                if(robot::set_rpm(left_rpm, right_rpm)) {
                    transport::send_command_ok("MotorRpmCommand");
                } else {
                    transport::send_command_error("Command %s failed probably trying to change direction without stopping\n", to_string(enumname), sb_buffer_as_cstr(bh));
                }
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::MotorsHalt: {
            if(validate_encoder_read(args)) {
                printf("%s\n", to_string(enumname));
                robot::stop_all();
                transport::send_command_ok("StopCommand");
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::EncodersRead:
        {
            int number = 1;
            if(validate_encoder_read(args)) {
                // printf("%s\n", to_string(enumname));
                Handle h = tx_pool::allocate();
                robot::tojson_encoder_samples(h);
                transport::send_json_response(&h);
                // printf("End of read encoder cmd \n");
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::EncodersStream:
        {
            int interval_ms = 0;
            if(validate_encoders_stream(args, interval_ms)) {
                printf("%s interval_ms: %d\n", to_string(enumname), interval_ms);
                // Handle h = tx_pool::allocate();
                // tojson_encoder_samples(h);
                // transport::send_json_response(&h);
                // printf("End of read encoder cmd \n");
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::PidArgsUpdate: {
            transport::send_command_ok("PidArgUpdate command not implemented [%s]", sb_buffer_as_cstr(bh));
            break;
        }
        case CommandName::Echo: {
            if(validate_encoder_read(args)) {
                printf("%s\n", to_string(enumname));
                transport::send_command_ok("Echo command [%s]", sb_buffer_as_cstr(bh));
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::LoadTest: {
            const char* response_source = "qwertyuiopasdfghjklzxcvbnm1234567890~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:ZXCVBNM<>?";
            int count, response_length, nbr_per_second;
            if(validate_loadtest(args, count, response_length, nbr_per_second)) {
                int interval_ms = (int)(1000.00 / (float)nbr_per_second);
                uint64_t start_time = micros();
                for(int i = 0; i < count; i++) {
                    uint64_t  m = micros();
                    transport::send_command_ok("Loadtest time: %ld count: %i interval: %d [%s]", m, i, interval_ms, response_source);
                    #if PICO_ON_DEVICE
                    sleep_ms(interval_ms);
                    #endif
                }
                uint64_t  end_time = micros();
                transport::send_command_ok("Elapsed time %llu micro seconds", (end_time - start_time));
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::SoftwareReset:
            HwUtils::pico_reset();
            // *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
            break;

        case CommandName::WheelVelocity: {
            double left_vel, right_vel;
            if(validate_wheel_velocity( args, left_vel, right_vel)) {
                auto status = robot::set_wheel_velocity_ms(left_vel, right_vel);
                if (status == robot::SetWheelVelocityStatus::Ok)
                    transport::send_command_ok("WheelVelocity %f  %f", left_vel, right_vel);
                else
                    transport::send_command_error("WheelVelocity %f  %f status: %s", left_vel, right_vel, to_string(status));
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::PidOnOff:
            robot::pid_toggle();
            break;

        case CommandName::Help:
            printf("Commands: \n");
            printf("    v/vel left      right       Set wheel velocity for each wheel m/s -0.11 +.11\n");
            printf("    w/pwm left      right       Set pwm percentage for each motors, values in range -100 .. 100\n");
            printf("    r/rpm left_rpm  right_rpm   Set speed of each motor in revs per minute\n");
            printf("    s                           Stop both motors \n");
            printf("    e                           Read both encoders\n");
            printf("    c                           Echo what ever follows the 'c'\n");
            printf("    b                           Software Reset\n");
            printf("    p                           Toggle pid controller on/off\n");
            printf("    ?                           Help - print this message\n");
        
        default: {
            FTRACE("Execute command case default\n");
            transport::send_command_error("Unknowncommand");
            break;
        }

    }


    #endif
}
