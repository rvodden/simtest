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
};

struct simulator* simulator_init(void);
void simulator_destroy(struct simulator*);
void simulator_run(struct simulator*);
void simulator_terminate(struct simulator*);

#endif /* __simulator_h__ */
