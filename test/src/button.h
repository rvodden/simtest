#include <stdlib.h>
#include <libwebsockets.h>
#include <sim_avr.h>

#ifndef __BUTTON_H__
#define __BUTTON_H__

#include "simulator.h"
#include "component.h"
#include "event.h"

typedef struct button_t button_t;

component_t* button_init( simulator_t * simulator, const char * name );
void button_destroy( component_t* component );
void button_connect( component_t* component, avr_irq_t* irq );
void button_process_message(component_t* component, struct event_message* message);
void button_press( button_t* );
void button_release( button_t* );

#endif /* __BUTTON_H__ */
