#undef TRACE_ON
#if defined(PLATFORM_IS_LINUX)
    #include <stdio.h>
#else
    #include "pico/stdlib.h"
    #include <pico/error.h>

#endif
#include "trace.h"
#include "transmit_buffer_pool.h"
#include "transport.h"
#if defined(PLATFORM_IS_LINUX)
    #pragma message("transport platform is linux")
#endif
#define STX "\x02"
#define ETX "\x03"
#define COMMAND_RESPONSE_CHANNEL     '1'
#define UNSOLICITED_MESSAGE_CHANNEL  '2'

#define COMMAND_OK_RESPONSE_TYPE     'P'
#define COMMAND_ERROR_RESPONSE_TYPE  'P'
#define COMMAND_JSON_RESPONSE_TYPE   'J'

#define TRANSPORT_START_SENTINEL "/**AA**/"
#define TRANSPORT_END_SENTINEL "/**BB**/"

using namespace transport::buffer;


void transport::transport_init() 
{
    transport::buffer::rx_pool::init();
    transport::buffer::tx_pool::init();
}
static void transport_putchar(char ch) 
{
    #ifdef PLATFORM_IS_LINUX
        putchar(ch);
    #else
        putchar(ch);
    #endif

}
static void transport_puts(const char* astring) 
{
    for(int i = 0; astring[i] != (char)0; i++) {
        #ifdef PLATFORM_IS_LINUX
            putchar(astring[i]);
        #else
            putchar_raw(astring[i]);
        #endif
    }
    // #ifdef PLATFORM_IS_LINUX
    //     puts(astring);
    // #else
    //     stdio_put_string(astring);
    // #endif
}
static void transport_send(transport::buffer::Handle tbp, char channel,  char type) 
{
    #if defined(TRANSPORT_MODE_PACKET)
    transport_puts(STX);
    #elif defined(TRANSPORT_MODE_LINE)
    #else
         #error "invalid choice for TRANSPORT_MODE_????"
    #endif
    char bb[3] = {channel, type, (char)0};
    transport_putchar(channel);
    transport_putchar(type);
    transport_puts(transport::buffer::sb_buffer_as_cstr(tbp));
    #ifdef TRANSPORT_MODE_PACKET
    transport_puts(ETX);
    #else
    transport_puts("\n");
    #endif
}

void transport::send_command_ok(const char* fmt, ...)
{
    transport::buffer::Handle h = tx_pool::allocate();
    transport::buffer::sb_sprintf(h, "OK ");
    va_list(args);
    va_start(args, fmt);
    transport::buffer::sb_vsprintf(h, fmt, args);
    va_end(args);
    transport_send(h, '1', 'P');
    transport::buffer::Header* hp = (transport::buffer::Header*)h;
    tx_pool::deallocate(h);
}
void transport::send_command_error(const char* fmt, ...)
{
    transport::buffer::Handle h = tx_pool::allocate();
    transport::buffer::sb_sprintf(h, "ERROR ");
    va_list(args);
    va_start(args, fmt);
    transport::buffer::sb_vsprintf(h, fmt, args);
    va_end(args);
    transport_send(h, '1', 'P');
    tx_pool::deallocate(h);
}
void transport::send_json_response(transport::buffer::Handle* hp)
{
    transport::buffer::Handle h = *(hp);
    #ifndef LOCAL_TEST
    ASSERT(hp != nullptr)
    ASSERT(h != nullptr)
    #endif
    transport_send(h, '1', 'J');
    tx_pool::deallocate(h);
    *hp = nullptr;
}

