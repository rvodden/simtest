#ifndef __LED_H__
#define __LED_H__

#include <stdlib.h>

#include <libwebsockets.h>
#include <sim_avr.h>

#include "simulator.h"

#ifdef LED_IMPORT
    #define EXTERN
#else
    #define EXTERN extern
#endif /* LED_IMPORT */


/* Constants Declarations */

/* Types Declarations */

typedef struct led_t led_t;

component_t* led_init( struct simulator * simulator, const char * name );
void led_destroy( void* param );
void led_switch( struct avr_irq_t * irq, uint32_t value, void * param );
void led_connect( component_t* component, avr_irq_t* irq );

#endif /* __LED_H__ */
