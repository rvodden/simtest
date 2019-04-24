#include <libwebsockets.h>
#include <string.h>

#ifndef __event_h__
#define __event_h__

enum event_types {
    LED_ON,
    LED_OFF
};

struct lejp_ctx*    event_parser_init       (char* buffer, int buffer_size);
static signed char  event_parser_callback   (struct lejp_ctx *ctx, char reason);

typedef struct event_t {
    char* event_name;
} event_t;

#endif /* __event_h__ */