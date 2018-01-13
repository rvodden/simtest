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
    char* payload;
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
struct websocket_per_vhost_data {
    struct lws_ring* ring;
    struct websocket_per_session_data* head;
};

int websocket_callback(struct lws *wsi, enum lws_callback_reasons reason, void *user, void *in, size_t len);
int websocket_init_protocol( struct lws_context*, struct lws_plugin_capability* );
int websocket_destroy_protocol( struct lws_context* );
int websocket_send_message( struct lws*, struct websocket_message* );
void websocket_destroy_message(void *_msg);

