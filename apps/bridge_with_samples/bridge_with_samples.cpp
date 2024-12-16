#undef FTRACE_ON
#include <functional>
#include <stdio.h>
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

#define MODE_REPORT 1
#define MODE_MOTION 2
#define MODE_REMOTE_PID 3
#define MODE_EXERCISE 4
#define MODE_COMMANDS_ONLY 5

#include "pico/stdlib.h"
#include <pico/error.h>
#include <cli/execute_commands.h>
#include <version.h>

using namespace transport::buffer;

void do_commands();
void heart_beat();
void report_samples();
static void local_execute_commands(Argv& args, transport::buffer::Handle bh);

static bool test_get_char_if_available(int* char_received) {
	int ch = getchar_timeout_us(0);
	if(ch == PICO_ERROR_TIMEOUT) {
		// printf("get_char_if_available: no char timedout\n");
		return false;
	}
	*char_received = ch;
	// printf("get_char_if_available: got ch: %c decimal value of ch %d\n", (char)ch, ch);
	return true;
}

MotionControl* motion_control_ptr;
Encoder*       left_encoder_ptr;
Encoder*       right_encoder_ptr;
Task*          sample_streamer_task_ptr;

transport::Reader treader;
int main()
{
	transport::transport_init();
	treader.begin();

	stdio_init_all();
	stdio_set_translate_crlf(&stdio_usb, false);
	trace_init();
	sleep_ms(5000);
//	print_fmt("bridge (version:%s ) starting ... \n", VERSION_NUMBER);
    transport::send_boot_message("bridge_with_samples (version:%s ) " PROG_NAME  "starting ... \n", VERSION_NUMBER);
	
	robot_init();
	motion_control_ptr = get_motion_controller_ptr();
	left_encoder_ptr = get_encoder(DriveSide::left);
	right_encoder_ptr = get_encoder(DriveSide::right);

	Task cli_task(20, do_commands);
	Task heart_beat_task(2000, heart_beat);
    Task sample_streamer_task(1000, report_samples);
    sample_streamer_task.suspend();
    sample_streamer_task_ptr = &sample_streamer_task;
	// Task collect_samples_task(200, &robot_collect_encoder_samples);
	// robot_start_encoder_sample_collection((uint64_t)10000);
	while (1)
	{
		cli_task();
		heart_beat_task();
		sample_streamer_task();
	}
}
void heart_beat()
{
	printf("Heart beat \n");
}
void report_samples()
{
    Handle h = tx_pool::allocate();
    tojson_encoder_samples(h);
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
            printf("This is what treader got [%s]\n", sb_buffer_as_cstr(bh));
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
                robot_set_raw_pwm_percent(left_pwm, right_pwm);
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
                if(robot_set_rpm(left_rpm, right_rpm)) {
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
                robot_stop_all();
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
                tojson_encoder_samples(h);
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
                printf("Encoder stream  %s  interval_ms: %f\n", to_string(enumname), interval_ms);
                sample_streamer_task_ptr->update_interval(interval_ms);
                sample_streamer_task_ptr->start();
                // Handle h = tx_pool::allocate();
                // tojson_encoder_samples(h);
                // transport::send_json_response(&h);
                // printf("End of read encoder cmd \n");
            } else {
                transport::send_command_error("Invalid %s command %s\n", to_string(enumname), sb_buffer_as_cstr(bh));
            }
            break;
        }
        case CommandName::EncodersStreamStop:
        {
            int number = 1;
            if(validate_encoders_stream_stop(args)) {
                printf("EncoderStream Stop%s\n", to_string(enumname));
                sample_streamer_task_ptr->suspend();
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
