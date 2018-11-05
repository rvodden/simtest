#include <stdlib.h>

#include "simulator.h"
#include "websocket.h"
#include "http.h"

void sighandler(int);

static void signal_cb(void*, int);
static void* avr_run_thread(void*);

static struct lws_context *context;

static void* avr_run_thread(void *param) {
    struct simulator *simulator = (struct simulator*) param;
    simulator_run(simulator);

    return NULL;
}

void sighandler(int sig)
{
	signal_cb(NULL, sig);
}

static void signal_cb(void *handle, int signum)
{
    lwsl_err("Signal %d caught, exiting...\n", signum);
    switch (signum) {
        case SIGTERM:
        case SIGINT:
            break;
        default:

            break;
    }
    lws_context_destroy(context);
}

int main ( void ) 
{
    struct simulator *simulator;
    struct websocket_context_data* context_data;

    simulator = simulator_init();
    
    context_data = malloc(sizeof(struct websocket_context_data));
    memset(context_data, 0, sizeof(struct websocket_context_data));
    context_data->ring = simulator->ring;

    context = http_init(context_data); 
    
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    simulator->context_data = context_data;

    signal(SIGINT, sighandler);
    
    pthread_t run;
    pthread_create(&run, NULL, avr_run_thread, simulator);


    while (!lws_service(context, 0));
	
    pthread_cancel(run);
    
    /* when we decided to exit the event loop */
	lws_context_destroy(context);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");

    free(context_data);
    simulator_destroy(simulator);

	return 0;
}

