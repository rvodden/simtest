#undef LWS_OPENSSL_SUPPORT
#include <libwebsockets.h>
#include "simulator.h"

struct per_session_data {
    char path[128];
};

struct lws_context* http_init( struct websocket_context_data* );

/* TODO: Find a way to dynamically generate this part */

extern const char _binary_resources_index_html_start[];
extern const char _binary_resources_index_html_end[];


/*
extern const char* asm("_binary_resources_style_css_start");
extern const char* asm("_binary_resources_style_css_end");
extern const char* asm("_binary_resources_style_css_start");

extern const char* asm("_binary_resources_app_js_start");
extern const char* asm("_binary_resources_app_js_end");
extern const char* asm("_binary_resources_app_js_size");
*/
