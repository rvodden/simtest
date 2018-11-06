#include <stdlib.h>
#include <stdint.h>
#include <libwebsockets.h>

#include "http.h"
#include "websocket.h"

#define CONTEXT_PATH "/"

static int http_callback(struct lws*, enum lws_callback_reasons, void*, void*, size_t);

static const struct lws_protocols protocols[] = {
    {
        "http-only",   // name
        http_callback, // callback
        sizeof(struct per_session_data),              // per_session_data_size
        0, 
        0, NULL, 0
    },
    WEBSOCKET_PROTOCOL,
    {NULL,NULL, 0, 0, 0, NULL, 0}
};

static const struct lws_extension exts[] = {
    { NULL, NULL, NULL /* terminator */ }
};

static const struct lws_http_mount mount = {
    NULL,
    CONTEXT_PATH,
    NULL,
    "index.html",
    "http-only",
    NULL,
    NULL,
    NULL,
    0,
    0,
    0,
    0,
    0,
    0,
    LWSMPRO_CALLBACK,
    1,
    NULL
};

static int http_callback(struct lws *wsi, enum lws_callback_reasons reason,
		    void *user, void *in, size_t len)
{
    struct per_session_data *per_session_data = (struct per_session_data *)user;
    uint8_t buffer[LWS_PRE + 2048];
    uint8_t *start = &buffer[LWS_PRE];
    uint8_t *p = start;
    uint8_t *end = &buffer[sizeof(buffer) - LWS_PRE - 1];

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

        case LWS_CALLBACK_HTTP:
            lwsl_debug("Received HTTP Callback\n");
            lws_snprintf(per_session_data->path, sizeof(per_session_data->path), "%s", (const char*) in);
            if(lws_add_http_common_headers(wsi, HTTP_STATUS_OK, "text/html", LWS_ILLEGAL_HTTP_CONTENT_LEN, &p, end))
                return 1;

            if(lws_finalize_write_http_header(wsi, start, &p, end))
                return 1;

            lws_callback_on_writable(wsi);

            return 0;

        case LWS_CALLBACK_HTTP_WRITEABLE:
            lwsl_debug("Received HTTP Callback writable\n");
            if (!per_session_data)
                break;
            
            uint16_t _binary_resources_index_html_size = _binary_resources_index_html_end - _binary_resources_index_html_start;          
            lwsl_debug("Writing %d bytes of data", _binary_resources_index_html_size);
            
            memcpy(p, _binary_resources_index_html_start, _binary_resources_index_html_size);
            
            p += _binary_resources_index_html_size;
            
            if(lws_write(wsi, start, lws_ptr_diff(p, start), LWS_WRITE_HTTP_FINAL) != lws_ptr_diff(p,start)) {
                lwsl_debug("Could not write");
                return 1;
            }

            if(lws_http_transaction_completed(wsi)) {
                lwsl_debug("Transaction must close now");
                return -1;
            }

            return 0;
        default:
            break;
    }

    return lws_callback_http_dummy(wsi, reason, user, in, len);
}

struct lws_context* http_init( struct websocket_context_data* context_data ) {
    int uid = -1, gid = -1;
    int opts = 0;

    struct lws_context_creation_info info;
    struct lws_context *context;

    memset(&info, 0, sizeof info);
    info.port = 7681;

    lws_set_log_level(
            LLL_ERR |
            LLL_WARN |
            LLL_NOTICE |
            LLL_INFO |
            LLL_DEBUG
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
    info.mounts = &mount;

    info.user = context_data;

    info.extensions = exts;
    info.timeout_secs = 5;

    info.protocols = protocols;

    context = lws_create_context(&info);
    context_data->context = context;
    return context;
}

