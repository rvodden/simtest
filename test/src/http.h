#undef LWS_OPENSSL_SUPPORT
#include <libwebsockets.h>
#include "simulator.h"

struct lws_context* http_init( struct simulator* );
void http_run ( struct lws_context* );
