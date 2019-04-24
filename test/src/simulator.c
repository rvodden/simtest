/**
 * \file simulator.c
 * \author Richard Vodden
 * \date 24/04/19
 * \brief Contains the core implementation of the simulator runtime functions.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sim_irq.h>
#include <sim_avr.h>
#include <sim_elf.h>
#include <avr_ioport.h>

#include <libwebsockets.h>

#include "event.h"
#include "led.h"
#include "button.h"
#include "simulator.h"
#include "websocket.h"

led_t led;
button_t button;

/** 
 * \brief creates a new simulator
 *
 *
 */
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

    simulator->components_head = NULL;

    avr_init(simulator->avr);
    avr_load_firmware(simulator->avr, &firmware);

    pthread_mutex_init(&simulator->lock_output_ring, NULL);
    simulator->output_ring = lws_ring_create( sizeof(struct websocket_message), 32, websocket_destroy_message);

    pthread_mutex_init(&simulator->lock_input_ring, NULL);
    simulator->input_ring = lws_ring_create( sizeof(struct event_message), 32, destroy_event_message);

    pthread_mutex_init(&simulator->lock_terminate, NULL);
    simulator->terminate = 0;

    /* create the LED */
    led_init(simulator, &led, "led");

    /* attach the LED to the port */
    avr_connect_irq(avr_io_getirq(simulator->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0), led.irq);

    /* create the BUTTON */
    button_init(simulator, &button, "button");

    /* attach the BUTTON to the port */
    avr_connect_irq(button.irq, avr_io_getirq(simulator->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 4));

    return simulator;
}

void simulator_add_component(struct simulator* simulator, struct component* component) {
    struct component* tail = simulator->components_head;
    component->next = NULL;

    if (!tail) {
        simulator->components_head = component;
    } else {
        while(tail->next) {
            tail = tail->next;
        }
        tail->next = component;
    }
}

void simulator_destroy(struct simulator* simulator) {
    lwsl_debug("Destroying the simulator\n");
    lws_ring_destroy(simulator->output_ring);
    pthread_mutex_destroy(&simulator->lock_output_ring);
    lws_ring_destroy(simulator->input_ring);
    pthread_mutex_destroy(&simulator->lock_input_ring);
    led_destroy(&led);
    avr_terminate(simulator->avr);
    free(simulator);
}

void simulator_run(struct simulator* simulator) {
    int state = 0;
    int terminated = 0;

    while (1) {
        state = avr_run(simulator->avr);
        pthread_mutex_lock(&simulator->lock_terminate);
        terminated = simulator->terminate;
        pthread_mutex_unlock(&simulator->lock_terminate);
        if (terminated || state == cpu_Done || state == cpu_Crashed)
            break;

        pthread_mutex_lock( &simulator->lock_input_ring );
		struct event_message* message = ( struct event_message* ) lws_ring_get_element( simulator->input_ring, NULL);
       
        if(message) {
            lwsl_notice("Got message for: %s, with value %d\n", message->destination, message->event.value);
            lws_ring_consume(simulator->input_ring,NULL,NULL,1);
        }


        pthread_mutex_unlock(&simulator->lock_input_ring);
    }

    pthread_exit(NULL);
}

void simulator_terminate(struct simulator* simulator) {
    pthread_mutex_lock(&simulator->lock_terminate);
    simulator->terminate = 1;
    pthread_mutex_unlock(&simulator->lock_terminate);
}
