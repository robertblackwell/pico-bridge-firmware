#ifndef H_pico_gpio_irq_dispatcher_h
#define H_pico_gpio_irq_dispatcher_h

// #include <pico/stdlib.h>
// #include <stdio.h>
#include <stdint.h>
#include <pico/types.h>
#include <pico/time.h>

#define MAX_PICO_IRQ_HANDLERS 5
class PicoGpioIrqDispatcher
{
	public:
	typedef void(*GpioIrqHandler)();
	struct IrqHandlerEntry {
		bool			active;
		uint    		gpio_pin;
		GpioIrqHandler	handler;
	};
	IrqHandlerEntry handler_table[MAX_PICO_IRQ_HANDLERS];
	uint irq_handler_count;
	PicoGpioIrqDispatcher();
	void add_handler(uint gpio_pin, GpioIrqHandler handler);
	IrqHandlerEntry* find(uint gpio_pin);
	void remove_handler(uint gpio_pin);
	void call_handler(uint gpio_pin);
};
void gpio_dispatcher_irq_handler(uint gpio_pin, uint32_t events);
void attachGpioInterrupt(uint gpio_pin, PicoGpioIrqDispatcher::GpioIrqHandler handler);
void detachGpioInterruptHandler(uint gpio_pin);
void gpio_irq_off(uint gpio_pin);
void gpio_irq_backon(uint gpio_pin);
bool digital_read(uint gpio_pin);
uint32_t millis();
uint64_t micros();
bool digital_read(int pin);
bool stdio_available_read();
bool stdio_connected();
#endif