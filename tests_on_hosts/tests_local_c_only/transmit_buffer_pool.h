#ifndef H_transmit_buffer_pool_h
#define H_transmit_buffer_pool_h
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "static_buffers.h"

namespace TransmitBufferPool {

transport::buffer::Handle allocate();
void                  deallocate(transport::buffer::Handle h);


} // namespace
#endif