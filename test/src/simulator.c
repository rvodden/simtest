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
#include <sim_gdb.h>
#include <avr_ioport.h>

#include <libwebsockets.h>

#include "event.h"
#include "led.h"
#include "button.h"
#include "simulator.h"
#include "websocket.h"

component_t* led;
component_t* button;

struct simulator_t {
    avr_t*                avr;
    struct lws_ring*      output_ring;
    pthread_mutex_t       lock_output_ring;
    struct lws_ring*      input_ring;
    pthread_mutex_t       lock_input_ring;
    struct lws_context*   context;
    uint8_t               terminate;
    pthread_mutex_t       lock_terminate;
    struct component_t*   components_head;
};

/* PUBLIC IMPLEMENTATIONS */

/**
 * \brief creates a new simulator
 *
 *
 */
simulator_t* simulator_init() {
    simulator_t* simulator;

    simulator = malloc(sizeof(simulator_t));

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

//    simulator->avr->gdb_port = 1234;
//    simulator->avr->state = cpu_Stopped;
//    avr_gdb_init(simulator->avr);

    pthread_mutex_init(&simulator->lock_output_ring, NULL);
    simulator->output_ring = lws_ring_create( sizeof(struct websocket_message), 32, websocket_destroy_message);

    pthread_mutex_init(&simulator->lock_input_ring, NULL);
    simulator->input_ring = lws_ring_create( sizeof(struct event_message), 32, destroy_event_message);

    pthread_mutex_init(&simulator->lock_terminate, NULL);
    simulator->terminate = 0;

    /* create the LED */
    led = led_init(simulator, "led");

    /* attach the LED to the port */
    led_connect(led, avr_io_getirq(simulator->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 0));

    /* create the BUTTON */
    button = button_init(simulator, "button");

    /* attach the BUTTON to the port */
    button_connect(button, avr_io_getirq(simulator->avr, AVR_IOCTL_IOPORT_GETIRQ('B'), 4));

    return simulator;
}

int component_get_id(component_t *pComponent);

void simulator_send_message(simulator_t* simulator, char* text) {
    struct websocket_message*  message = calloc(1, sizeof(struct websocket_message));
    lws_strncpy(message->payload, text, WEBSOCKET_MAX_MESSAGE_LENGTH);
    message->length = strlen(message->payload);
    lwsl_debug("Sending the following message: %s\n", message->payload);

    pthread_mutex_lock(&simulator->lock_output_ring);
    if(!lws_ring_get_count_free_elements(simulator->output_ring)) {
        lwsl_user("Dropping as no space in the output_ring buffer.\n");
    }

    lws_ring_insert(simulator->output_ring, message,1);
    lwsl_debug("Cancelling service on context: %p\n", (void *)simulator->context);
    lws_cancel_service(simulator->context);
    pthread_mutex_unlock(&simulator->lock_output_ring);
    free(message);
}

void simulator_destroy(simulator_t* simulator) {
    lwsl_debug("Destroying the components\n");
    destroy_components(simulator->components_head);

    lwsl_debug("Destroying the simulator\n");
    lws_ring_destroy(simulator->output_ring);
    pthread_mutex_destroy(&simulator->lock_output_ring);
    lws_ring_destroy(simulator->input_ring);
    pthread_mutex_destroy(&simulator->lock_input_ring);
    led_destroy(led);
    avr_terminate(simulator->avr);
    free(simulator);
}

void simulator_run(simulator_t* simulator) {
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
        if(message != NULL) {
            lwsl_notice("Got message for: %d, with value %d\n", message->destination_id, message->event.value);

            component_t* destination = get_component_with_id(simulator->components_head, message->destination_id);
            if (destination) {
                lwsl_debug("Message is for %s.\n", get_component_name(destination));
                process_component_message(destination, message);
            }
            lws_ring_consume(simulator->input_ring,NULL,NULL,1);
        }

        pthread_mutex_unlock(&simulator->lock_input_ring);

    }

    switch(state) {
        case cpu_Done:
            lwsl_err("Simulator Thread Exited as CPU is done.\n");
            break;
        case cpu_Crashed:
            lwsl_err("Simulator Thread Exited as CPU crashed.\n");
            break;
    }
    pthread_exit(NULL);
}

void simulator_terminate(simulator_t* simulator) {
    pthread_mutex_lock(&simulator->lock_terminate);
    simulator->terminate = 1;
    pthread_mutex_unlock(&simulator->lock_terminate);
}

avr_irq_pool_t * get_simulator_irq_pool(simulator_t* simulator) {
    return &simulator->avr->irq_pool;
}

component_t* get_simulator_components(simulator_t* simulator) {
    return simulator->components_head;
}

component_t* add_simulator_component(simulator_t* simulator, const char* name, void (*process_message) (component_t*, struct event_message*), void (*destroy) (component_t*), void* definition) {
    component_t *component = component_init(simulator, name, process_message, destroy, definition);
    simulator->components_head = add_component(get_simulator_components(simulator), component);
    return component;
}


struct lws_context* get_simulator_context(simulator_t* simulator) {
    return simulator->context;
}

void set_simulator_context(simulator_t* simulator, struct lws_context* context) {
    simulator->context = context;
}

struct lws_ring* get_simulator_output_ring(simulator_t* simulator) {
    return simulator->output_ring;
}

struct lws_ring* get_simulator_input_ring(simulator_t* simulator) {
    return simulator->input_ring;
}
pthread_mutex_t* get_simulator_output_ring_lock(simulator_t* simulator) {
    return &simulator->lock_output_ring;
}

pthread_mutex_t* get_simulator_input_ring_lock(simulator_t* simulator) {
    return &simulator->lock_input_ring;
}
