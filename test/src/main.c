#include <stdlib.h>

#include "simulator.h"

#undef LWS_OPENSSL_SUPPORT
#include <libwebsockets.h>

struct lws_context_creation_info info;
struct lws_context *context;

void sighandler( int );
static void signal_cb(uv_signal_t *watcher, int signum);

#define LOCAL_RESOURCE_PATH "/Users/voddenr/eclipse-workspace/simtest/test/resources/"
char *resource_path = LOCAL_RESOURCE_PATH;

void sighandler(int sig)
{
    fprintf(stderr, "Caught signal: %i\n", sig);
    lws_cancel_service(context);
}

static const struct lws_http_mount mount = {
    NULL,	/* linked-list pointer to next*/
    "/",		/* mountpoint in URL namespace on this vhost */
    LOCAL_RESOURCE_PATH, /* where to go on the filesystem for that */
    "index.html",	/* default filename if none given */
    NULL,
    NULL,
    NULL,
    NULL,
    0,
    0,
    0,
    0,
    0,
    0,
    LWSMPRO_FILE,	/* mount type is a directory in a filesystem */
    1,		/* strlen("/"), ie length of the mountpoint */
    NULL,

    { NULL, NULL } // sentinel
};

static const struct lws_extension exts[] = {
	{ NULL, NULL, NULL /* terminator */ }
};

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
    avr_t* avr;

    int uid = -1, gid = -1;
    int opts = 0;

    memset(&info, 0, sizeof info);
    info.port = 7681;

    signal(SIGINT, sighandler);
    lws_set_log_level(7, NULL);

    lwsl_notice("Richard's Simulator");

    info.max_http_header_pool = 16;
    info.options = opts | 
        LWS_SERVER_OPTION_FALLBACK_TO_RAW |
        LWS_SERVER_OPTION_VALIDATE_UTF8 |
        LWS_SERVER_OPTION_LIBUV;

    info.gid = gid;
    info.uid = uid;

    info.extensions = exts;
    info.timeout_secs = 5;

    /* tell lws about our mount we want */
    info.mounts = &mount;

    context = lws_create_context(&info);
    if (context == NULL) {
        lwsl_err("libwebsocket init failed\n");
        return -1;
    }

    /* libuv event loop */
	lws_uv_sigint_cfg(context, 1, signal_cb);
	if (lws_uv_initloop(context, NULL, 0)) {
		lwsl_err("lws_uv_initloop failed\n");
		goto bail;
	}

    lws_libuv_run(context, 0);

    bail:
	/* when we decided to exit the event loop */
	lws_context_destroy(context);
	lws_context_destroy2(context);
	lwsl_notice("libwebsockets-test-server exited cleanly\n");

	return 0;

    /* Initialise the avr simulator */
    avr = simulator_init();
    // simulator_run(avr);

    return 0;
}
