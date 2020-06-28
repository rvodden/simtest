#ifndef __simulator_h__
#define __simulator_h__

#include <sim_avr.h>
#include <libwebsockets.h>
#undef ARRAY_SIZE

#include <libwebsockets.h>
#include <pthread.h>

typedef struct simulator_t simulator_t;

#include "component.h"
#include "event.h"

simulator_t* simulator_init(void);
void simulator_destroy(simulator_t*);
void simulator_run(simulator_t*);
void simulator_terminate(simulator_t*);

void simulator_send_message(simulator_t*, char*);
void simulator_process_message(char*);
avr_irq_pool_t * get_simulator_irq_pool(simulator_t* simulator);
component_t* get_simulator_components(simulator_t* simulator);
component_t* add_simulator_component(
        simulator_t* simulator,
        const char* name,
        void (*process_message) (component_t*, struct event_message*),
        void (*destroy) (component_t*),
        void* definition);

struct lws_context* get_simulator_context(simulator_t* simulator);
void set_simulator_context(simulator_t* simulator, struct lws_context* context);

struct lws_ring* get_simulator_output_ring(simulator_t* simulator);
struct lws_ring* get_simulator_input_ring(simulator_t* simulator);
pthread_mutex_t* get_simulator_output_ring_lock(simulator_t* simulator);
pthread_mutex_t* get_simulator_input_ring_lock(simulator_t* simulator);

#endif /* __simulator_h__ */
