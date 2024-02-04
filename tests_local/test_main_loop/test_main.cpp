//
//  main.m
//  test_buffers
//
//  Created by ROBERT BLACKWELL on 11/26/17.
//  Copyright © 2017 Blackwellapps. All rights reserved.
//
#include <cstdio>
#include <cstdlib>
#include <unittest.h>
#define LOCAL_TEST
#include "cli/argv.h"
#include "cli/commands.h"
#include "transport/static_buffers.h"
#include <encoder_sample.h>
#include <encoder.h>
#include "transport/transport.h"
#include "transport/transmit_buffer_pool.h"
#include <robot.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
int test_01() {
    void* p;
    StaticBuffers::Handle bh = TransmitBufferPool::allocate();
    int x = 3;
    StaticBuffers::sb_sprintf(bh, "This is a message %d %f\n", 33, 44.0);
    UT_EQUAL_INT(x, 3);
    return 0;
}
int test_02() {
    EncoderSample sleft{};
    sleft.s_contains_data = true;
    sleft.s_pin_state = "F";
    sleft.s_name = "left";
    sleft.s_encoder_addr = nullptr;
    sleft.s_motor_rpm = 5600.22;
    sleft.s_wheel_rpm = (sleft.s_motor_rpm / 229.0);
    sleft.s_speed_mm_per_second = (sleft.s_wheel_rpm / 60.0) * (22.0/7.0) * 70.0;
    sleft.s_musecs_per_interrupt = ((sleft.s_motor_rpm / 60.0) / 24.0);
    EncoderSample sright{};
    sright.s_contains_data = true;
    sright.s_pin_state = "B";
    sright.s_name = "right";
    sright.s_encoder_addr = nullptr;
    sright.s_motor_rpm = 5638.88;
    sright.s_wheel_rpm = (sleft.s_motor_rpm / 229.0);
    sright.s_speed_mm_per_second = (sleft.s_wheel_rpm / 60.0) * (22.0/7.0) * 70.0;
    sright.s_musecs_per_interrupt = ((sleft.s_motor_rpm / 60.0) / 24.0);
    
    StaticBuffers::Handle tbp = TransmitBufferPool::allocate();

    tojson_two_encoder_samples(tbp, sleft, sright);
    transport_send_command_error("");
    transport_send_command_ok("");
    transport_send_json_response(&tbp);

    return 0;
}

const char* input[] = {"aaaa bb", "bbb ccc", "c dddd \n"};
const char* saved_p = (input[0]);
bool get_char_if_available(int* char_received) {
    if(*saved_p != (char)0) {
        char ch = *saved_p;
        saved_p++;
        *char_received = (int)ch;
        return true;
    } else {
        saved_p++;
        return false;
    }
}

