#include <stdio.h>

#include <sim_avr.h>

#include "led.h"
#include "websocket.h"

int msq_num = 0;
struct lws* lwsi;

void
led_init(
        struct simulator *simulator,
        led_t * led,
        const char * name)
{
    led->irq = avr_alloc_irq(&simulator->avr->irq_pool, 0, 1, &name);
    led->simulator = simulator;
    avr_irq_register_notify( led->irq, &led_switch, simulator);
}

void 
led_switch( struct avr_irq_t * irq, uint32_t value, void * param ) {
    struct simulator *simulator = (struct simulator*) param;
    struct websocket_message message;
    switch(value) {
        case 1:
            message.payload = "Led On\n";
            message.length = strlen(message.payload);
            lws_ring_insert(simulator->ring, &message, sizeof(message));
            break;
        case 0:
            message.payload = "Led Off\n";
            message.length = strlen(message.payload);
            lws_ring_insert(simulator->ring, &message, sizeof(message));
            break;
        default:
            break;
    }
}


