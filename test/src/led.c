#include <stdio.h>

#include <sim_avr.h>

#include "led.h"


void
led_init(
        struct avr_t *avr,
        led_t * led,
        const char * name)
{
    led->irq = avr_alloc_irq(&avr->irq_pool, 0, 1, &name);
    led->avr = avr;
    avr_irq_register_notify( led->irq, &led_switch, NULL);
}

void 
led_switch( struct avr_irq_t * irq, uint32_t value, void * param ) {
    switch(value) {
        case 1:
            printf("LED On\n");
            break;
        case 0:
            printf("LED Off\n");
            break;
        default:
            break;
    }
    fflush(stdout);
}


