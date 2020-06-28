#include <sim_avr.h>
#undef ARRAY_SIZE
#include <libwebsockets.h>

#include "event.h"

#ifndef __simulator_h__
#define __simulator_h__

struct simulator {
    avr_t*                avr;
    struct lws_ring*      output_ring;
    pthread_mutex_t       lock_output_ring;
    struct lws_ring*      input_ring;
    pthread_mutex_t       lock_input_ring;
    struct lws_context*   context;
    uint8_t               terminate;
    pthread_mutex_t       lock_terminate;
    struct component_t*   components_head;
};

typedef struct component_t component_t;

struct simulator* simulator_init(void);
void simulator_destroy(struct simulator*);
void simulator_run(struct simulator*);
void simulator_terminate(struct simulator*);

component_t* simulator_add_component(struct simulator* simulator, const char* name, void (*process_message) (component_t*, struct event_message*), void (*destroy) (component_t*), void* definition);
void simulator_remove_component(struct simulator*, struct component_t*);
void* simulator_component_get_definition(component_t *component);
const char* simulator_component_get_name(component_t *component);
void simulator_send_message(component_t *component, char* message);
void simulator_process_message(char*);

#endif /* __simulator_h__ */
