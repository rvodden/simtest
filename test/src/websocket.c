#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "websocket.h"
#include "event.h"
#include "simulator.h"

#define WEBSOCKET_QUEUE_LENGTH 32

/* queue free space below this, rx flow is disabled */
#define WEBSOCKET_RXFLOW_MIN (4)
/* queue free space above this, rx flow is enabled */
#define WEBSOCKET_RXFLOW_MAX ((2 * WEBSOCKET_QUEUE_LENGTH) / 3)

/* Data for an instance of the websocket protocol */
struct websocket_context_data {
    struct lws_context *context;
    struct lws_ring *output_ring;
    pthread_mutex_t* lock_output_ring;
    struct lws_ring *input_ring;
    pthread_mutex_t* lock_input_ring;

    struct websocket_per_session_data *psd_list;

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
}


void websocket_destroy_input_message(void *_msg)
{
    struct websocket_input_message *message = (struct websocket_input_message*) _msg;
    lejp_destruct(message->ctx);
    free(message->ctx);
}


int websocket_callback(struct lws *lwsi, enum lws_callback_reasons reason, void *user, void *in, size_t len)
{
    /* grab the per session data, and the per vhost data */
    struct websocket_per_session_data *session_data = ( struct websocket_per_session_data * ) user;
    struct lws_context* context = lws_get_context(lwsi);
    struct websocket_context_data *context_data = lws_context_user( context );

    switch (reason) {

        case LWS_CALLBACK_PROTOCOL_INIT:
            lwsl_info("Websocket Protocol Iniialized\n");

            /* Create per vhost data */
            if(!context_data) {
                lwsl_err("Cannot allocate context data");
                return 1;
            }

            context_data->context = context;

            break;

        case LWS_CALLBACK_PROTOCOL_DESTROY:
            break;

        case LWS_CALLBACK_ESTABLISHED:
            lwsl_info("New websocket protocol connection received\n");

            /* tell this session about our protocol instance, lws instance, and point at the tail of the buffer */
            session_data->lwsi = lwsi;
            session_data->tail = lws_ring_get_oldest_tail(context_data->output_ring);

            /* stick this session at the front of the list */
            session_data->next = context_data->psd_list;
            context_data->psd_list = session_data;
            break;

        case LWS_CALLBACK_CLOSED:
            /* remove session from the list */
            context_data->psd_list = delete_session(context_data->psd_list, session_data);
            break;

        case LWS_CALLBACK_SERVER_WRITEABLE:
            /* we'll get a callback with this code whenever there
             * is something to send */
            lwsl_notice("Sending a message\n");
            pthread_mutex_lock(context_data->lock_output_ring);

			/* grab the message which is at the tail position for this session */
			struct websocket_message* message = ( struct websocket_message* ) lws_ring_get_element( context_data->output_ring, &(session_data->tail) );

			/* break out if its null */
			if(!message) {
                lwsl_debug("No message to send\n");
                pthread_mutex_unlock(context_data->lock_output_ring);
                break;
            }

			/* copy it into a suitable place with preamble */
            /* TODO: refactor this so we don't have a malloc on every message send */
            int number_of_sent_characters;
			unsigned char* carrier = malloc(sizeof(struct websocket_message) + LWS_PRE + 1);
			unsigned char* carried_message = memcpy(carrier+LWS_PRE, message, sizeof(struct websocket_message));
			number_of_sent_characters = lws_write(lwsi,carried_message, message->length, LWS_WRITE_TEXT);
			free(carrier);

			if ( number_of_sent_characters < (int) message->length ) { /* fatal error - connection needs closing */
				lwsl_err("%s: Failed to write to websocket: %d\n", __func__, number_of_sent_characters);
                pthread_mutex_unlock(context_data->lock_output_ring);
				return -1;
			}

			lws_ring_consume_and_update_oldest_tail(
				context_data->output_ring,
				struct websocket_per_session_data,
				&session_data->tail,
				1,
				context_data->psd_list,
				tail,
				next
			);

            if(lws_ring_get_element( context_data->output_ring, &(session_data->tail))) {
                /* more to send */
                lws_callback_on_writable(session_data->lwsi);
            }
            pthread_mutex_unlock(context_data->lock_output_ring);
            break;

        case LWS_CALLBACK_EVENT_WAIT_CANCELLED:
            lwsl_debug("Woken up!\n");
            /* send a writable event on each session */
            struct websocket_per_session_data* session = context_data->psd_list;

            if(!session) {
                /* if there aren't any sessions, kill this message */
                lwsl_debug("No sessions!\n");
                pthread_mutex_lock(context_data->lock_output_ring);
			    lws_ring_consume(
				    context_data->output_ring,
                    NULL,
    				NULL,
                    1
    			);
                pthread_mutex_unlock(context_data->lock_output_ring);
            } else {
                while (session) {
                    lws_callback_on_writable(session->lwsi);
                    session = session->next;
                }
            }
            break;

        case LWS_CALLBACK_RECEIVE:
            lwsl_notice("Received: %.*s\n", (int) len, (char *)in);
            /* attempt to parse received object, check result and throw back appropriate response */
            struct event_message input_message;

            event_parse(&input_message, (char*) in, (int) len);

            lwsl_debug("Parsed message into: %i, %i\n", input_message.destination_id, input_message.event.value);
            
            pthread_mutex_lock(context_data->lock_input_ring);
            if(!lws_ring_get_count_free_elements(context_data->input_ring)) {
                lwsl_err("Dropping as no space in the input_ring buffer.\n");
                pthread_mutex_unlock(context_data->lock_input_ring);
                break;
            }

            lws_ring_insert(context_data->input_ring, &input_message, 1);
            pthread_mutex_unlock(context_data->lock_input_ring);
            break;

        default:
            break;

    }

    return 0;
}

struct websocket_context_data* create_websocket_context_data(simulator_t* simulator) {
    struct websocket_context_data* websocket_context_data = malloc(sizeof(struct websocket_context_data));
    memset(websocket_context_data, 0, sizeof(struct websocket_context_data));
    websocket_context_data->output_ring = get_simulator_output_ring(simulator);
    websocket_context_data->lock_output_ring = get_simulator_output_ring_lock(simulator);
    websocket_context_data->input_ring = get_simulator_input_ring(simulator);
    websocket_context_data->lock_input_ring = get_simulator_input_ring_lock(simulator);

    return websocket_context_data;
}

void destroy_websocket_context_data(struct websocket_context_data* context_data) {
    free(context_data);
}

/* PRIVATE IMPLEMENTATION */

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
