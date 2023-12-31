#ifndef H_trace_h
#define H_trace_h
#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>

/**
 * This module provides macros and function for outputting trace/debug and telemetry
 * information while a microcontroller program is running.
 * 
 * The code is specific to a raspberrypi pico/pico_w
 * 
 * The trace module can output to stdio (either usb or default uart depending on cmake settings),
 * or to the uart that is not used by the stdio system.
 * 
 * The choice of output port is controlled by the compile time symbol 
 * 
 *      IRACOON_TRACE_NONSTDIO_UART
 * 
 * If defined this symbol instructs this module to use the uart not used by stdio.
 * 
 * If undefined this module should use stdio for trace output.
 * 
 * This module must be initialized by a call to trace_init()
*/
#ifdef __cplusplus
extern "C" 
{
#endif

#if defined IRACOON_TRACE_NONSTDIO_UART
    #if PICO_DEFAULT_UART == 0
        // #warning "Trace non stdio uart = uart1"
        #define IRACOON_TRACE_UART uart1
        #define IRACOON_TRACE_UART_NBR 1
    #else
        // #warning "Trace non stdio uart == uart0"
        #define IRACOON_TRACE_UART uart0
        #define IRACOON_TRACE_UART_NBR 0
    #endif
    #define IRACOON_TRACE_PUTS(s) uart_puts(IRACOON_TRACE_UART, (s));
    #if !defined IRACOON_TRACE_BAUDRATE
        #define IRACOON_TRACE_BAUDRATE 115200
    #endif
    #if !defined IRACOON_TRACE_RX_PIN
        #define IRACOON_TRACE_RX_PIN 9
    #endif
    #if !defined IRACOON_TRACE_TX_PIN
        #define IRACOON_TRACE_TX_PIN 8
    #endif    
#else
    #define IRACOON_TRACE_PUTS(s) puts_raw((s));
#endif

void trace_init();
void trace_init_stdio();
void trace_init_full(uart_inst_t* uart, int baudrate, int rx_pin, int tx_pin);

void print_fmt(const char* fmt, ...);
void print_trace(const char* filename, int line_number, const char* func, const char* fmt, ...);

#define TPRINTF(fmt, ...) do {\
    print_fmt(fmt, ##__VA_ARGS__); \
} while(0);

#define isdigit(c) ((c >= '0') && (c <= '9'))

/**
 * Forgive the following bit of C++ trickery. If avr-g++ was c++14 we could do a little better.
 * These are compile-time functions that take the basename from a path
 * Compile-time is very attractive for Arduino as it does not toke
 * data or program memory
 */
#if 1
inline const char* str_end(const char *str) {
    return *str ? str_end(str + 1) : str;
}

inline bool str_slant(const char *str) {
    return *str == '/' ? true : (*str ? str_slant(str + 1) : false);
}

inline const char* r_slant(const char* str) {
    return *str == '/' ? (str + 1) : r_slant(str - 1);
}
inline const char* file_name(const char* str) {
    return str_slant(str) ? r_slant(str_end(str)) : str;
}
#endif
// gets the Least significant byte
#define mshed_lowByte(x) (uint8_t)(x & 0x00ff)
// gets the MostSignificantByte
#define mshed_highByte(x) (uint8_t)(x >> 8)

#ifdef FTRACE_ON
#define FTRACE(fmt, ...) do { \
    print_trace(file_name(__FILE__), __LINE__, __FUNCTION__, fmt, ##__VA_ARGS__); \
} while(0);
#else
#define FTRACE(fmt, ...)
#endif

#ifdef FTRACE_ON
#define FTRACE_PRINT(fmt, ...) do { \
    print_fmt(fmt, ##__VA_ARGS__);  \
} while(0);
#else
#define FTRACE_PRINT(fmt, ...)
#endif


#ifdef FDEBUG_ON
#define FDEBUG(fmt, ...) do { \
    print_fmt("TRACE %s:[%d]:[%s] ", file_name(__FILE__), __LINE__, __FUNCTION__); \
    print_fmt(fmt, ##__VA_ARGS__);  \
} while(0);
#else
#define FDEBUG(fmt, ...)
#endif

#ifdef FDEBUG_ON
#define FDEBUG_PRINT(fmt, ...) do { \
    print_fmt(fmt, ##__VA_ARGS__);  \
} while(0);
#else
#define FDEBUG_PRINT(fmt, ...)
#endif

#ifdef FDEBUG_ON
#define FDUMP_TOKENS(token, msg) do { \
    (token).dump(msg);  \
} while(0);
#else
#define FDUMP_TOKENS(token, msg)
#endif

#ifndef ARDUINO
inline void delay(long x) {sleep_ms(x);}
#endif

#define FATAL_ERROR() do { \
    print_fmt("Error in file:%s at line:%d \n", file_name(__FILE__), __LINE__); \
    delay(5000); \
} while(1);

#define FATAL_ERROR_MSG(msg) do { \
    print_fmt("Error in file:%s at line:%d msg: %s\n", file_name(__FILE__), __LINE__, msg);                                  \
    delay(5000); \
} while(1);

#define FATAL_ERROR_PRINTF(fmt, ...) do {  \
    print_fmt("Error in file:%s at line:%d" fmt "\n",  file_name(__FILE__), __LINE__,  __VA_ARGS__); \
    delay(5000); \
} while(1);

#define ASSERT(condition) do { \
    if(!(condition)) {         \
        FATAL_ERROR()\
    }                               \
} while(0);
#define ASSERT_MSG(condition, msg) do { \
    if(!(condition)) {              \
        FATAL_ERROR_MSG(msg)              \
    }\
} while(0);
#define ASSERT_PRINTF(condition, fmt, ...) do { \
    if(!(condition)) {              \
        FATAL_ERROR_PRINTF(fmt, __VA_ARGS__);   \
    }  \
} while(0);


/**
 * Utility functions for Arduino
 */

void print_fmt(const char* fmt, ...);
void trace_hexdump_uint16(const char* msg, uint16_t v);
void trace_hexdump_uint32(const char* msg, uint32_t v);
void trace_hexdump(const char* m, uint8_t* buf, uint16_t len );
// char* bits2string(uint8_t bits);
// char* bits2string(uint16_t bits);

void printFloat(float value, int places);

char* floa2str(float f);

#ifdef __cplusplus
}
#endif


#endif
