#ifndef WEBSOCKET_H
#define WEBSOCKET_H 

#define LWS_DLL
#define LWS_INTERNAL

#include <libwebsockets.h>
#include "simulator.h"

#define WEBSOCKET_MAX_INSTANCES 3

#define WEBSOCKET_PROTOCOL \
	{ \
		"websocket-protocol", \
		websocket_callback, \
		sizeof(struct websocket_per_session_data), \
		4096, /* rx buf size must be >= permessage-deflate rx size */ \
		0, NULL, 0 \
	}

struct websocket_context_data;

/* has to be defined here, rather than in .c like context_data as
 * we need to tell WEBSOCKET_PROTOCOL about the size of it */
struct websocket_per_session_data {
    struct lws *lwsi; /* link to libwebsockets instance */
    uint32_t tail; /* tail pointer in the ring buffer for this session */

    struct websocket_per_session_data* next;
};


struct websocket_message {
    char payload[32];
    size_t length;
};

int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
int websocket_destroy_protocol( struct lws_context* );
void websocket_callback_all_in_context_on_writeable( struct lws_context* );
void websocket_destroy_message(void *_msg);

struct websocket_context_data* create_websocket_context_data(struct simulator* simulator);

#endif  /* WEBSOCKET_H */
