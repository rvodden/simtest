#include "event.h"

static signed char  event_parser_callback   (struct lejp_ctx *ctx, char reason);

static const char * const paths_message[] = {
	"id",
    "message"
};

enum message_paths {
    MESSAGE_ID,
    MESSAGE,
};

void destroy_event_message(void* event_message) {

}

int event_parse(struct event_message* message, char* buffer, int buffer_size) {
    int retval = 1, m;
    struct lejp_ctx ctx;

	lwsl_notice("Starting the JSON Parser\n");
	lejp_construct(&ctx, event_parser_callback, message, paths_message, LWS_ARRAY_SIZE(paths_message));

	m = lejp_parse(&ctx, (uint8_t *)buffer, buffer_size);

	if (m < 0 && m != LEJP_CONTINUE) {
		lwsl_err("parse failed %d\n", m);
		goto bail;
	}
	
    lwsl_notice("okay\n");
	retval = 0;

bail:
	return retval;
}

static signed char event_parser_callback(struct lejp_ctx *ctx, char reason) {
    struct event_message* message = (struct event_message*) ctx->user;
	if (reason & LEJP_FLAG_CB_IS_VALUE) { /* is this a value we've seen */
        switch(ctx->path_match -1) {
            case MESSAGE_ID:
                lwsl_debug("Received the destination of the message.\n");
                 message->destination_id = atoi(ctx->buf);
            case MESSAGE:
                lwsl_debug("Received the message.\n");
                message->event.value = atoi(ctx->buf);
        }
	}

    return 0;
}
