#ifndef H_transmit_buffer_pool_h
#define H_transmit_buffer_pool_h
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "buffers.h"



namespace transport::buffer::tx_pool {

    void init();
    transport::buffer::Handle allocate();
    void deallocate(transport::buffer::Handle h);


} // namespace transport::buffer::tx_pool


namespace transport::buffer::rx_pool {

void init();
transport::buffer::Handle allocate();
void                  deallocate(transport::buffer::Handle h);


} // namespace transport::buffer::rx_pool




#endif