#include <stdint.h>
#include <new>
#include "transmit_buffer_pool.h"
#if  !defined(LOCAL_TEST)
#include "trace.h"
#endif

using RxPool = transport::buffer::Pool<1,128>;
using TxPool = transport::buffer::Pool<3, 512>;

// allocate static space for the 2 buffer pools 
alignas(8) uint8_t  tx_pool_mem[sizeof(TxPool)];
alignas(8) uint8_t  rx_pool_mem[sizeof(RxPool)];

using namespace transport::buffer;

TxPool* tx_pool_ptr;
void tx_pool::init() 
{
    tx_pool_ptr = new ((void*)tx_pool_mem) Pool<3, 512>();
}
Handle tx_pool::allocate() 
{
    return tx_pool_ptr->allocate();
}
void tx_pool::deallocate(transport::buffer::Handle h) 
{
    tx_pool_ptr->deallocate(h);
}


RxPool* rx_pool_ptr;
void rx_pool::init() 
{
    rx_pool_ptr = new ((void*)rx_pool_mem) Pool<1, 128>();
}
Handle rx_pool::allocate() 
{
    return rx_pool_ptr->allocate();
}
void rx_pool::deallocate(transport::buffer::Handle h) 
{
    rx_pool_ptr->deallocate(h);
}

