#include <stdlib.h>
#include <libwebsockets.h>
#include <sim_avr.h>

#include "simulator.h"

#ifndef __BUTTON_H__
#define __BUTTON_H__

typedef struct button_t {
    avr_irq_t * irq;	// input irq
    struct simulator *simulator;
    uint8_t value;
    const char* name;
} button_t;

void button_init( struct simulator * simulator, button_t* button, const char * name );
void button_destroy( button_t* button );
void button_press( button_t* );
void button_release( button_t* );

#endif /* __BUTTON_H__ */
