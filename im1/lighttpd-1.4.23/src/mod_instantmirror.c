#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "base.h"
#include "log.h"
#include "buffer.h"

#include "plugin.h"

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#define SOCK_PATH "../../socketfd"

/**
 * this is a instantmirror for a lighttpd plugin
 *
 *
 */


/* plugin config for all request/connections */

typedef struct {
	array *match;
} plugin_config;

typedef struct {
	PLUGIN_DATA;

	buffer *match_buf;

	plugin_config **config_storage;

	plugin_config conf;
} plugin_data;

typedef struct {
	size_t foo;
} handler_ctx;

static handler_ctx * handler_ctx_init() {
	handler_ctx * hctx;

	hctx = calloc(1, sizeof(*hctx));

	return hctx;
}

static void handler_ctx_free(handler_ctx *hctx) {

	free(hctx);
}

/* init the plugin data */
INIT_FUNC(mod_instantmirror_init) {
	plugin_data *p;

	p = calloc(1, sizeof(*p));

	p->match_buf = buffer_init();

	return p;
}

/* detroy the plugin data */
FREE_FUNC(mod_instantmirror_free) {
	plugin_data *p = p_d;

	UNUSED(srv);

	if (!p) return HANDLER_GO_ON;

	if (p->config_storage) {
		size_t i;

		for (i = 0; i < srv->config_context->used; i++) {
			plugin_config *s = p->config_storage[i];

			if (!s) continue;

			array_free(s->match);

			free(s);
		}
		free(p->config_storage);
	}

	buffer_free(p->match_buf);

	free(p);

	return HANDLER_GO_ON;
}

/* handle plugin config and check values */

SETDEFAULTS_FUNC(mod_instantmirror_set_defaults) {
	plugin_data *p = p_d;
	size_t i = 0;

	config_values_t cv[] = {
		{ "instantmirror.array",             NULL, T_CONFIG_ARRAY, T_CONFIG_SCOPE_CONNECTION },       /* 0 */
		{ NULL,                         NULL, T_CONFIG_UNSET, T_CONFIG_SCOPE_UNSET }
	};

	if (!p) return HANDLER_ERROR;

	p->config_storage = calloc(1, srv->config_context->used * sizeof(specific_config *));

	for (i = 0; i < srv->config_context->used; i++) {
		plugin_config *s;

		s = calloc(1, sizeof(plugin_config));
		s->match    = array_init();

		cv[0].destination = s->match;

		p->config_storage[i] = s;

		if (0 != config_insert_values_global(srv, ((data_config *)srv->config_context->data[i])->value, cv)) {
			return HANDLER_ERROR;
		}
	}

	return HANDLER_GO_ON;
}

#define PATCH(x) \
	p->conf.x = s->x;
static int mod_instantmirror_patch_connection(server *srv, connection *con, plugin_data *p) {
	size_t i, j;
	plugin_config *s = p->config_storage[0];

	PATCH(match);

	/* skip the first, the global context */
	for (i = 1; i < srv->config_context->used; i++) {
		data_config *dc = (data_config *)srv->config_context->data[i];
		s = p->config_storage[i];

		/* condition didn't match */
		if (!config_check_cond(srv, con, dc)) continue;

		/* merge config */
		for (j = 0; j < dc->value->used; j++) {
			data_unset *du = dc->value->data[j];

			if (buffer_is_equal_string(du->key, CONST_STR_LEN("instantmirror.array"))) {
				PATCH(match);
			}
		}
	}

	return 0;
}
#undef PATCH

URIHANDLER_FUNC(mod_instantmirror_uri_handler) {
//	plugin_data *p = p_d;
//	int s_len;
//	size_t k, i;
	struct sockaddr_un remote;
	int s,t,len;

	UNUSED(srv);

	log_error_write(srv, __FILE__, __LINE__,  "s s", con->uri.path->ptr,  "-- handling file as instantmirror file");

	if ((s = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) 
	{
		perror("socket");
		exit(1);
	}
	
	remote.sun_family = AF_UNIX;
	strcpy(remote.sun_path, SOCK_PATH);
	len = strlen(remote.sun_path) + sizeof(remote.sun_family);
	if (connect(s, (struct sockaddr *)&remote, len) == -1)
	{
		perror("connect");
		exit(1);
	}
	log_error_write(srv, __FILE__, __LINE__,  "s s", con->uri.path->ptr,  "-- handling file as instantmirror file CONNECTED");

	if (send(s, con->uri.path->ptr, strlen(con->uri.path->ptr), 0) == -1)
	{
		perror("send");
		exit(1);
	}
	close(s);
	
	/*if (con->mode != DIRECT) return HANDLER_GO_ON;

	if (con->uri.path->used == 0) return HANDLER_GO_ON;

	mod_instantmirror_patch_connection(srv, con, p);

	s_len = con->uri.path->used - 1;

	for (k = 0; k < p->conf.match->used; k++) {
		data_string *ds = (data_string *)p->conf.match->data[k];
		int ct_len = ds->value->used - 1;

		if (ct_len > s_len) continue;
		if (ds->value->used == 0) continue;

		if (0 == strncmp(con->uri.path->ptr + s_len - ct_len, ds->value->ptr, ct_len)) {
			con->http_status = 403;

			return HANDLER_FINISHED;
		}
	}

return HANDLER_FINISHED;*/
	/* not found */
	return HANDLER_GO_ON;
}

/* this function is called at dlopen() time and inits the callbacks */

int mod_instantmirror_plugin_init(plugin *p) {
	p->version     = LIGHTTPD_VERSION_ID;
	p->name        = buffer_init_string("instantmirror");

	p->init        = mod_instantmirror_init;
	p->handle_uri_clean  = mod_instantmirror_uri_handler;
	p->set_defaults  = mod_instantmirror_set_defaults;
	p->cleanup     = mod_instantmirror_free;

	p->data        = NULL;

	return 0;
}
