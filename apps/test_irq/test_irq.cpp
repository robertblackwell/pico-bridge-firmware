
#include <pico/stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <pico/types.h>
#include <hardware/gpio.h>
#include <pico_gpio_irq_dispatcher.h>
#define OUTPIN_01 13
#define OUTPIN_02 12
#define INPIN_01 15
#define INPIN_02 14

static const char* gpio_irq_str[] = {
	"LEVEL_LOW",
	"LEVEL_HIGH",
	"EDGE_FALL",
	"EDGE_RISE"
};

auto gpio_event_string(char* buf, uint32_t events) -> void
{
	for(uint i = 0; i < 4; i++) {
		uint mask = (1<<i);
		if(events & mask) {
			const char* event_str = gpio_irq_str[i];
			while (*event_str != '\0') {
				*buf++ = *event_str++;
			}
		}
		events &= ~mask;
	}
	*buf++ = '\0';
}

static char events_str[128];

auto irq_handler_01(uint gpio_pin, uint32_t events) -> void 
{
	// gpio_event_string(events_str, events);
	printf("irq_handler_01 pin: %u  event: %x \n", gpio_pin, events);
}
auto irq_handler_02(uint gpio_pin, uint32_t events) -> void 
{
	// gpio_event_string(events_str, events);
	printf("irq_handler_02 pin: %u  event: %x \n", gpio_pin, events);
}
auto irq_dispatcher(uint gpio_pin, uint32_t events) -> void 
{
	// gpio_event_string(events_str, events);
	printf("irq_dispatcher pin: %u  event: %x \n", gpio_pin, events);
	if(gpio_pin == INPIN_01) {
		irq_handler_01(gpio_pin, events);
	} else if(gpio_pin == INPIN_02) {
		irq_handler_02(gpio_pin, events);
	}
}


auto irq_raw_handler_01() -> void 
{
	if(gpio_get_irq_event_mask(INPIN_01) & GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE) {
		gpio_acknowledge_irq(INPIN_01, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE);
	}
	// gpio_event_string(events_str, events);
	printf("irq_raw_handler_01\n");
}
auto irq_raw_handler_02() -> void 
{
	// gpio_event_string(events_str, events);
	printf("irq_raw_handler_02\n");
}
#if 0
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
	PicoGpioIrqDispatcher()
	{
		irq_handler_count = 0;
		for(int i = 0; i < MAX_PICO_IRQ_HANDLERS; i++) {
			handler_table[i].active = false;
			handler_table[i].handler = nullptr;
		}
	}
	void add_handler(uint gpio_pin, GpioIrqHandler handler)
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
	IrqHandlerEntry* find(uint gpio_pin)
	{
		printf("find: %u\n", gpio_pin);
		for(uint i = 0; i < MAX_PICO_IRQ_HANDLERS; i++) {
			if(handler_table[i].gpio_pin == gpio_pin) {
				printf("find - found i: %u  pin: %u\n", i, handler_table[i].gpio_pin);
				return &(handler_table[i]);
			}
		}
		return nullptr;
		FATAL_ERROR_PRINTF("PicoIrqDispatcher::find could not find handler for pin : %u\n", gpio_pin);
	} 
	void remove_handler(uint gpio_pin)
	{
		IrqHandlerEntry* entry = find(gpio_pin);
		if(entry != nullptr) 
			entry->handler = nullptr;
	}
	void call_handler(uint gpio_pin)
	{
		printf("call_handler gpio_pi: %u \n", gpio_pin);
		auto entry_ptr = find(gpio_pin);
		if(entry_ptr->active) {
			entry_ptr->handler();
		} 
	}
};
PicoGpioIrqDispatcher irq_dispatcher_instance;
void gpio_dispatcher_irq_handler(uint gpio_pin, uint32_t events)
{
	printf("gpio_dispatcher_irq_handler pin: %u events: %x\n", gpio_pin, events);
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
#endif
int main() 
{
	printf("We are here\n");
	stdio_init_all();
	sleep_ms(2000);
	printf("Starting test of PicoGpioIrqDispatcher\n");

	gpio_init(OUTPIN_01);
	gpio_set_dir(OUTPIN_01, GPIO_OUT);
	gpio_init(OUTPIN_02);
	gpio_set_dir(OUTPIN_02, GPIO_OUT);
	auto output_01 = true;
	auto output_02 = true;
	#if 0
	gpio_init(INPIN_01);
	gpio_set_dir(INPIN_01, GPIO_IN);
	gpio_set_irq_enabled_with_callback(INPIN_01, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, irq_dispatcher);
	gpio_init(INPIN_02);
	gpio_set_dir(INPIN_02, GPIO_IN);
	gpio_set_irq_enabled_with_callback(INPIN_02, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true, irq_dispatcher);
	#endif
	attachGpioInterrupt(INPIN_01, irq_raw_handler_01);
	attachGpioInterrupt(INPIN_02, irq_raw_handler_02);
	printf("Init complete\n");
	int counter = 0;
	while(1) {
		counter++;
		if(counter == 10) {
			gpio_irq_off(INPIN_01);
		}
		if(counter == 20) {
			gpio_irq_backon(INPIN_01);
		}
		if(counter == 30) {
			gpio_irq_off(INPIN_02);
		}
		if(counter == 40) {
			gpio_irq_backon(INPIN_02);
		}
		printf("Loop counter: %d\n", counter);
		gpio_put(OUTPIN_01, (output_01));
		output_01 = !output_01;
		gpio_put(OUTPIN_02, (output_02));
		output_02 = !output_02;
		sleep_ms(3000);
	}
}