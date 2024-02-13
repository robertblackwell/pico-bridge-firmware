#ifndef H_transport_buffer_h
#define H_transport_buffer_h
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include "trace.h"
namespace transport {
namespace buffer {

    struct Header {
        Header(void *memptr, size_t capacity) {
            m_available = true;
            m_capacity = capacity;
            m_used_length = 0;
            m_buffer_ptr = (char *) memptr;
            m_next_available_p = m_buffer_ptr;
            m_buffer_as_str = m_buffer_ptr;
        }

        void reset() {
            m_used_length = 0;
            *m_buffer_ptr = '\0';
            m_next_available_p = m_buffer_ptr;
            m_buffer_as_str = m_buffer_ptr;
        }

        bool m_available;
        size_t m_used_length;
        size_t m_capacity;
        char *m_buffer_ptr;
        char *m_next_available_p;
        char *m_buffer_as_str;
        // char   m_buffer[512];
    };

// using Handle = void*;
    typedef Header *Handle;

    template<int const N>
    struct Buffer {
        Handle get_handle() {
            return (Handle) (&(this->m_header));
        }

        Buffer(): m_header(&m_mem[0], N) {
        }

        Header m_header;
        uint8_t m_mem[N]{};
    };

    template<int const HOWMANY, int const SIZE>
    struct Pool {
        Pool() {
            m_size = SIZE;
            m_number = HOWMANY;
            for (int i = 0; i < HOWMANY; i++) {
                m_buffers[i].m_header.m_available = true;
            }
        }

        int m_size;
        int m_number;
        Buffer<SIZE> m_buffers[HOWMANY];

        Handle allocate() {
            for (int i = 0; i < HOWMANY; i++) {
                Header *tbph = &(m_buffers[i].m_header);
                if (tbph->m_available) {
                    tbph->m_available = false;
                    return (Handle) (tbph);
                }
            }
            ASSERT_MSG(false, "Should not get here - used too many static buffers")
            return nullptr;
        }

        void deallocate(Handle tbh) {
            auto *tbph = (Header *) tbh;
            ASSERT(tbph != nullptr);
            ASSERT((tbph)->m_available == false);
            tbph->m_available = true;
            tbph->m_used_length = 0;
            tbph->m_next_available_p = tbph->m_buffer_ptr;
            tbph->m_buffer_as_str = tbph->m_next_available_p;
        }
    };
    
    #define TransportBuffer_JsonAddValue(bh, fmt, value) \
    do {\
        using namespace transport::buffer; \
            auto tbph = (Header*)b; \
            tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value); \
            tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]); \
    } while(0);


    void sb_json_add(transport::buffer::Handle b, const char* fmt);

    #define TEMPLATE_JSON_ADD
    #ifdef TEMPLATE_JSON_ADD

    template<typename T>
        void sb_json_add(transport::buffer::Handle b, const char* fmt, T value)
        {
            auto tbph = (transport::buffer::Header*)b;
            tbph->m_used_length += snprintf(tbph->m_next_available_p, tbph->m_capacity - tbph->m_used_length, fmt, value);
            tbph->m_next_available_p = (char*)&(tbph->m_buffer_ptr[tbph->m_used_length]);
        }
    #else
    void sb_json_add(transport::buffer::Handle b, const char* fmt, const char* value);

    void sb_json_add(transport::buffer::Handle b, const char* fmt, void* value);

    void sb_json_add(transport::buffer::Handle b, const char* fmt, long value);

    void sb_json_add(transport::buffer::Handle b, const char* fmt, double value);

    void sb_json_add(transport::buffer::Handle b, const char* fmt, float value);
    #endif

    size_t sb_sprintf(Handle buffer_h, const char *fmt, ...);

    size_t sb_vsprintf(Handle buffer_h, const char *fnt, va_list args);

    char *sb_buffer_as_cstr(Handle buffer_h);

    size_t sb_buffer_length(Handle buffer_h);

    void sb_append(Handle buffer_h, char ch);

    void sb_reset(Handle bh);

} // namespace buffer
} // namespace transport
#endif