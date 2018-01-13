#include <stdlib.h>

#include "simulator.h"
#include "http.h"

void sighandler(int sig);

static void signal_cb(uv_signal_t *watcher, int signum);
static void* avr_run_thread(void *param);

static struct lws_context *context;
static struct simulator *global_simulator;

static void* avr_run_thread(void *param) {
    struct simulator *simulator = (struct simulator*) param;
    simulator_run(simulator);

    return NULL;
}

void sighandler(int sig)
{
    fprintf(stderr, "Caught signal: %i\n", sig);
    lws_cancel_service(context);
}

static void signal_cb(uv_signal_t *watcher, int signum)
{
    lwsl_err("Signal %d caught, exiting...\n", signum);
    switch (watcher->signum) {
        case SIGTERM:
        case SIGINT:
            break;
        default:
            signal(SIGABRT, SIG_DFL);
            abort();
            break;
    }
    lws_libuv_stop(context);
}

int main ( void ) 
{
    struct simulator *simulator;

    simulator = simulator_init();
    global_simulator = simulator;
    context = http_init(simulator); 
    
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }
    
    signal(SIGINT, sighandler);
    
    pthread_t run;
    pthread_create(&run, NULL, avr_run_thread, simulator);

    /* libuv event loop */
	lws_uv_sigint_cfg(context, 1, signal_cb);
	if (lws_uv_initloop(context, NULL, 0)) {
		lwsl_err("lws_uv_initloop failed\n");
	} else {
        lws_libuv_run(context, 0);
    }
	
    pthread_cancel(run);
    
    /* when we decided to exit the event loop */
	lws_context_destroy(context);
	lws_context_destroy2(context);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");

	return 0;
}