int test_03() {
    const char* test_input[] = {"aaaa b", "bbbb ", "c12ccc d", "ddd \n"};
    saved_p = (test_input[0]);
    TransportReader tpreader{};
    while(1) {
        tpreader.run();
        if(tpreader.available()) {
            StaticBuffers::Handle bh = tpreader.borrow_buffer();
            Argv args{StaticBuffers::sb_buffer_as_cstr(bh)};
            printf("Hello");
            UT_EQUAL_INT(args.token_count, 4);
            UT_EQUAL_CSTR(args.token_at(0), "aaaa");
            UT_EQUAL_CSTR(args.token_at(1), "bbbbb");
            UT_EQUAL_CSTR(args.token_at(2), "c12ccc");
            UT_EQUAL_CSTR(args.token_at(3), "dddd");
            tpreader.return_buffer(bh);
            return 0;
        } 
    }
}
int test_04() {
    const char* test_input[] = {"aaaa 1", "2345 ", "-128 12", ".0786 ", "-98.7654", " \n"};
    saved_p = (test_input[0]);
    TransportReader tpreader{};
    while(1) {
        tpreader.run();
        if(tpreader.available()) {
            StaticBuffers::Handle bh = tpreader.borrow_buffer();
            Argv args{StaticBuffers::sb_buffer_as_cstr(bh)};
            printf("Hello");
            UT_EQUAL_INT(args.token_count, 5)
            UT_EQUAL_CSTR(args.token_at(0), "aaaa")
            UT_EQUAL_CSTR(args.token_at(1), "12345")
            {
                int i = 0;
                bool success = argparse_posint(args, 1, i);
                UT_TRUE(success)
                UT_EQUAL_INT(i, 12345)
            }
            UT_EQUAL_CSTR(args.token_at(2), "-128")
            {
                int i = 0;
                bool success = argparse_negint(args, 2, i);
                UT_TRUE(success)
                UT_EQUAL_INT(i, -128)
            }
            UT_EQUAL_CSTR(args.token_at(3), "12.0786")
            {
                double d = 0.0;
                bool success = argparse_double(args, 3, d);
                UT_TRUE(success)
                UT_EQUAL_DOUBLE(d, 12.0786)
            }
            UT_EQUAL_CSTR(args.token_at(4), "-98.7654")
            {
                double d = 0.0;
                bool success = argparse_double(args, 4, d);
                UT_TRUE(success)
                UT_EQUAL_DOUBLE(d, -98.7654)
            }
            tpreader.return_buffer(bh);
            return 0;
        }
    }
}
void one_main_loop(const char* input)
{
    saved_p = input;

    TransportReader tpreader{};
    while(1) {
        tpreader.run();
        if(tpreader.available()) {
            StaticBuffers::Handle bh = tpreader.borrow_buffer();
            Argv args{StaticBuffers::sb_buffer_as_cstr(bh)};
            CommandName name = command_lookup(args.token_at(0));
            printf("Hello name: %s \n", to_string(name));
            switch(name) {
                case CommandName::MotorsPwmPercent: {
                    double left_pwm, right_pwm;
                    if(validate_pwm(args, left_pwm, right_pwm)) {
                        printf("%f %f\n", left_pwm, right_pwm);
                        
                    } else {
                        printf("Invalid %s command %s\n", to_string(name), StaticBuffers::sb_buffer_as_cstr(bh));
                    }
                }
                break;
                case CommandName::MotorsRpm: {
                    double left_rpm, right_rpm;
                    if(validate_rpm(args, left_rpm, right_rpm)) {
                        printf("%f %f\n", left_rpm, right_rpm);
                    } else {
                        printf("Invalid %s command %s\n", to_string(name), StaticBuffers::sb_buffer_as_cstr(bh));
                    }

                }
                break;
                case CommandName::EncodersRead: {
                    if(validate_encoder_read(args)) {
                        printf("%s\n", to_string(name));
                    } else {
                        printf("Invalid %s command %s\n", to_string(name), StaticBuffers::sb_buffer_as_cstr(bh));
                    }

                }
                break;
                case CommandName::MotorsHalt: {
                    if(validate_encoder_read(args)) {
                        printf("%s\n", to_string(name));
                    } else {
                        printf("Invalid %s command %s\n", to_string(name), StaticBuffers::sb_buffer_as_cstr(bh));
                    }

                }
                    break;
                case CommandName::Echo: {
                    if(validate_encoder_read(args)) {
                        printf("%s\n", to_string(name));
                    } else {
                        printf("Invalid %s command %s\n", to_string(name), StaticBuffers::sb_buffer_as_cstr(bh));
                    }

                }
                break;
                case CommandName::Error: {

                }
                break;
                case CommandName::PidArgsUpdate:
                case CommandName::None:
                {

                }
                break;
            }
            tpreader.return_buffer(bh);
            break;
        }
    }
}
int test_05() {
    const char* test_input = 
        "pwm 80.0\0" " 87 \n"  
        "pwm 80.0\0" " 187 \n"
        "rpm 3400.0 \0" " 3600.0 \n"
        "stop \0" "\n"
        "echo this \0" "is some stuff rpm 3400.0 3600.0 \n";
    const char* t = test_input;

    one_main_loop("pwm 80.0\0" " 87 \n");
    one_main_loop("pwm 80.0\0" " 187 \n");
    one_main_loop("rpm 3400.0 \0" " 3600.0 \n");
    one_main_loop("stop \0" "\n");
    one_main_loop("echo this \0" "is some stuff rpm 3400.0 3600.0 \n");

    return 0;
}

#pragma clang diagnostic pop

int main()
{
    trace_init_stdio();
    printf("Hello world\n");
    // UT_ADD(test_01);
    UT_ADD(test_05);
    int rc = UT_RUN(); 
}