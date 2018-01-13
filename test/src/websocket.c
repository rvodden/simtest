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
    struct websocket_per_session_data *head; /* linked list of sessions */
};


void websocket_destroy_message(void *_msg)
{
	struct websocket_message *msg = _msg;

	free(msg->payload);
	msg->payload = NULL;
	msg->length = 0;
    free(msg);
}

static struct websocket_per_vhost_data* get_vhost_data( struct lws* lwsi ) {
    return ( struct websocket_per_vhost_data   * ) lws_protocol_vh_priv_get (
                lws_get_vhost(lwsi),
                lws_get_protocol(lwsi)
                );
}

static void calback_all_in_instance_on_writeable( struct websocket_instance* wi ) {
    struct websocket_per_session_data* current = wi->head;
    while(current) {
        lws_callback_on_writable(current->lwsi);
    }
}

int websocket_callback(struct lws *lwsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    /* grab the per session data, and the per vhost data */
    struct websocket_per_session_data *session_data = ( struct websocket_per_session_data * ) user;
    struct websocket_per_vhost_data   *context_data = lws_context_user( lws_get_context(lwsi) );

    struct websocket_instance *ws;
    
    if (session_data)
        ws = session_data->ws;
    else
        ws = NULL;

    switch (reason) {

        case LWS_CALLBACK_PROTOCOL_INIT:
            lws_protocol_vh_priv_zalloc(
                lws_get_vhost(lwsi),
				lws_get_protocol(lwsi),
				sizeof(struct websocket_per_vhost_data)
            );
            lwsl_info("Websocket Protocol Iniialized\n");
            break;

        case LWS_CALLBACK_ESTABLISHED:
            lwsl_info("New websocket protocol connection recieved\n");
            
            if ( !ws ) { /* there's no created instance attached to this vhost */
                ws = malloc(sizeof(*ws));
                if (!ws) return 1;
                memset(ws, 0, sizeof(*ws));
            }

            /* tell this session about our protocol instance, lws instance, and point at the tail of the buffer */
            session_data->ws = ws;
            session_data->lwsi = lwsi;
            session_data->tail = lws_ring_get_oldest_tail(context_data->ring);

            /* stick this session at the front of the list */

            session_data->next = ws->head;
            ws->head = session_data;

            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            /* we'll get a callback with this code whenever there
             * is something to send */
            lwsl_notice("Sending a message\n");
            
            uint32_t oldest_tail = lws_ring_get_oldest_tail(ws->ring);
            struct websocket_message* message;
            int n;

            /* loop around until the send pipe is chocked 
             * if there's nothing send we'll break out */
            do {
                /* grab the message which is at the tail position for this session */
                message = ( struct websocket_message* ) lws_ring_get_element( ws->ring, &session_data->tail );
                
                /* break out if its null */
                if(!message) break;

                /* check for a payload */
                if( message->payload ) {
                    n = lws_write(lwsi,(unsigned char*) message->payload + LWS_PRE, message->length, LWS_WRITE_TEXT);

                    if ( n < 0 ) { /* fatal error - connection needs closing */
                        lwsl_info("%s: WRITEABLE: %d\n", __func__, n);
                        return -1;
                    }

                } else { /* null payload */
                    lwsl_err("%s: NULL payload: worst = %d, pss->tail = %d\n", __func__, oldest_tail, session_data->tail);
                }
                
                if(!lws_ring_consume(ws->ring, &session_data->tail, NULL, 1))
                    break;
            } while (!lws_send_pipe_choked(lwsi));
            
            break;

        case LWS_CALLBACK_RECEIVE:
            lwsl_notice("Received: %s\n", in);
            break;

        case LWS_CALLBACK_CLOSED:
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

int websocket_destroy_protocol( struct lws_context *context ) {

    return 0;
}

