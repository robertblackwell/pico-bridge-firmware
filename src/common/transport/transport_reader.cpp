#define FTRACE_ON
#if defined(PLATFORM_IS_LINUX)
    #include <stdio.h>
    bool get_char_if_available(int* char_received);

//static bool get_char_if_available(int* char_received) {
//    printf("we got here");
//    return true;
//}

#else
    #include "pico/stdlib.h"
    #include <pico/error.h>
    static bool get_char_if_available(int* char_received) {
        int ch = getchar_timeout_us(0);
        if(ch == PICO_ERROR_TIMEOUT) {
            return false;
        }
        *char_received = ch;
        return true;
    }

#endif
#include "trace.h"
#include "transmit_buffer_pool.h"
#include "transport.h"

#define TRANSPORT_STATE_START 1
#define TRANSPORT_STATE_WAITING_STX 1           // waiting for chararcter STX
#define TRANSPORT_STATE_READING_LINE 2
#define TRANSPORT_STATE_READING_MSG 2

#define TRANSPORT_STATE_MSG_AVAILABLE 3
#define TRANSPORT_STATE_LINE_AVAILABLE 3
// #define TRANSPORT_STATE_ARGS_AVAILABLE 4

#define TRANSPORT_STATE_NEED_ARGS 4
#if defined(PLATFORM_IS_LINUX)


#endif
char static tolower(char c) {
    return ((c >= 'A') && (c <= 'Z')) ? (char)(c - 'A' + 'a'): c;
}

namespace transport {

void Reader::begin()
{
    m_state = TRANSPORT_STATE_START;
    // m_next_pos = 0;
    m_chars_available = false;
    m_buffer_handle = transport::buffer::rx_pool::allocate();
}
Reader::~Reader()
{
    transport::buffer::rx_pool::deallocate(m_buffer_handle);
}
#if TRANSPORT_MODE_PACKET
/**
 * This version of Transport::run() reads and parses data that comes in as a delimited message.
 * Each message :
 * -    starts with a STX character
 * -    followed by a '1' or '2' the channel number
 * -    then the message body, all printable characters, no control characters.
 * -    the end of the message is sfignified by an ETX characater.
 * 
 * The STX, channel number and ETX are not present in the message buffer after reading is complete
 * and tokenization takes place.
*/

void Reader::run()
{
    #define STXCHAR ((char)02)
    #define ETXCHAR ((char)03)

    if(m_state == TRANSPORT_STATE_ARGS_AVAILABLE)
        return;
    // printf("cli::run test ch available\n");
    int chin;
    char ch;
    while(get_char_if_available(&chin)) {
        ch = chin;
        print_fmt("chin : %c %x\n", (char)ch, (int)chin);
        switch(m_state) {
            case TRANSPORT_STATE_WAITING_STX:
                if(ch == STXCHAR) {
                    FTRACE("STX found chaging state")
                    m_state = TRANSPORT_STATE_READING_MSG;
                } else {
                    print_fmt("STX not found %c\n", (char)chin);
                }
            break;
            case TRANSPORT_STATE_READING_MSG:
            if(ch == ETXCHAR) {
                m_buffer[m_next_pos] = (char)0;
                m_state = TRANSPORT_STATE_MSG_AVAILABLE;
                FTRACE("ETX found m_buffer: %s\n", m_buffer);
                // pull the channel number off the fron
                tokenize_line(&(m_buffer[1]), m_argv);
                FDUMP_TOKENS((this->m_argv), "this->m_argv");
                m_next_pos = 0;
                m_buffer[0] = (char)0;
                m_state = TRANSPORT_STATE_ARGS_AVAILABLE;
                return;
            } else {
                m_buffer[m_next_pos] = ch;
                m_next_pos++;
                ASSERT_MSG((m_next_pos < CLI_BUFFER_MAX), "input line too long in cli::run")
                m_buffer[m_next_pos] = (char)0;
                FTRACE("No ETX found m_buffer: %s\n", m_buffer);
            }
            break;
        }
    }
    return;
}
#else
/**
 * This version of Transport::run() reads and parses data that comes in as a line.
 * That is a string terminated by a '\n' character. The character '\r' is ignored. 
*/
void Reader::run()
{
    if(m_state == TRANSPORT_STATE_LINE_AVAILABLE)
        return;
    int chin;
    char ch;
    
    while(get_char_if_available(&chin)) {
        // printf("reader.run chin: %d\n", chin);
        m_state = TRANSPORT_STATE_READING_LINE;
        ch = tolower(chin);
        if((ch == '\n') || (transport::buffer::sb_space_remaining(m_buffer_handle) <= 2)) {
            m_state = TRANSPORT_STATE_LINE_AVAILABLE;
            return;
        }else if (ch == '\r') {
            ;
        } else {
            // printf("cli::run ch: %c\n", ch);
            transport::buffer::sb_append(m_buffer_handle, ch);
        }
    }
    return;
}
#endif
bool Reader::available()
{
    // printf("reader.available m_state: %d\n", m_state);
    return (m_state == TRANSPORT_STATE_LINE_AVAILABLE);
}
transport::buffer::Handle Reader::consume()
{
    ASSERT(m_state == TRANSPORT_STATE_LINE_AVAILABLE)
    transport::buffer::Handle tmp = m_buffer_handle;
    m_buffer_handle = transport::buffer::rx_pool::allocate();
    m_state = TRANSPORT_STATE_START;
    return tmp;
}
transport::buffer::Handle Reader::borrow_buffer()
{
    ASSERT(m_state == TRANSPORT_STATE_LINE_AVAILABLE)
    m_state = TRANSPORT_STATE_START;
    return m_buffer_handle;
}
void Reader::return_buffer(transport::buffer::Handle bh)
{
    ASSERT(bh == m_buffer_handle);
    transport::buffer::sb_reset(m_buffer_handle);
    m_state == TRANSPORT_STATE_START;
}

} // namespace transport