#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include <tusb.h>
#include "cli.h"

int main()
{
    Cli cli;
    stdio_init_all();
    while(! tud_cdc_connected()) {
        sleep_ms(100);
    }
    cli.begin();
    while(true) {
        sleep_ms(500);
        cli.run();
        if(cli.available()) {
            Argv& argv = cli.consume();
            argv.dump("There was a message :");
        }
    }
}
