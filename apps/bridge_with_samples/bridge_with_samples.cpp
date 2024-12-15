#undef FTRACE_ON
#include <functional>
#include <cstdio>
#include <pico/stdlib.h>
#include <hardware/gpio.h>
#include <hardware/pwm.h>
#include <tusb.h>

#include "trace.h"
#include "dri0002.h"
#include "config.h"
#include "encoder_v2.h"
#include "task.h"
#include "motion.h"
#include "reporter.h"
#include "cli/argv.h"
#include "cli/commands.h"
#include "transport/buffers.h"
#include "transport/transmit_buffer_pool.h"
#include "transport/transport.h"
#include "robot.h"

#include "pico/stdlib.h"
#include <pico/error.h>
#include <cli/execute_commands.h>
#include <version.h>

using namespace transport::buffer;
static void local_execute_commands(Argv& args, transport::buffer::Handle bh);
void do_commands();
void heart_beat();

transport::Reader treader;
DRI0002V1_4 dri0002{
		MOTOR_RIGHT_DRI0002_SIDE, 
		MOTOR_RIGHT_PWM_PIN, 				// E1
		MOTOR_RIGHT_DIRECTION_SELECT_PIN, 	// M1
		
		MOTOR_LEFT_DRI0002_SIDE, 
		MOTOR_LEFT_PWM_PIN, 				// E2
		MOTOR_LEFT_DIRECTION_SELECT_PIN	    // E2
};
Encoder* encoder_left_ptr;
Encoder* encoder_right_ptr;
MotionControl motion_controller{&dri0002, encoder_left_ptr, encoder_right_ptr};

void encoder_samples();
int main()
{
	transport::transport_init();
	treader.begin();
	encoder_left_ptr = Encoder::encoder_left_start();
	encoder_right_ptr = Encoder::encoder_right_start();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
//	print_fmt("bridge (version:%s ) starting ... \n", VERSION_NUMBER);
    transport::send_boot_message("bridge_with_samplesmake (version:%s ) starting ... \n", VERSION_NUMBER);
	Task cli_task(20, do_commands);
	Task heart_beat_task(1000, heart_beat);
	/**
	 * Setup a task that reports encoder samples regularly, but it will not start reporting 
	 * until an encoders_stream command is issued
	 */
	Reporter samples_reporter{encoder_left_ptr, encoder_right_ptr};
	Task samples_streamer{1000, &samples_reporter};
	samples_streamer.suspend();

	while (1)
	{
		cli_task();
		heart_beat_task();
		samples_streamer();
	}
}
void heart_beat()
{
	printf("Heart beat \n");
}
void encoder_samples() 
{
	Handle h = tx_pool::allocate();
	tojson_encoder_samples(motion_controller, h);
	transport::send_json_response(&h);
}
void do_commands()
{
    treader.run();
    if (treader.available()) {
        Handle bh = treader.borrow_buffer();
        Argv args{};
        if (!args.tokenize(sb_buffer_as_cstr(bh))) {
            printf("Tokenize failed buffer: %s\n", sb_buffer_as_cstr(bh));
        } else {
            FTRACE("This is what treader got [%s]\n", sb_buffer_as_cstr(bh));
            FDUMP_TOKENS(args, "Dump tokens message")
            CommandName enumname = command_lookup(args.token_at(0));
            FTRACE("command_lookup result: [%s]", to_string(enumname))
            local_execute_commands(args, bh);
        }
		treader.return_buffer(bh);
    }
}
static void local_execute_commands(Argv& args, transport::buffer::Handle bh)
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
                robot_set_raw_pwm_percent(motion_controller, left_pwm, right_pwm);
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
                if(robot_set_rpm(motion_controller, left_rpm, right_rpm)) {
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
                robot_stop_all(motion_controller);
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
                tojson_encoder_samples(motion_controller, h);
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
                    sleep_ms(interval_ms);
                }
                uint64_t  end_time = micros();
                transport::send_command_ok("Elapsed time %llu micro seconds", (end_time - start_time));
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::SoftwareReset:
            *((volatile uint32_t*)(PPB_BASE + 0x0ED0C)) = 0x5FA0004;
            break;
        case CommandName::Help:
            printf("Commands: \n");
            printf("    w/pwm left      right       Set pwm percentage for each motors, values in range -100 .. 100\n");
            printf("    r/rpm left_rpm  right_rpm   Set speed of each motor in revs per minute\n");
            printf("    s                           Stop both motors \n");
            printf("    e                           Read both encoders\n");
            printf("    c                           Echo what ever follows the 'c'\n");
            printf("    b                           Software Reset\n");
            printf("    ?                           Help - print this message\n");
        
        default: {
            FTRACE("Execute command case default\n");
            transport::send_command_error("Unknowncommand");
            break;
        }

    }

    #endif
}
