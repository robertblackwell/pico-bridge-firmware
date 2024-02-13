#ifndef H_c_staticbuffers_h
#define H_c_staticbuffers_h
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "c_static_buffers.h"
#define slice_t(NAME) 
#define NAME transmit_buffer_pool
// #define BUFFER_POOL(SIZE, HOWMANY, NAME) \

    typedef struct NAME ## _slice_s { \
        buffer_header_t m_header;  \
        uint8_t m_mem[SIZE];\
    } NAME ## _slice_t; \
    struct NAME ## _s{ \
        int m_size=SIZE; \ 
        int m_number = HOWMANY; \
        NAME ## _slice_t buffers[HOWMANY];\
    } NAME ; \
    inline void NAME ##_init() { \
        for(int i = 0; i < HOWMANY; i++) { \
            buffer_header_init(&(NAME ## .buffers[i].m_header), &(NAME ## .buffers[i].m_mem), SIZE); \
        } \
    } \
    inline void* NAME ## _allocate(void* pool_ptr) { \
        for(int i = 0; i < HOWMANY; i++) { \
            buffer_header_t hdp = &(NAME ## .buffers[i].m_header); \
            if(bhp->m_available) { \
                return bhp; \
            } \
        } \
        ASSERT_MSG(false, "Should not get here - used too many static buffers"); \
        return NULL; \
    } 

buffer_handle_t buffer_pool_allocate();
void buffer_pool_deallocate(buffer_handle_t h);

BUFFER_POOL(512, 4, transmit_pool)


size_t    csb_sprintf(buffer_handle_t tbuffer, const char* fmt, ...);
char*     csb_buffer_as_cstr(buffer_handle_t tbuffer);
size_t    csb_buffer_length(buffer_handle_t tbuffer);
void      csb_append(buffer_handle_t tbp, char ch);

#endif