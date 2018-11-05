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
   
    if(!simulator->context_data->head) return;

    struct websocket_message*  message = malloc(sizeof(struct websocket_message));
    memset(message, 0, sizeof(struct websocket_message));

    switch(value) {
        case 1:
            strcpy(message->payload, "Led On\n");
            message->length = 7;
            break;
        case 0:
            strcpy(message->payload, "Led Off\n");
            message->length = 8;
            break;
        default:
            break;
    }
    
    message->length = strlen(message->payload);

    pthread_mutex_lock(&simulator->lock_ring);
    lws_ring_insert(simulator->ring, message, sizeof(message));
    pthread_mutex_unlock(&simulator->lock_ring);

    websocket_callback_all_in_context_on_writeable(simulator->context_data);
}


