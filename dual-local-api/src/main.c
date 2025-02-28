/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/logging/log_ctrl.h>

#include <afb-v4.h>

/******************************************************************/
/*** declaration of APIs following afb-binding interface        ***/
/*** see https://docs.redpesk.bzh/docs/en/master/developer-guides/reference-v4/ */
/******************************************************************/

static void hello_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params);
static int tuto_mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata);

/* static declaration of the verbs.
 * here only one: hello
 */
static const afb_verb_t tuto_desc_verbs[] = {
	{ .verb="hello", .callback=hello_verb_cb },
	{ .verb=NULL }
};

/* static declaration of api tuto0
 */
static const afb_binding_t tuto0_desc_api = {
	.api = "tuto0",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

/* static declaration of api tuto1
 */
static const afb_binding_t tuto1_desc_api = {
	.api = "tuto1",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

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
	AFB_REQ_NOTICE(req, "Hi observer! I'm %s and I received hello from %s",
		afb_req_get_called_api(req), who);

	/* create the reply string */
	retcod = snprintf(hello, sizeof hello, "Hello %s!", who);
	if (retcod >= (int)sizeof hello) {
		AFB_REQ_WARNING(req, "name too long, truncated!");
		retcod = (int)sizeof hello - 1;
		hello[retcod] = 0;
	}

	/* make the reply string
         * copy it because stack value are removed before the reply is delivered
	 */
	afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_STRINGZ,
		hello, (size_t)(retcod + 1 /*with last zero*/));
	afb_req_reply(req, 0, 1, &reply);
}

/* main control function 
 * serve to schedule actions at pre-init and init
 */
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

/******************************************************************/
/*** declaration of using the above API                         ***/
/******************************************************************/

#include <zafb-helpers.h>
#include <sys/rp-verbose.h>

/* handle statically the api handlers
 * (but not of use in this sample)
 */
static afb_api_t tuto0, tuto1;

/* handle call's replies
 */
static void onrep(
	void *closure, /* closure given in 'afb_api_call' */
	int status,    /* returned status */
	unsigned nreplies,           /* count of replied data */
	afb_data_t const replies[],  /* array of replied data */
	afb_api_t api  /* api handler as given in 'afb_api_call' */
) {
	int rc;
	unsigned idx;
	afb_data_t str;

	/* log */
	AFB_API_NOTICE(api, "reply received closure=%s", (const char*)closure);

	/* check status and display it */
	if (AFB_IS_ERRNO(status))
		AFB_API_NOTICE(api, "got error %d", status);
	else
		AFB_API_NOTICE(api, "got cool status of %d", status);

	/* iterate on returned data and print it if possible */
	for (idx = 0 ; idx < nreplies ; idx++) {
		rc = afb_data_convert(replies[idx], AFB_PREDEFINED_TYPE_STRINGZ, &str);
		if (rc >= 0) {
			AFB_API_NOTICE(api, "got data %u as a string: %s", idx,
					(const char *)afb_data_ro_pointer(str));
			afb_data_unref(str);
		}
		else {
			AFB_API_NOTICE(api, "got data %u of type %s", idx,
					afb_type_name(afb_data_type(replies[idx])));
		}
	}

	/* log */
	AFB_API_NOTICE(api, "leaving reply processing");
}

/* start routine called once scheduler started.
 * 'signum' is always zero on Zephyr.
 * 'arg' is the closure argument given to 'afb_sched_start' */
void start()
{
	afb_data_t data;

	while(LOG_PROCESS());

	/* declaration of APIs */
	RP_INFO("Adding APIs ...");
	zafb_binding_add(&tuto0, &tuto0_desc_api);
	zafb_binding_add(&tuto1, &tuto1_desc_api);

	/* start of APIs */
	RP_INFO("Starting APIs ...");
	zafb_start_all_api();
	while(LOG_PROCESS());

	/* sample internal call */
	RP_INFO("API tuto0 call API tuto1 hello verb with arg \"tuto-0\"");
	afb_create_data_raw(&data, AFB_PREDEFINED_TYPE_STRINGZ, "tuto-0", 7, NULL, NULL);
	afb_api_call(tuto0, "tuto1", "hello", 1, &data, onrep, "*call-1*");

	RP_INFO("Sleeping 1 sec....");
	k_sleep(K_MSEC(1000));
	RP_INFO("Wake up !");

	/* other internal call */
	RP_INFO("API tuto1 call API tuto0 hello verb with arg \"tuto-1\"");
	afb_create_data_raw(&data, AFB_PREDEFINED_TYPE_STRINGZ, "tuto-1", 7, NULL, NULL);
	afb_api_call(tuto1, "tuto0", "hello", 1, &data, onrep, "*call-2*");

	/* end */
	RP_INFO("end of start");
	while(LOG_PROCESS());
}

/* start program */
int main(void)
{
	LOG_INIT();
	/* set the log mask to higher verbosity (optional) */
	rp_set_logmask(rp_Log_Mask_Debug);
	/* start AFB scheduler */
	zafb_start(start, 30, 4, 1);
}

