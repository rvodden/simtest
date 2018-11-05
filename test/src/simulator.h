#include <sim_avr.h>
#undef ARRAY_SIZE
#include <libwebsockets.h>

#ifndef __simulator_h__
#define __simulator_h__

struct simulator {
    avr_t*           avr;
    struct lws_ring* ring;
    struct websocket_context_data* context_data;
    pthread_mutex_t  lock_ring;
};

struct simulator* simulator_init(void);
void simulator_destroy(struct simulator*);
void simulator_run(struct simulator*);

#endif /* __simulator_h__ */
