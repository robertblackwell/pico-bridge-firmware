#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "trace.h"

typedef void (trace_puts_t)(const char*);

trace_puts_t* trace_puts_func_ptr;
uart_inst_t* trace_uart_ptr;
int trace_baudrate;
int trace_rx_pin;
int trace_tx_pin;
bool trace_add_cr = false;

void trace_uart_puts(const char* s)
{
    // printf("trace_uart_puts trace_uart_ptr %p  u0: %p u1: %p\n ", trace_uart_ptr, uart0, uart1);
    uart_puts(trace_uart_ptr, s);
    if(trace_add_cr) {
        uart_puts(trace_uart_ptr, "\r");
    }
}
void trace_stdio_puts(const char* s)
{
    puts_raw(s);
}
void trace_init()
{
    #ifdef IRACOON_TRACE_NONSTDIO_UART
        TPRINTF("trace_uart: %d tx_pin: %d rx_pin %d\n", IRACOON_TRACE_UART_NBR, IRACOON_TRACE_TX_PIN, IRACOON_TRACE_RX_PIN);
        trace_init_full(IRACOON_TRACE_UART, IRACOON_TRACE_BAUDRATE, IRACOON_TRACE_RX_PIN, IRACOON_TRACE_TX_PIN);
    #else
        trace_init_stdio();
        // using stdio for tracing - rely on caller to do some form of stdio_init_all()
    #endif
}
void trace_init_stdio()
{
    trace_puts_func_ptr = &trace_stdio_puts;
}
void trace_init_full(uart_inst_t* uart_ptr, int baudrate, int rx_pin, int tx_pin)
{
    // printf("trace_init_full usrt_ptr %p baudrate: %d rx_pin: %d tx_pin: %d\n", uart_ptr, baudrate, rx_pin, tx_pin);
    trace_uart_ptr = uart_ptr;
    trace_rx_pin = rx_pin;
    trace_tx_pin = tx_pin;
    trace_baudrate = baudrate;

    uart_init(trace_uart_ptr, baudrate);
    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);

    trace_puts_func_ptr = trace_uart_puts;
    trace_add_cr = false;
}

#define PRINT_FMT_BUFLEN 512
static char str_buf[PRINT_FMT_BUFLEN];

void print_trace(const char* filename, int line_number, const char* func, const char* fmt, ...)
{
    va_list(args);
    va_start(args, fmt);
    int len = 0;
    len += snprintf(str_buf, PRINT_FMT_BUFLEN, "TRACE %s:[%d]:[%s] ", filename, line_number, func); \
    len += vsnprintf(&str_buf[len], PRINT_FMT_BUFLEN - len, fmt, args);
    trace_puts_func_ptr(str_buf);
    va_end(args);
}

void print_fmt(const char* fmt, ...) {
    // printf("print_fmt called trace_puts_func_ptr %p \n", trace_puts_func_ptr);
    va_list(args);
    va_start(args, fmt);
    int len = vsnprintf(str_buf, PRINT_FMT_BUFLEN, fmt, args);
    ASSERT_MSG((len < PRINT_FMT_BUFLEN), "print_fmt buffer overflow\n");
    trace_puts_func_ptr(str_buf);
    
    // printf("%.*s", len, str_buf);
    va_end(args);
}
void trace_hexdump(const char* m, uint8_t* buf, uint16_t len ) {

    print_fmt("%s addr: %p length : %d  0x", m, buf, len);
    for(uint16_t i = 0; i < len; i++) {
        print_fmt("%2.2X", (uint8_t)buf[i]);
    }
    print_fmt("\n");
}

void trace_hexdump_uint16(const char* msg, uint16_t v) {
    uint8_t  lb = mshed_lowByte(v);
    uint8_t  hb = mshed_highByte(v);
    print_fmt("%s as uint16_t 0x%4.4X lowbyte 0x%2.2X highbyte 0x%2.2X \n", msg, v, lb, hb);
}
void trace_hexdump_uint32(const char* msg, uint32_t v) {
    print_fmt("%s as uint32_t 0x%8.8X  \n", msg, v);
    trace_hexdump("as memory ", (uint8_t*)&v, 4);
}

char* bits2string(uint8_t bits) {
    static char buf[20];
    char* p = buf;
    strcpy(p, "0b");
    int count = 0;
    uint8_t  mask = 0b10000000;
    for (mask = 0b1000000; mask > 0; mask >>= 1) {
        // auto x = ((bits & mask) == mask) ? "1" : "0";
        if(count % 4 == 0) {
            strcat(buf, " ");
        }
        strcat(buf, ((bits & mask) != 0) ? "1" : "0");
        count++;
    }
    return buf;
}
char* bits2string(uint16_t bits) {
    static char buf[40];
    char* p = buf;
    strcpy(p, "0b");
    int count = 0;
    uint16_t  mask = (unsigned short)0x8000; //0b10000000000000000;
    for (; mask > 0; mask >>= 1) {
        // auto x = ((bits & mask) == mask) ? "1" : "0";
        if(count % 4 == 0) {
            strcat(buf, " ");
        }
        strcat(buf, ((bits & mask) != 0) ? "1" : "0");
        count++;
    }
    return buf;
}
// printFloat prints out the float 'value' rounded to 'places' places after the decimal point

// void printFloat(float value, int places) {
//   // this is used to cast digits
//     int digit;
//     float tens = 0.1;
//     int tenscount = 0;
//     int i;
//     float tempfloat = value;

//     // make sure we round properly. this could use pow from <math.h>, but doesn't seem worth the import
//     // if this rounding step isn't here, the value  54.321 prints as 54.3209

//     // calculate rounding term d:   0.5/pow(10,places)  
//     float d = 0.5;
//     if (value < 0)
//         d *= -1.0;
//         // divide by ten for each decimal place
//     for (i = 0; i < places; i++)
//         d/= 10.0;    
//     // this small addition, combined with truncation will round our values properly
//     tempfloat +=  d;

//     // first get value tens to be the large power of ten less than value
//     // tenscount isn't necessary but it would be useful if you wanted to know after this how many chars the number will take

//     if (value < 0)
//         tempfloat *= -1.0;
//     while ((tens * 10.0) <= tempfloat) {
//         tens *= 10.0;
//         tenscount += 1;
//     }


//     // write out the negative if needed
//     if (value < 0)
//         Serial.print('-');

//     if (tenscount == 0)
//         Serial.print(0, DEC);

//     for (i=0; i< tenscount; i++) {
//         digit = (int) (tempfloat/tens);
//         Serial.print(digit, DEC);
//         tempfloat = tempfloat - ((float)digit * tens);
//         tens /= 10.0;
//     }

//     // if no places after decimal, stop now and return
//     if (places <= 0)
//         return;

//     // otherwise, write the point and continue on
//     Serial.print('.');  

//     // now write out each decimal place by shifting digits one by one into the ones place and writing the truncated value
//     for (i = 0; i < places; i++) {
//         tempfloat *= 10.0;
//         digit = (int) tempfloat;
//         Serial.print(digit,DEC);  
//         // once written, subtract off that digit
//         tempfloat = tempfloat - (float) digit;
//     }
// }
char* floa2str(float f)
{
    static char buffer[40];
    // char* p = buffer;
    int nbefore = (long)(f);
    // int after2 = ((long)(f * 100.0) % 100);
    sprintf(buffer, "%d", nbefore);
    return buffer;
}
