#include <stdio.h>
#include <sim_avr.h>

#include "led.h"
#include "websocket.h"

void led_init(
        struct simulator *simulator,
        led_t * led,
        const char * name)
{
    printf("Initializing LED: %s\n", name);
    led->irq = avr_alloc_irq(&simulator->avr->irq_pool, 0, 1, &name);
    led->simulator = simulator;
    led->name = name;
    avr_irq_register_notify(led->irq, &led_switch, simulator);
    simulator_add_component(simulator, led);
}

void led_destroy(led_t* led)
{
    printf("Destroying LED: %s\n", led->name);
    avr_free_irq(led->irq, 1);
}

void led_switch( struct avr_irq_t * irq, uint32_t value, void * param ) {
    struct simulator *simulator = (struct simulator*) param;

    struct websocket_message*  message = malloc(sizeof(struct websocket_message));
    memset(message, 0, sizeof(struct websocket_message));

    switch(value) {
        case 1:
            message->length = lws_snprintf(message->payload, WEBSOCKET_MAX_MESSAGE_LENGTH, "%s", "Led On\n");
            break;
        case 0:
            message->length = lws_snprintf(message->payload, WEBSOCKET_MAX_MESSAGE_LENGTH, "%s", "Led Off\n");
            break;
        default:
            break;
    }

    pthread_mutex_lock(&simulator->lock_ring);
    if(!lws_ring_get_count_free_elements(simulator->ring)) {
        lwsl_user("Dropping as no space in the ring buffer.\n");
    }

    lws_ring_insert(simulator->ring, message,1);
    lwsl_debug("Cancelling service on context: %p\n", (void *)simulator->context);
    lws_cancel_service(simulator->context);
    pthread_mutex_unlock(&simulator->lock_ring);
}
