#include "event.h"

static const char * const paths[] = {
	"dummy___"
};

struct lejp_ctx* event_parser_init(char* buffer, int buffer_size) {
    int fd, n, retval = 1, m;
	struct lejp_ctx ctx;

	lws_set_log_level(7, NULL);

	lwsl_notice("libwebsockets-test-lejp  (C) 2017 - 2018 andy@warmcat.com\n");
	lwsl_notice("  usage: cat my.json | libwebsockets-test-lejp\n\n");

	lejp_construct(&ctx, event_parser_callback, NULL, paths, LWS_ARRAY_SIZE(paths));

	fd = 0;

	while (n > 0) {
		n = read(fd, buffer, buffer_size);

		if (n <= 0)
			continue;

		m = lejp_parse(&ctx, (uint8_t *)buffer, n);

		if (m < 0 && m != LEJP_CONTINUE) {
			lwsl_err("parse failed %d\n", m);
			goto bail;
		}
	}
	lwsl_notice("okay\n");
	retval = 0;
bail:
	lejp_destruct(&ctx);

	return retval;
}


static signed char event_parser_callback(struct lejp_ctx *ctx, char reason) {
    char buffer[1024], *p = buffer, *end = &buffer[sizeof(buffer)];

	if (reason & LEJP_FLAG_CB_IS_VALUE) {
		p += lws_snprintf(p, p - end, "   value '%s' ", ctx->buf);
		if (ctx->ipos) {
			int n;

			p += lws_snprintf(p, p - end, "(array indexes: ");
			for (n = 0; n < ctx->ipos; n++)
				p += lws_snprintf(p, p - end, "%d ", ctx->i[n]);
			p += lws_snprintf(p, p - end, ") ");
		}
		lwsl_notice("%s (%s)\r\n", buffer,
		       reason_names[(unsigned int)
			(reason) & (LEJP_FLAG_CB_IS_VALUE - 1)]);

		(void)reason_names; /* NO_LOGS... */
		return 0;
	}

	switch (reason) {
	case LEJPCB_COMPLETE:
		lwsl_notice("Parsing Completed (LEJPCB_COMPLETE)\n");
		break;
	case LEJPCB_PAIR_NAME:
		lwsl_notice("path: '%s' (LEJPCB_PAIR_NAME)\n", ctx->path);
		break;
	}

	return 0;
}