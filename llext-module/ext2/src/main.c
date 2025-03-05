/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/llext/symbol.h>

#define AFB_BINDING_VERSION 4
#include <afb/afb-binding.h>

#define DIRECT 0

/******************************************************************/
/*** declaration of APIs following afb-binding interface        ***/
/*** see https://docs.redpesk.bzh/docs/en/master/developer-guides/reference-v4/ */
/******************************************************************/

static void hello_cb(afb_req_t req, unsigned nparams, afb_data_t const *params);
static int mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata);

/* static declaration of the verbs.
 * here only one: hello
 */
static const afb_verb_t desc_verbs[] = {
	{ .verb="hello", .callback=hello_cb },
	{ .verb=NULL }
};

/* declaration of api tuto1
 */
const afb_binding_t afbBindingExport = {
	.api = "ext2",
	.mainctl = mainctl,
#if !DIRECT
	.require_api = "ext1",
#endif
	.verbs = desc_verbs
};

#if DIRECT
static int append(char *dest, int size, const char *str)
{
	int len = 0;
	while (len < size && (dest[len] = str[len]) != 0)
		len++;
	return len;
}
#else
static void onhellorep(void *closure, int status, unsigned nreplies, afb_data_t const replies[], afb_req_t req)
{
	AFB_REQ_NOTICE(req, "reply of ext 1");
	afb_data_array_addref(nreplies, replies);
	afb_req_reply(req, status, nreplies, replies);
}
#endif

/* hello verb callback */
static void hello_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int retcod;
	const char *who = "World"; /* default value */
	afb_data_t name;
#if DIRECT
	char hello[100];
	afb_data_t reply;
#endif

	/* some output for tracking the process */
	AFB_REQ_DEBUG(req, "Hi debugger!");

	/* extract the name of the caller */
	retcod = afb_req_param_convert(req, 0, AFB_PREDEFINED_TYPE_STRINGZ, &name);
	who = retcod == 0 ? afb_data_ro_pointer(name) : "World" /* default value */;
	AFB_REQ_NOTICE(req, "Hi observer! I'm %s and I received hello from %s",
		afb_req_get_called_api(req), who);

#if DIRECT
	/* create the reply string */
	retcod = append(hello, (int)sizeof(hello), "Hello ");
	retcod += append(&hello[retcod], (int)sizeof(hello) - retcod, who);
	retcod += append(&hello[retcod], (int)sizeof(hello) - retcod, "!");
	if (retcod >= (int)sizeof hello) {
		AFB_REQ_WARNING(req, "name too long, truncated!");
		retcod = (int)sizeof hello - 1;
		hello[retcod] = 0;
	}
	AFB_REQ_NOTICE(req, "replying \"%s\"", hello);

	/* make the reply string
         * copy it because stack value are removed before the reply is delivered
	 */
	afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_STRINGZ,
		hello, (size_t)(retcod + 1 /*with last zero*/));
	afb_req_reply(req, 0, 1, &reply);
#else
	AFB_REQ_NOTICE(req, "Calling ext 1");
	afb_data_addref(name);
	afb_req_subcall(req, "ext1", "hello", 1, &name, 0, onhellorep, NULL);
#endif
}

/* main control function 
 * serve to schedule actions at pre-init and init
 */
static int mainctl(afb_api_t api, afb_ctlid_t ctlid, afb_ctlarg_t ctlarg, void *userdata)
{
	switch (ctlid) {
	case afb_ctlid_Pre_Init:
		AFB_API_NOTICE(api, "ext2 in pre-init");
		break;

	case afb_ctlid_Init:
		AFB_API_NOTICE(api, "ext2 in init");
		break;

	default:
		break;
	}
	return 0;
}

#include <zephyr/llext/symbol.h>
LL_EXTENSION_SYMBOL(afbBindingExport);
LL_EXTENSION_SYMBOL(afbBindingRoot);
LL_EXTENSION_SYMBOL(afbBindingV4r1_itfptr);
LL_EXTENSION_SYMBOL(afbBindingV4_itf_revision);
/*
*/

