#include <stdlib.h>

#include "simulator.h"
#include "websocket.h"
#include "http.h"

static void             sighandler(int);
static void*            avr_run_thread(void*);
static struct           lws_context *context;
static pthread_t        run;
static struct simulator *simulator;

static void* avr_run_thread(void *param) {
    struct simulator *simulator = (struct simulator*) param;
    simulator_run(simulator);

    return NULL;
}

static void sighandler(int sig)
{
    lwsl_debug("Recieved SIGINT: %d\n", sig);
    pthread_cancel(run);
    http_destroy(simulator->context);
    simulator_terminate(simulator);
    pthread_join(run, NULL);
    simulator_destroy(simulator);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");
    exit(1);
}

int main ( void ) 
{
    simulator = simulator_init();
    context = http_init(simulator); 
    
    if (!context) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    simulator->context = context;

    signal(SIGINT, sighandler);
    
    pthread_create(&run, NULL, avr_run_thread, simulator);
    
    while (!lws_service(context, 0));
    
    /* when we decided to exit the event loop */

	return 0;
}

