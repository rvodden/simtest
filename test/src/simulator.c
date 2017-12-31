#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sim_irq.h>
#include <sim_avr.h>
#include <sim_elf.h>
#include <avr_ioport.h>

#include "led.h"
#include "simulator.h"

led_t led;

avr_t* simulator_init( void ) {
    avr_t * avr;

    elf_firmware_t firmware;
    const char* firmware_filename = "main.elf";

    if( elf_read_firmware(firmware_filename, &firmware) ) 
    {
        fprintf(stderr, "Unable to open firmware: %s\n", firmware_filename );        
        exit(-1);
    }

    printf("firmware %s f=%d mmcu=%s\n", firmware_filename, (int)firmware.frequency, firmware.mmcu);

    if( strlen(firmware.mmcu) == 0) {
        fprintf(stderr, "Unable to get cpu type from firmware; defaulting to attiny85.\n");
        strcpy(firmware.mmcu, "attiny85");
    }

    if((int)firmware.frequency == 0) {
        fprintf(stderr, "Unable to get cpu frequency from firmware; defaulting to 8000000.\n");
        firmware.frequency = 8000000;
    }

    avr = avr_make_mcu_by_name( firmware.mmcu );

    if (!avr) {
        fprintf(stderr, "AVR '%s' not known\n", firmware.mmcu);
        exit(-2);
    }

    avr_init(avr);
    avr_load_firmware(avr, &firmware);

    led_init(avr, &led, "led" );

    avr_connect_irq(avr_io_getirq(avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led.irq);

    return avr;
}

void simulator_run(avr_t *avr) {
    int state = 0;

    while (1) {
        state = avr_run(avr);
        if (state == cpu_Done || state == cpu_Crashed)
            break;
    }

    avr_terminate(avr);
}
