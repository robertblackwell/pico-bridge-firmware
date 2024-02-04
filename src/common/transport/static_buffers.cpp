#include "static_buffers.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif


size_t StaticBuffers::sb_sprintf(StaticBuffers::Handle tbh, const char* fmt, ...) {
    auto* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    va_list vlist;
    va_start(vlist, fmt);
    char* x = va_arg(vlist, char* );
    size_t size = vsnprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, vlist);
    tbph->m_used_length += size;
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
    *(tbph->m_next_available_p) = (char)0;
    va_end(vlist);
    return size;
}
size_t StaticBuffers::sb_vsprintf(StaticBuffers::Handle tbh, const char* fmt, va_list vlist) {
    auto* tbph = (StaticBuffers::Header*)tbh;
#ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
#endif
//    va_list vlist;
//    va_start(vlist, fmt);
//    char* x = va_arg(vlist, char* );
    size_t size = vsnprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, vlist);
    tbph->m_used_length += size;
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
    *(tbph->m_next_available_p) = (char)0;
//    va_end(vlist);
    return size;
}

size_t StaticBuffers::sb_buffer_length(StaticBuffers::Handle tbh) {
    auto* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_used_length;
}
char* StaticBuffers::sb_buffer_as_cstr(StaticBuffers::Handle tbh) {
    auto* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_buffer_as_str;
}
void StaticBuffers::sb_append(StaticBuffers::Handle tbh, char ch) {
    auto* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr)
    ASSERT(tbph->m_used_length + 1 < tbph->m_capacity)
    #endif
    *(tbph->m_next_available_p) = ch;
    tbph->m_next_available_p++;
    tbph->m_used_length++;
    // keep the buffer a valid cstr
    *(tbph->m_next_available_p) = '\0';
}
void StaticBuffers::sb_reset(StaticBuffers::Handle tbh)
{
    auto* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr)
    #endif
    tbph->reset();
}
