
#include "transmit_buffer_pool.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

using namespace transport::buffer;

transport::buffer::Pool<3, 512> transmit_pool;

Handle TransmitBufferPool::allocate() {
    return transmit_pool.allocate();
}
void TransmitBufferPool::deallocate(transport::buffer::Handle h) {
    transmit_pool.deallocate(h);
}

// size_t tb_sprintf(TBuffer* tbp, const char* fmt, ...) {
//     #ifndef LOCAL_TEST
//     ASSERT(tbp != nullptr);
//     #endif
//     va_list(args);
//     va_start(args, fmt);
//     size_t size = vsnprintf(tbp->m_next_available_p, tbp->m_capacity - tbp->m_used_length, fmt, args);
//     tbp->m_used_length += size;
//     tbp->m_next_available_p = &(tbp->m_buffer[tbp->m_used_length]);
//     *(tbp->m_next_available_p) = (char)0;
//     va_end(args);
//     return size;
// }
// size_t tb_buffer_length(TBuffer* tbp) {
//     #ifndef LOCAL_TEST
//     ASSERT(tbp != nullptr);
//     #endif
//     return tbp->m_used_length;
// }
// char* tb_buffer_as_cstr(TBuffer* tbp) {
//     #ifndef LOCAL_TEST
//     ASSERT(tbp != nullptr);
//     #endif
//     return tbp->m_buffer_as_str;
// }
// void tb_append(TBuffer* tbp, char ch) {
//     #ifndef LOCAL_TEST
//     ASSERT(tbp != nullptr)
//     ASSERT(tbp->m_used_length + 1 < tbp->m_capacity)
//     #endif
//     *(tbp->m_next_available_p) = ch;
//     tbp->m_next_available_p++;
//     tbp->m_used_length++;
// }