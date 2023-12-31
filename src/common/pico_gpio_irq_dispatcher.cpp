#include "pico_gpio_irq_dispatcher.h"
#include <pico/stdlib.h>

#include <stdio.h>
#include <stdint.h>
#include <pico/types.h>
#include <hardware/gpio.h>

#include "trace.h"

PicoGpioIrqDispatcher::PicoGpioIrqDispatcher()
{
    irq_handler_count = 0;
    for(int i = 0; i < MAX_PICO_IRQ_HANDLERS; i++) {
        handler_table[i].active = false;
        handler_table[i].handler = nullptr;
    }
}
void PicoGpioIrqDispatcher::add_handler(uint gpio_pin, GpioIrqHandler handler)
{
    ASSERT_PRINTF((handler != nullptr), "PicoGpioIrqDispatch::add_handler handler == nullptr gpio_pin: %u", gpio_pin)
    for(uint i = 0; i < MAX_PICO_IRQ_HANDLERS; i++) {
        if(handler_table[i].handler == nullptr) {
            handler_table[i].active = true;
            handler_table[i].handler = handler;
            handler_table[i].gpio_pin = gpio_pin;
            irq_handler_count += 1;
            return;
        }
    }
    FATAL_ERROR_PRINTF("PicoIrqDispatcher::add table full pin number: %u\n", gpio_pin);
}
PicoGpioIrqDispatcher::IrqHandlerEntry* PicoGpioIrqDispatcher::find(uint gpio_pin)
{
    FTRACE("find: %u\n", gpio_pin);
    for(uint i = 0; i < MAX_PICO_IRQ_HANDLERS; i++) {
        if(handler_table[i].gpio_pin == gpio_pin) {
            FTRACE("find - found i: %u  pin: %u\n", i, handler_table[i].gpio_pin);
            return &(handler_table[i]);
        }
    }
    return nullptr;
    FATAL_ERROR_PRINTF("PicoIrqDispatcher::find could not find handler for pin : %u\n", gpio_pin);
} 
void PicoGpioIrqDispatcher::remove_handler(uint gpio_pin)
{
    IrqHandlerEntry* entry = find(gpio_pin);
    if(entry != nullptr) 
        entry->handler = nullptr;
}
void PicoGpioIrqDispatcher::call_handler(uint gpio_pin)
{
    FTRACE("call_handler gpio_pi: %u \n", gpio_pin);
    auto entry_ptr = find(gpio_pin);
    if(entry_ptr->active) {
        entry_ptr->handler();
    } 
}

static PicoGpioIrqDispatcher irq_dispatcher_instance;

void gpio_dispatcher_irq_handler(uint gpio_pin, uint32_t events)
{
	FTRACE("gpio_dispatcher_irq_handler pin: %u events: %x\n", gpio_pin, events);
	irq_dispatcher_instance.call_handler(gpio_pin);
}
void attachGpioInterrupt(uint gpio_pin, PicoGpioIrqDispatcher::GpioIrqHandler handler)
{
	gpio_init(gpio_pin);
	gpio_set_dir(gpio_pin, GPIO_IN);
	irq_dispatcher_instance.add_handler(gpio_pin, handler);
	gpio_set_irq_enabled_with_callback(gpio_pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, gpio_dispatcher_irq_handler);
}
void detachGpioInterruptHandler(uint gpio_pin)
{
	irq_dispatcher_instance.remove_handler(gpio_pin);
}
void gpio_irq_off(uint gpio_pin)
{
	auto entry = irq_dispatcher_instance.find(gpio_pin);
	entry->active = false;
}
void gpio_irq_backon(uint gpio_pin)
{
	auto entry = irq_dispatcher_instance.find(gpio_pin);
	if(entry != nullptr)
		entry->active = true;
}
uint32_t millis()
{
    return to_ms_since_boot(get_absolute_time());
}
uint64_t micros()
{
    return to_us_since_boot(get_absolute_time());
}
bool digital_read(int pin)
{
    return (gpio_get(pin) == 0) ? 0: 1;
}
#if 0
bool stdio_chars_avalable_flag = false;
void stdio_chars_available_callback(void* arg)
{
    stdio_chars_avalable_flag = true;
}

bool stdio_available_read()
{
    // return stdio_chars_avalable_flag;
}
int iracoon_stdio_getchar()
{
    if(stdio_available_read()) {
        int chint = getchar_timeout_us(5);
        if(0 <= chint and chint <=255) {
            return chint;
        } else {

        }
    }
}
bool stdio_connected(){}
bool stdio_available_read()
{
    return tud_cdc_available();
}
bool stdio_connected()
{
    return tud_cdc_connected();
}
#else
#endif