#include <stdlib.h>

#include <sim_avr.h>

#ifndef __LED_H__
#define __LED_H__

typedef struct led_t {
    avr_irq_t * irq;	// output irq
    struct avr_t * avr;
    uint8_t value;
} led_t;


void led_init( struct avr_t * avr, led_t* led, const char * name);
void led_switch( struct avr_irq_t * irq, uint32_t value, void * param );

#endif /* __LED_H__ */
