#include <sim_avr.h>
#undef ARRAY_SIZE
#include <libwebsockets.h>

#ifndef __simulator_h__
#define __simulator_h__

struct simulator {
    avr_t*              avr;
    struct lws_ring*    ring;
    struct lws_context* context;
    pthread_mutex_t     lock_ring;
    uint8_t             terminate;
    pthread_mutex_t     lock_terminate;
    struct component*   head;
};

struct component {
    struct components* next;
    char* name;
    void (*process_message) (char*); 
};

struct simulator* simulator_init(void);
void simulator_destroy(struct simulator*);
void simulator_run(struct simulator*);
void simulator_terminate(struct simulator*);

void simulator_add_component(struct simulator*, struct component*);
void simulator_remove_component(struct simulator*, struct component*);
void simulator_process_message(char*);

#endif /* __simulator_h__ */
