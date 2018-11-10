#undef LWS_OPENSSL_SUPPORT
#include <libwebsockets.h>
#include "simulator.h"

struct per_session_data {
    char path[128];
    char* message;
    size_t length;
};

struct lws_context* http_init( struct simulator* );

/* TODO: Find a way to dynamically generate this part */

extern const char _binary_resources_index_html_start[];
extern const char _binary_resources_index_html_end[];
extern const int  _binary_resources_index_html_size[];

extern const char _binary_resources_style_css_start[];
extern const char _binary_resources_style_css_end[];
extern const int _binary_resources_style_css_size[];

extern const char _binary_resources_app_js_start[];
extern const char _binary_resources_app_js_end[];
extern const int _binary_resources_app_js_size[];
