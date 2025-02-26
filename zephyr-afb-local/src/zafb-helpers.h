/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <afb-v4.h>
#include <afb-apis.h>

struct afb_apiset *zafb_apiset();

int zafb_binding_add(afb_api_t *api, const afb_binding_t *binding);

int zafb_start();

