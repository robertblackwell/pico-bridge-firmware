#ifndef H_transport_h
#define H_transport_h
#include <stdio.h>
#include <pico_gpio_irq_dispatcher.h>
#include "pico/stdlib.h"

int transport_send(char* buffer, int len);

// class Transport
// {

//     /**
//      * Send a buffer of given length to the host via the stdio serial interface
//     */

// };


#endif