#ifndef H_transmit_buffer_pool_h
#define H_transmit_buffer_pool_h
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "buffers.h"

namespace transport {
namespace buffer {    
namespace tx_pool {

void init();
transport::buffer::Handle allocate();
void deallocate(transport::buffer::Handle h);


} // namespace tx_pool

namespace rx_pool {

void init();
transport::buffer::Handle allocate();
void                  deallocate(transport::buffer::Handle h);


} // namespace rx_pool
} //namespace buffer
} //namespace transport


#endif