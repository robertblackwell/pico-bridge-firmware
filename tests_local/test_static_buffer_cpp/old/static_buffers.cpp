#include "static_buffers.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

size_t sb_sprintf(StaticBuffers::Handle tbh, const char* fmt, ...) {
    StaticBuffers::Header* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    va_list(args);
    va_start(args, fmt);
    size_t size = vsnprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, args);
    tbph->m_used_length += size;
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
    *(tbph->m_next_available_p) = (char)0;
    va_end(args);
    return size;
}
size_t sb_buffer_length(StaticBuffers::Handle tbh) {
    StaticBuffers::Header* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_used_length;
}
char* sb_buffer_as_cstr(StaticBuffers::Handle tbh) {
    StaticBuffers::Header* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_buffer_as_str;
}
void sb_append(StaticBuffers::Handle tbh, char ch) {
    StaticBuffers::Header* tbph = (StaticBuffers::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr)
    ASSERT(tbph->m_used_length + 1 < tbph->m_capacity)
    #endif
    *(tbph->m_next_available_p) = ch;
    tbph->m_next_available_p++;
    tbph->m_used_length++;
}
