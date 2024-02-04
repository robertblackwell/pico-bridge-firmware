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
//#define LOCAL_TEST
#include "static_buffers.h"
#include <encoder_sample.h>
#include <encoder.h>
#include "transport.h"
//#include <transport.h>
//#include <robot.h>

StaticBuffers::Buffer<512> abuf;
StaticBuffers::Pool<3, 512> apool;

using namespace StaticBuffers; 

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wshadow"
int test_01() {
    auto p = &apool;
    Handle tbh = apool.allocate();
    // int x = 3;
    // tb_sprintf(tbh, "This is a message %d %f\n", 33, 44.0);
    // UT_EQUAL_INT(x, 3);
    return 0;
}
#if 1
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
    
    Handle tbh = apool.allocate();

    tojson_two_encoder_samples(tbh, sleft, sright);
    // transport_send_command_error("");
    // transport_send_command_ok("");
    // transport_send_json_response(&tbp);
    auto hp = (StaticBuffers::Header*)tbh;
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
int test_transport_send()
{
    transport_send_command_ok("This is from echo %s", "thisisthevalue");
    return 0;
}
int test_command_fill_echo()
{
    char buffer[100];
    strcpy(buffer, " echo 111 222 3 4 5 666");
    Argv args;
    args.tokenize(buffer);
    CommandBuffer cb{args};
    printf("   ");
    UT_EQUAL_INT(((int)CliCommands::Echo), (int)cb.identity());
    UT_EQUAL_CSTR(cb.echo_command.line, buffer);
    return 0;
}
int test_command_fill_error()
{
    const char* input = " xxecho 111 222 3 4 5 666";
    const char* expected = "ERROR[Unknown command] --  xxecho 111 222 3.000 4 5 666";
    char buffer[100];
    strcpy(buffer, " xxecho 111 222 3.000 4 5 666");
    Argv args;
    args.tokenize(buffer);
    {
        int one;
        bool b1 = argparse_posint(args, 1, one);
        UT_TRUE(b1)
        UT_EQUAL_INT(one, 111)
    }
    {
        int one;
        bool b1 = argparse_posint(args, 2, one);
        UT_TRUE(b1)
        UT_EQUAL_INT(one, 222)
    }
    {
        double v;
        bool b1 = argparse_double(args, 3, v);
        UT_TRUE(b1)
        UT_EQUAL_DOUBLE(v, 3.00)
    }
    CommandBuffer cb{args};
    printf("   ");
    UT_EQUAL_INT(((int)CliCommands::Error), (int)cb.identity());
    UT_EQUAL_CSTR(cb.error_command.what(), expected);
    return 0;
}
// int test_03() {
//     TransportReader tpreader{};
//     while(1) {
//         tpreader.run();
//         if(tpreader.available()) {
//             printf("Hello");
//         } 
//     }
// }
#endif
#pragma clang diagnostic pop

int main()
{
//    trace_init_stdio();
    printf("Hello world\n");
    // UT_ADD(test_01);
    // UT_ADD(test_02);
    UT_ADD(test_command_fill_error);
//    UT_ADD(test_command_fill_echo);
//    UT_ADD(test_transport_send);
    int rc = UT_RUN();
}