#ifndef H_transport_h
#define H_transport_h
#include <cstdio>
#include "buffers.h"
#include "cli/argv.h"
/**
 * When transmitting to a host the transport module has a choice of modes of operation.
 * 
 * ## PACKET_MODE
 * -    in packet mode each complete transmission is bracketted or framed by a starting STX
 *      character and ended by a closing ETX character
 * 
 * ## LINE_MODE
 * -    in line mode there is generally no specific starting chararcter but each transmission is ended
 *      by a line feed "\n" and there must be no line feed characters in the body of the transmission.
 * 
 * the choice of mode is controlled by the value of the compile variables TRANSPORT_MODE_PACKET, TRANSPORT_MODE_LINE
 * One of these and one only must be defined.
 * 
*/
namespace transport {
    
enum ContentType {
    OK_RESPONSE = 'P',
    ERROR_RESPONSE = 'P',
    JSON_RESONSE = 'J',
    BOOT_MESSAGE = 'B',
};

enum Channel {
    RPC_CHANNEL = '1',
    UNSOLICITED_CHANNEL = '2'
};

void transport_init();

struct Reader {

    Reader();

    /** begin must be called to allocate an internal buffer */
    void begin();
    ~Reader();
    void run();

    [[nodiscard]] bool available() const;

    // void consume(Argv& argv);

    // Gives you a buffer of input data - caller must deallocate
    // the buffer after use.
    transport::buffer::Handle consume();
    /** This function borrows the buffer from the transport module in order to parse the contents.
     * But will return the buffer after parsiing.
     */
    transport::buffer::Handle borrow_buffer();
    /**
     * Signals to transport that the consumer is done with the buffer
     * and will not use it again after this call. After the call the buffer handle is invalid.
     * @param bh
     */
    void  return_buffer(transport::buffer::Handle& bh);

    int                         m_state;
    transport::buffer::Handle   m_buffer_handle{};
    bool                        m_chars_available;
};

void send_command_ok(const char* fmt, ...);
void send_command_error(const char* fmt, ...);
void send_boot_message(const char* fmt, ...);
void send_json_response(transport::buffer::Handle* buffer_handle);

} // end namespace transport
#endif