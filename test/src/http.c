#include <stdlib.h>
#include <stdint.h>
#include <libwebsockets.h>

#include "http.h"
#include "websocket.h"

#define LOCAL_RESOURCE_PATH "/Users/voddenr/eclipse-workspace/simtest/test/resources/"
#define CONTEXT_PATH "/"
char *resource_path = LOCAL_RESOURCE_PATH;

static const struct lws_http_mount mount = {
    NULL,	/* linked-list pointer to next*/
    CONTEXT_PATH,		/* mountpoint in URL namespace on this vhost */
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
    strlen(CONTEXT_PATH),		/* strlen("/"), ie length of the mountpoint */
    NULL,

    { NULL, NULL } // sentinel
};

static const struct lws_protocols protocols[] = {
    {
        "http-only",   // name
        lws_callback_http_dummy, // callback
        0,              // per_session_data_size
        0, 
        0, NULL, 0
    },
    WEBSOCKET_PROTOCOL,
    {NULL,NULL, 0, 0, 0, NULL, 0}
};

static const struct lws_extension exts[] = {
    { NULL, NULL, NULL /* terminator */ }
};

static int http_callback(struct lws *wsi, enum lws_callback_reasons reason,
		    void *user, void *in, size_t len)
{
#ifdef __APPLE__
    uint64_t tid;
#endif    
    
    switch ( reason ) {
        case LWS_CALLBACK_GET_THREAD_ID:
#ifdef __APPLE__        
            pthread_threadid_np(NULL, &tid);
            return tid;
#else
            return pthread_self();
#endif
        default:
            return lws_callback_http_dummy(wsi, reason, user, in, len);
    }
}

struct lws_context* http_init( struct simulator* simulator  ) {
    int uid = -1, gid = -1;
    int opts = 0;

    struct lws_context_creation_info info;
    struct lws_context *context;

    struct websocket_per_vhost_data *wpvd;

    memset(&info, 0, sizeof info);
    info.port = 7681;

    lws_set_log_level(
            LLL_ERR |
            LLL_WARN |
            LLL_NOTICE |
            LLL_INFO 
  //          LLL_DEBUG |
  //          LLL_PARSER |
  //          LLL_HEADER |
  //          LLL_EXT |
  //          LLL_CLIENT |
  //          LLL_LATENCY |
  //          LLL_USER
            , NULL); /* debug */

    lwsl_notice("Richard's Simulator\n");

    info.max_http_header_pool = 16;
    info.options = opts |
        LWS_SERVER_OPTION_VALIDATE_UTF8 |
        LWS_SERVER_OPTION_LIBUV;

    info.gid = gid;
    info.uid = uid;

    wpvd = malloc(sizeof(struct websocket_per_vhost_data));
    memset(wpvd, 0, sizeof(struct websocket_per_vhost_data));
    wpvd->ring = simulator->ring;

    info.user = wpvd;

    info.extensions = exts;
    info.timeout_secs = 5;

    /* tell lws about our mount we want */
    info.mounts = &mount;
    info.protocols = protocols;

    context = lws_create_context(&info);
    return context;
} 
