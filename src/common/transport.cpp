#undef TRACE_ON
#include "trace.h"
#include <transport.h>
#define TRANSPORT_START_SENTINEL "/**AA**/"
#define TRANSPORT_END_SENTINEL "/**BB**/"

/**
 * Send a buffer of given length to the host via the stdio serial interface
*/
int transport_send(char* buffer, int len)
{
    int sentinel_len = strlen(TRANSPORT_START_SENTINEL)+strlen(TRANSPORT_END_SENTINEL);
    // int out_buffer_available_space = tud_cdc_write_available();
    FTRACE("Transport::send len: %d sentinels: %d\n", len, sentinel_len);
    long startTtime = micros();
    puts_raw(TRANSPORT_START_SENTINEL);
    puts_raw(buffer);
    puts_raw(TRANSPORT_END_SENTINEL);
    long endtime = micros();

    FTRACE("transport::send buffer of len: %d took %ld micro secs", len + sentinel_len, endtime - startTtime);
    return len;
}
