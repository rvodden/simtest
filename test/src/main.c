#include <stdlib.h>

#include "simulator.h"
#include "websocket.h"
#include "http.h"

static void sighandler(int);

static void* avr_run_thread(void*);

static struct lws_context *context;

static void* avr_run_thread(void *param) {
    struct simulator *simulator = (struct simulator*) param;
    simulator_run(simulator);

    return NULL;
}

static int interrupted;

static void sighandler(int sig)
{
    interrupted = 1;
}

int main ( void ) 
{
    struct simulator *simulator;

    simulator = simulator_init();
    
    context = http_init(simulator); 
    
    if (!context) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    simulator->context = context;

    signal(SIGINT, sighandler);
    
    pthread_t run;
    pthread_create(&run, NULL, avr_run_thread, simulator);

    while (!interrupted) 
        if(lws_service(context, 1000))
            interrupted = 1;
	
    
    /* when we decided to exit the event loop */
    pthread_cancel(run);
	lws_context_destroy(context);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");

    simulator_destroy(simulator);

	return 0;
}

