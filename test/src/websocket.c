#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "websocket.h"
#include "simulator.h"

static int num_wsi = 0;
static char* message_buffer;

#define WEBSOCKET_QUEUE_LENGTH 32

/* queue free space below this, rx flow is disabled */
#define WEBSOCKET_RXFLOW_MIN (4)
/* queue free space above this, rx flow is enabled */
#define WEBSOCKET_RXFLOW_MAX ((2 * WEBSOCKET_QUEUE_LENGTH) / 3)

/* Data for an instance of the websocket protocol */
struct websocket_instance {
    struct lws_ring *ring;
    pthread_mutex_t* lock_ring;
    int message_allocated;
    char rx_enabled;
};

/* PRIVATE PROTOTYPES */
static struct websocket_per_session_data* delete_session(
        struct websocket_per_session_data*, 
        struct websocket_per_session_data* 
);

/* PUBLIC IMPLEMENTATION*/

void websocket_destroy_message(void *_msg)
{
	struct websocket_message *msg = _msg;
    // free(msg);
}

static struct websocket_context_data* get_vhost_data( struct lws* lwsi ) {
    return ( struct websocket_context_data   * ) lws_protocol_vh_priv_get (
                lws_get_vhost(lwsi),
                lws_get_protocol(lwsi)
                );
}

void websocket_callback_all_in_context_on_writeable( struct websocket_context_data* context_data ) {

    lwsl_info("Send message\n");
    struct websocket_per_session_data* session_data = context_data->head;
    while (session_data) {
        lwsl_info("Sending to: %p\n", session_data->lwsi);
        lws_cancel_service(context_data->context);
        session_data = session_data->next;
    }
}

int websocket_callback(struct lws *lwsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    /* grab the per session data, and the per vhost data */
    struct websocket_per_session_data *session_data = ( struct websocket_per_session_data * ) user;
    struct websocket_context_data *context_data = lws_context_user( lws_get_context(lwsi) );

    struct websocket_instance *ws = lws_get_protocol(lwsi);
    
    switch (reason) {

        case LWS_CALLBACK_PROTOCOL_INIT:
            lwsl_info("Websocket Protocol Iniialized\n");
            
            /* Create per vhost data */
            if(!context_data) {
                lwsl_err("Cannot allocate context data");
                return 1;
            }  
            break;
            
        case LWS_CALLBACK_PROTOCOL_DESTROY:
            free(ws);
            break;

        case LWS_CALLBACK_ESTABLISHED:
            lwsl_info("New websocket protocol connection received\n");
            
            /* tell this session about our protocol instance, lws instance, and point at the tail of the buffer */
            session_data->ws = ws;
            session_data->ws->ring = context_data->ring;
            session_data->lwsi = lwsi;
            session_data->tail = lws_ring_get_oldest_tail(context_data->ring);

            /* stick this session at the front of the list */
            session_data->next = context_data->head;
            context_data->head = session_data;
            break;

        case LWS_CALLBACK_CLOSED:
            /* remove session from the list */
            context_data->head = delete_session(context_data->head, session_data);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            /* we'll get a callback with this code whenever there
             * is something to send */
            lwsl_notice("Sending a message\n");
            
            uint32_t oldest_tail = lws_ring_get_oldest_tail(ws->ring);
            struct websocket_message* message;
            int n;

			/* grab the message which is at the tail position for this session */
			message = ( struct websocket_message* ) lws_ring_get_element( ws->ring, &(session_data->tail) );

			/* break out if its null */
			if(!message) {
                lwsl_debug("No message to send\n");
                break;
            }
			lwsl_info("Message is: %s\n", message);

			/* copy it into a suitable place with preamble */
			unsigned char* carrier = malloc(sizeof(struct websocket_message) + LWS_PRE + 1);
			unsigned char* carried_message = memcpy(carrier+LWS_PRE, message, sizeof(struct websocket_message));

			n = lws_write(lwsi,carried_message, message->length, LWS_WRITE_TEXT);

			if ( n < (int) message->length ) { /* fatal error - connection needs closing */
				lwsl_err("%s: Failed to write to websocket: %d\n", __func__, n);
				return -1;
			}

			lws_ring_consume_and_update_oldest_tail(
				ws->ring,
				struct websocket_per_session_data,
				&session_data->tail,
				1,
				context_data->head,
				tail,
				next
			);

			free(carrier);
            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            lwsl_debug("Woken up!\n");
            if(session_data)
                lws_callback_on_writable(session_data->lwsi);
            break;

        case LWS_CALLBACK_RECEIVE:
            lwsl_notice("Received: %s\n", in);
            break;

        default:
            break;

    }

    return 0;
}
/*
int websocket_send_message( struct lws* lwsi, struct websocket_message* message ) {
    struct websocket_per_vhost_data   *wpvd = 
        ( struct websocket_per_vhost_data   * ) lws_protocol_vh_priv_get (
                lws_get_vhost(lwsi),
                lws_get_protocol(lwsi)
                );
    
    struct lws_ring* ring = wpvd->ring;
    
    pthread_mutex_lock(&wpvd->lock_ring);
    int n = (int) lws_ring_get_count_free_elements(ring);

    if (!n) {
        lwsl_notice("Ring full, dropping message");
        return -1;
    }
    
    lws_ring_insert( ring, message, 1 );

    pthread_mutex_unlock(&wpvd->ws->lock_ring);
}
*/
/* This little lot is only used if the protocol is linked dynamically */

/* List of protocols supported by this plugin */
static const struct lws_protocols protocols[] = {
	WEBSOCKET_PROTOCOL
};

/*
int websocket_init_protocol( struct lws_context *context, struct lws_plugin_capability *capability) {    
    if (capability->api_magic != LWS_PLUGIN_API_MAGIC) {
		lwsl_err("Plugin API %d, library API %d", LWS_PLUGIN_API_MAGIC,
			 capability->api_magic);
		return 1;
	}
   
    capability->protocols = protocols;
    capability->count_protocols = 1;
    capability->extensions = NULL;
    capability->count_extensions = 0;
    
    return 0;
}
*/

int websocket_destroy_protocol( struct lws_context *context ) {

    return 0;
}

/* PRIVATE IMPLEMENATION */

/* recursive function to delete a session from the session linked list */
static struct websocket_per_session_data* delete_session(
        struct websocket_per_session_data* head, 
        struct websocket_per_session_data* x) {
    struct websocket_per_session_data* next;
    if ( head == NULL) { // Found the tail
        return NULL;
    } else if (head == x) { // Found one to delete
        next = head->next;
        return next;
    } else { // Just keep going
        head->next = delete_session(head->next, x);
        return head;
    }
}
