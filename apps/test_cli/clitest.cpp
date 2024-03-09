#include <stdio.h>
#include "pico/stdlib.h"

int main()
{
    while(true) {
        sleep_ms(500);
        cli.run();
        if(cli.available()) {
            Argv& argv = cli.consume();
            argv.dump("There was a message :");
        }
    }
}
