/*
 * Copyright (c) 2012-2014 Wind River Systems, Inc.
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>

#include <sys/rp-verbose.h>
#include <afb-v4.h>
#include <afb-apis.h>

/******************************************/
/* AFB ZEPHYR INTEGRATION SECTION - START */
/******************************************/
struct afb_apiset *zafb_all_apis;

struct afb_apiset *zafb_apiset()
{
	if (zafb_all_apis == NULL) {
		zafb_all_apis = afb_apiset_create(NULL, 0);
	}
	return zafb_all_apis;
}	

int __zafb_binding_add_preinit__(afb_api_t api, void *closure)
{
	int rc;
	const afb_binding_t *binding = closure;
	union afb_ctlarg ctlarg;

	/* record the description */
	afb_api_v4_set_userdata(api, binding->userdata);
	afb_api_v4_set_mainctl(api, binding->mainctl);
	afb_api_v4_set_verbs(api, binding->verbs);
	rc = 0;
	if (binding->provide_class)
		rc =  afb_api_v4_class_provide(api, binding->provide_class);
	if (!rc && binding->require_class)
		rc =  afb_api_v4_class_require(api, binding->require_class);
	if (!rc && binding->require_api)
		rc =  afb_api_v4_require_api(api, binding->require_api, 0);
	if (rc >= 0 && binding->mainctl) {
		/* call the pre init routine safely */
		memset(&ctlarg, 0, sizeof ctlarg);
		ctlarg.pre_init.path = NULL;
		ctlarg.pre_init.uid = binding->api;
		ctlarg.pre_init.config = NULL;
		rc = afb_api_v4_safe_ctlproc(api, binding->mainctl, afb_ctlid_Pre_Init, &ctlarg);
	}

	/* seal after init allows the pre init to add things */
	if (rc >= 0)
		afb_api_v4_seal(api);
	return rc;
}

int zafb_binding_add(afb_api_t *api, const afb_binding_t *binding)
{
	int rc = afb_api_v4_create(
		api,
		zafb_apiset(), zafb_apiset(),
		binding->api, Afb_String_Const,
		binding->info, Afb_String_Const,
		binding->noconcurrency,
		__zafb_binding_add_preinit__, (void*)binding,
		NULL, Afb_String_Const);
	return rc;
}

int zafb_start()
{
	return afb_apiset_start_all_services(zafb_apiset());
}
/****************************************/
/* AFB ZEPHYR INTEGRATION SECTION - END */
/****************************************/

/* API TUTO SECTION - START */
/* hello verb callback */
void hello(afb_req_t req, unsigned nparams, afb_data_t const *params)
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

const afb_verb_t tuto_desc_verbs[] = {
	{ .verb="hello", .callback=hello },
	{ .verb=NULL }
};

static int tuto_mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
	switch (ctlid) {
	case afb_ctlid_Root_Entry:
		AFB_API_NOTICE(api, "unexpected root entry");
		break;

	case afb_ctlid_Pre_Init:
		AFB_API_NOTICE(api, "hello binding comes to live with uid %s", ctlarg->pre_init.uid);
#if defined(PREINIT_PROVIDE_CLASS)
		afb_api_provide_class(api, PREINIT_PROVIDE_CLASS);
#endif
#if defined(PREINIT_REQUIRE_CLASS)
		afb_api_require_class(api, PREINIT_REQUIRE_CLASS);
#endif
		break;

	case afb_ctlid_Init:
		AFB_API_NOTICE(api, "hello binding starting");
#if defined(INIT_REQUIRE_API)
		afb_api_require_api(api, INIT_REQUIRE_API, 1);
#endif
		break;

	case afb_ctlid_Class_Ready:
		AFB_API_NOTICE(api, "hello binding has classes ready");
		break;

	case afb_ctlid_Orphan_Event:
		AFB_API_NOTICE(api, "received orphan event %s", ctlarg->orphan_event.name);
		break;

	case afb_ctlid_Exiting:
		AFB_API_NOTICE(api, "exiting code %d", ctlarg->exiting.code);
		break;

	default:
		break;
	}
	return 0;
}

const afb_binding_t tuto0_desc_api = {
	.api = "tuto0",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

const afb_binding_t tuto1_desc_api = {
	.api = "tuto1",
	.mainctl = tuto_mainctl,
	.verbs = tuto_desc_verbs
};

afb_api_t tuto0, tuto1;

void onrep(
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
	afb_api_call(tuto0, "tuto0", "hello", 1, &reply, onrep, NULL);

	k_sleep(K_MSEC(1000));

	RP_INFO("API tuto1 call API tuto0 hello verb with arg \"zephyr\"");
	afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "zephyr", 7, NULL, NULL);
	afb_api_call(tuto1, "tuto1", "hello", 1, &reply, onrep, NULL);
}

static void run_afb()
{
	rp_set_logmask(-1);
	afb_ev_mgr_init();
	afb_sched_start(1, 1, 30, start, NULL);
}

#define RUN_IN_THREAD 0
#if RUN_IN_THREAD
static void *start_afb(void* arg)
{
	run_afb();
	return NULL;
}
static void run_afb_in_separate_thread()
{
	int rc;
	x_thread_t tid;
	printf ("starting of afb thread...\n");
	rc = x_thread_create(&tid, start_afb, NULL, 1);
	printf ("start of afb thread %d\n", rc);
}
#define run_afb run_afb_in_separate_thread
#endif

int main(void)
{
	run_afb();
	return 0;
}

