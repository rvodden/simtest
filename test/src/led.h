#include <stdlib.h>

#include <libwebsockets.h>
#include <sim_avr.h>
#include "simulator.h"

#ifndef __LED_H__
#define __LED_H__



typedef struct led_t {
    avr_irq_t * irq;	// output irq
    struct simulator *simulator;
    uint8_t value;
    const char* name;
} led_t;


void led_init( struct simulator * simulator, led_t* led, const char * name );
void led_destroy( led_t* led );
void led_switch( struct avr_irq_t * irq, uint32_t value, void * param );

#endif /* __LED_H__ */
