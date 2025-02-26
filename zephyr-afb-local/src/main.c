/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>

#include "zafb-helpers.h"

#include <sys/rp-verbose.h>
#include <afb-v4.h>

static void hello_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params);
static int tuto_mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata);

static const afb_verb_t tuto_desc_verbs[] = {
	{ .verb="hello", .callback=hello_verb_cb },
	{ .verb=NULL }
};

static const afb_binding_t tuto0_desc_api = {
	.api = "tuto0",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

static const afb_binding_t tuto1_desc_api = {
	.api = "tuto1",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

static afb_api_t tuto0, tuto1;

/* API TUTO SECTION - START */
/* hello verb callback */
static void hello_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int retcod;
	const char *who = "World"; /* default value */
	char hello[100];
	afb_data_t reply, name;

	/* some output for tracking the process */
	AFB_REQ_DEBUG(req, "Hi debugger!");

	/* extract the name of the caller */
	retcod = afb_req_param_convert(req, 0, AFB_PREDEFINED_TYPE_STRINGZ, &name);
	who = retcod == 0 ? afb_data_ro_pointer(name) : "World" /* default value */;
	AFB_REQ_NOTICE(req, "Hi observer! I'm %s and I received hello from %s", afb_req_get_called_api(req), who);

	/* create the reply string */
	retcod = snprintf(hello, sizeof hello, "Hello %s!", who);
	if (retcod >= (int)sizeof hello) {
		AFB_REQ_WARNING(req, "name too long, truncated!");
		retcod = (int)sizeof hello - 1;
		hello[retcod] = 0;
	}

	/* make the reply string */
	afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_STRINGZ, hello, (size_t)(retcod + 1 /*with last zero*/));
	afb_req_reply(req, 0, 1, &reply);
}

static int tuto_mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
	switch (ctlid) {
	case afb_ctlid_Pre_Init:
		AFB_API_NOTICE(api, "hello binding comes to live with uid %s", ctlarg->pre_init.uid);
		break;

	case afb_ctlid_Init:
		AFB_API_NOTICE(api, "hello binding starting");
		break;

	default:
		break;
	}
	return 0;
}

static void onrep(
	void *closure,
	int status,
	unsigned nreplies,
	afb_data_t const replies[],
	afb_api_t api
) {
	int rc;
	unsigned idx;
	afb_data_t str;

	AFB_API_NOTICE(api, "reply received");

	if (AFB_IS_ERRNO(status))
		AFB_API_NOTICE(api, "got error %d", status);

	for (idx = 0 ; idx < nreplies ; idx++) {
		rc = afb_data_convert(replies[idx], AFB_PREDEFINED_TYPE_STRINGZ, &str);
		if (rc >= 0) {
			AFB_API_NOTICE(api, "got data %u as string %s", idx, (const char *)afb_data_ro_pointer(str));
			afb_data_unref(str);
		}
		else
			AFB_API_NOTICE(api, "got data %u of type %s", idx, afb_type_name(afb_data_type(str)));
	}

	AFB_API_NOTICE(api, "leaving");
}

void start(int signum, void* arg)
{
	RP_INFO("Adding APIs ...");
	zafb_binding_add(&tuto0, &tuto0_desc_api);
	zafb_binding_add(&tuto1, &tuto1_desc_api);

	RP_INFO("Starting APIs ...");
	zafb_start();

	afb_data_t reply;

	RP_INFO("API tuto0 call API tuto1 hello verb with arg \"zephyr\"");
	afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "zephyr", 7, NULL, NULL);
	afb_api_call(tuto1, "tuto0", "hello", 1, &reply, onrep, NULL);

	k_sleep(K_MSEC(1000));

	RP_INFO("API tuto1 call API tuto0 hello verb with arg \"zephyr\"");
	afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "zephyr", 7, NULL, NULL);
	afb_api_call(tuto0, "tuto1", "hello", 1, &reply, onrep, NULL);
}

static void run_afb()
{
	rp_set_logmask(-1);
	afb_ev_mgr_init();
	afb_sched_start(1, 1, 30, start, NULL);
}

int main(void)
{
	run_afb();
	return 0;
}

