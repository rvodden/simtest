#undef LWS_OPENSSL_SUPPORT
#include <libwebsockets.h>

#ifndef __http_h__
#define __http_h__

#include "simulator.h"

struct per_session_data {
    char path[128];
    const char* message;
    size_t length;
};

struct lws_context* http_init( simulator_t* );
void http_destroy( struct lws_context* );

/* TODO: Find a way to dynamically generate this part */

extern const char* r_resources_app_js_begin;
extern const char* r_resources_app_js_end;
extern const int r_resources_app_js_size;

extern const char* r_resources_index_html_begin;
extern const char* r_resources_index_html_end;
extern const int r_resources_index_html_size;

extern const char* r_resources_style_css_begin;
extern const char* r_resources_style_css_end;
extern const int r_resources_style_css_size;

#endif // __http_h__
