#include "c_static_buffers.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

buffer_header_init(buffer_header_t* bh, void* memptr, size_t capacity) {
    bh->m_available = true;
    bh->m_capacity = capacity;
    bh->m_used_length = 0;
    bh->m_buffer_ptr = (char*)memptr;
    bh->m_next_available_p = bh->m_buffer_ptr;
    bh->m_buffer_as_str = bh->m_buffer_ptr;
}
void buffer_header_reset(buffer_header_t* bh) {
    bh->m_used_length = 0;
    bh->m_next_available_p = bh->m_buffer_ptr;
    bh->m_buffer_as_str = bh->m_buffer_ptr;
}

typedef struct pool_internals_s {
    int m_size;
    int m_number;
    union {
        void* start_of_buffers;
    };
} pool_internals_t;

void buffer_pool_init(void* pool_ptr)
{
    uint8_t* first_buffer_ptr =  
    int buf_size = sizeof(struct{buffer_header_t hd, uint8 m[] })
}

size_t csb_sprintf(buffer_handle_t tbh, const char* fmt, ...) {
    buffer_header_t* tbph = (buffer_header_t*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != NULL);
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
size_t csb_buffer_length(buffer_handle_t tbh) {
    buffer_header_t* tbph = (buffer_header_t*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != NULL);
    #endif
    return tbph->m_used_length;
}
char* csb_buffer_as_cstr(buffer_handle_t tbh) {
    buffer_header_t* tbph = (buffer_header_t*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != NULL);
    #endif
    return tbph->m_buffer_as_str;
}
void csb_append(buffer_handle_t tbh, char ch) {
    buffer_header_t* tbph = (buffer_header_t*)tbh;
    #ifndef LOCAL_TEST
    ASSERT(tbph != NULL)
    ASSERT(tbph->m_used_length + 1 < tbph->m_capacity)
    #endif
    *(tbph->m_next_available_p) = ch;
    tbph->m_next_available_p++;
    tbph->m_used_length++;
}
