#include "simulator.h"

#ifndef __component_h__
#define __component_h__

//TODO: refactor this so that is calculated from WEBSOCKET_MAX_MESSAGE_LENGTH
#define COMPONENT_MAX_MESSAGE_LENGTH 50

component_t* add_component_to_simulator(struct simulator* simulator, const char* name, void (*process_message) (component_t*, struct event_message*), void (*destroy) (component_t*), void* definition);
void simulator_remove_component(struct simulator*, struct component_t*);
void destroy_components(component_t* component);
component_t* get_component_with_id(component_t* head, int id);
void* get_component_definition(component_t* component);
const char* get_component_name(component_t* component);
void send_component_message(component_t* component, char* message);
void process_component_message(component_t* component, struct event_message* message);
#endif // __component_h__
