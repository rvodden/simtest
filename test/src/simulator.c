#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sim_irq.h>
#include <sim_avr.h>
#include <sim_elf.h>
#include <avr_ioport.h>

#include <libwebsockets.h>

#include "led.h"
#include "simulator.h"
#include "websocket.h"

led_t led;

struct simulator* simulator_init() {
    struct simulator * simulator;

    simulator = malloc(sizeof(struct simulator));

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

    simulator->avr = avr_make_mcu_by_name( firmware.mmcu );

    if (!simulator->avr) {
        fprintf(stderr, "AVR '%s' not known\n", firmware.mmcu);
        exit(-2);
    }

    avr_init(simulator->avr);
    avr_load_firmware(simulator->avr, &firmware);

    pthread_mutex_init(&simulator->lock_ring, NULL);
    simulator->ring = lws_ring_create( sizeof(struct websocket_message), 32, websocket_destroy_message);

    led_init(simulator, &led, "led");

    avr_connect_irq(avr_io_getirq(simulator->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led.irq);

    return simulator;
}

void simulator_run(struct simulator* simulator) {
    int state = 0;

    while (1) {
        state = avr_run(simulator->avr);
        if (state == cpu_Done || state == cpu_Crashed)
            break;
    }

    avr_terminate(simulator->avr);
}
