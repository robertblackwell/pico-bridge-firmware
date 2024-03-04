#include "buffers.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

namespace transport::buffer {

    size_t Header::space_remaining()
    {
        // auto* tbph = (transport::buffer::Header*)tbh;
        auto x = this->m_capacity - this->m_used_length;
        return x;
    }
    size_t Header::length() {
        return this->m_used_length;
    }
    char* Header::as_cstr() {
        return this->m_buffer_as_str;
    }
    void Header::append(char ch) {
        #ifndef LOCAL_TEST
        ASSERT(this->m_used_length + 1 < this->m_capacity)
        #endif
        *(this->m_next_available_p) = ch;
        this->m_next_available_p++;
        this->m_used_length++;
        // keep the buffer a valid cstr
        *(this->m_next_available_p) = '\0';
    }
    void Header::reset()
    {
        m_used_length = 0;
        *m_buffer_ptr = '\0';
        m_next_available_p = m_buffer_ptr;
        m_buffer_as_str = m_buffer_ptr;
    }

} // namespace

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
size_t transport::buffer::sb_space_remaining(transport::buffer::Handle tbh)
{
    return tbh->space_remaining();
}
size_t transport::buffer::sb_buffer_length(transport::buffer::Handle tbh) {
    return tbh->length();
}
char* transport::buffer::sb_buffer_as_cstr(transport::buffer::Handle tbh) {
    return tbh->as_cstr();
}
void transport::buffer::sb_append(transport::buffer::Handle tbh, char ch) {
    tbh->append(ch);
}
void transport::buffer::sb_reset(transport::buffer::Handle tbh)
{
    tbh->reset();
}
