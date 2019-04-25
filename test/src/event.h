#include <libwebsockets.h>
#include <string.h>

#ifndef __event_h__
#define __event_h__

#define EVENT_DESTINATION_MAX_LENGTH 32

enum event_types {
    LED_ON,
    LED_OFF
};

static const char * const reason_names[] = {
	"LEJPCB_CONSTRUCTED",
	"LEJPCB_DESTRUCTED",
	"LEJPCB_START",
	"LEJPCB_COMPLETE",
	"LEJPCB_FAILED",
	"LEJPCB_PAIR_NAME",
	"LEJPCB_VAL_TRUE",
	"LEJPCB_VAL_FALSE",
	"LEJPCB_VAL_NULL",
	"LEJPCB_VAL_NUM_INT",
	"LEJPCB_VAL_NUM_FLOAT",
	"LEJPCB_VAL_STR_START",
	"LEJPCB_VAL_STR_CHUNK",
	"LEJPCB_VAL_STR_END",
	"LEJPCB_ARRAY_START",
	"LEJPCB_ARRAY_END",
	"LEJPCB_OBJECT_START",
	"LEJPCB_OBJECT_END",
};


struct event {
    int value;
};

struct event_message {
    char destination[EVENT_DESTINATION_MAX_LENGTH];
    struct event event;
};

int event_parse(struct event_message*, char* buffer, int buffer_size);
void destroy_event_message( void* );


#endif /* __event_h__ */
