#include "buffers.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt)
{
    using namespace transport::buffer;
    auto tbph = (Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}
#ifdef TEMPLATE_JSON_ADD
#else
void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt, void* value)
{
    auto tbph = (transport::buffer::Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}

void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt, const char* value)
{
    auto tbph = (transport::buffer::Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}
void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt, long value)
{
    auto tbph = (transport::buffer::Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}
void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt, float value)
{
    auto tbph = (transport::buffer::Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}
void transport::buffer::sb_json_add(transport::buffer::Handle b, const char* fmt, double value)
{
    auto tbph = (transport::buffer::Header*)b;
    tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
    tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
}
#endif
size_t transport::buffer::sb_sprintf(transport::buffer::Handle tbh, const char* fmt, ...) {
    auto* tbph = (transport::buffer::Header*)tbh;
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
size_t transport::buffer::sb_vsprintf(transport::buffer::Handle tbh, const char* fmt, va_list vlist) {
    auto* tbph = (transport::buffer::Header*)tbh;
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

size_t transport::buffer::sb_buffer_length(transport::buffer::Handle tbh) {
    auto* tbph = (transport::buffer::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_used_length;
}
char* transport::buffer::sb_buffer_as_cstr(transport::buffer::Handle tbh) {
    auto* tbph = (transport::buffer::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr);
    #endif
    return tbph->m_buffer_as_str;
}
void transport::buffer::sb_append(transport::buffer::Handle tbh, char ch) {
    auto* tbph = (transport::buffer::Header*)tbh;
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
void transport::buffer::sb_reset(transport::buffer::Handle tbh)
{
    auto* tbph = (transport::buffer::Header*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != nullptr)
    #endif
    tbph->reset();
}
