#define LWS_DLL
#define LWS_INTERNAL
#include <libwebsockets.h>
#include <pthread.h>

#define WEBSOCKET_MAX_INSTANCES 3

#define WEBSOCKET_PROTOCOL \
	{ \
		"websocket-protocol", \
		websocket_callback, \
		sizeof(struct websocket_per_session_data), \
		4096, /* rx buf size must be >= permessage-deflate rx size */ \
		0, NULL, 0 \
	}

struct websocket_instance;

struct websocket_message {
    char* payload[32];
    size_t length;
};

/* per session data - its a singly linked list so that we can 
 * traverse it in websocket send message */
struct websocket_per_session_data {
    struct lws *lwsi; /* link to libwebsockets instance */
    struct websocket_instance* ws; /* link to websocket protocol instance */
    uint32_t tail; /* tail pointer in the ring buffer for this session */

    struct websocket_per_session_data* next;
};

/* per vshost data - in this case a linked list of websocket instances */
struct websocket_context_data {
    struct lws_context* context;
    struct lws_ring* ring;
    struct websocket_per_session_data* head; /* linked list of sessions */
};

int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
int websocket_destroy_protocol( struct lws_context* );
void websocket_callback_all_in_context_on_writeable( struct websocket_context_data* context_data );
void websocket_destroy_message(void *_msg);

