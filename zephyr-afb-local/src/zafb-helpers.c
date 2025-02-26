/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "zafb-helpers.h"
#include <stdio.h>
#include <zephyr/kernel.h>

#include <sys/rp-verbose.h>
#include <afb-v4.h>
#include <afb-apis.h>

/******************************************/
/* AFB ZEPHYR INTEGRATION SECTION - START */
/******************************************/
static struct afb_apiset *__zafb_all_apis__;

struct afb_apiset *zafb_apiset()
{
	if (__zafb_all_apis__ == NULL) {
		__zafb_all_apis__ = afb_apiset_create(NULL, 0);
	}
	return __zafb_all_apis__;
}	

static int __zafb_binding_add_preinit__(afb_api_t api, void *closure)
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

