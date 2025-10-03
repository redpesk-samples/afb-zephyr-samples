/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>

#include <zafb-helpers.h>
#include <sys/rp-verbose.h>
#include <afb-v4.h>

/*
 * Get LED configuration from the devicetree led0 alias. This is mandatory.
 */
#define LED0_NODE	DT_ALIAS(led0)
//#define LED0_NODE DT_NODELABEL(red_led) // led 3
#if !DT_NODE_HAS_STATUS_OKAY(LED0_NODE)
#error "Unsupported board: led0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED0_NODE, gpios);

/*
 * Get button configuration from the devicetree sw0 alias. This is mandatory.
 */
#define SW0_NODE	DT_ALIAS(sw0)
#if !DT_NODE_HAS_STATUS_OKAY(SW0_NODE)
#error "Unsupported board: sw0 devicetree alias is not defined"
#endif
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(SW0_NODE, gpios);
static struct gpio_callback button_cb_data;

/******************************************************************/
/*** TLS setting                                                ***/
/******************************************************************/
/* configure TLS */
#if CONFIG_AFB_ZEPHYR_MBEDTLS
   static uint8_t trust[] = {
#    include "trust.h"
   };
#  if MTLS
     static uint8_t cert[] = {
#      include "cert.h"
     };
     static uint8_t key[] = {
#      include "key.h"
     };
#    define TLS "mtls+"
#  else
#    define TLS "tls+"
#  endif /* MTLS */
#else
#  define TLS ""
#endif /* CONFIG_AFB_ZEPHYR_MBEDTLS */

/******************************************************************/
/*** permission check setting                                   ***/
/******************************************************************/
#if !defined(CHECK_PERM)
#  define CHECK_PERM 0
#endif

#if CHECK_PERM

/* configure the server IP */
#  if !defined(PREM_CHECK_IP)
#    define PREM_CHECK_IP "192.168.0.34"
//#    define PREM_CHECK_IP "10.18.127.169"
#  endif

/* configure the server port */
#  if !defined(PREM_CHECK_PORT)
#    define PREM_CHECK_PORT "4444"
#  endif

static const char perm_check_url[] =
	TLS"tcp:"PREM_CHECK_IP":"PREM_CHECK_PORT"/perm";
#endif

/******************************************************************/
/*** declaration of APIs following afb-binding interface	***/
/*** see https://docs.redpesk.bzh/docs/en/master/developer-guides/reference-v4/ */
/******************************************************************/

enum { edge_falling = 1, edge_rising = 2, edge_any = 3 } edge = edge_any;
afb_event_t evt_button = NULL;
afb_event_t evt_led = NULL;

/* utility function for unquoting string */
static void unquote(const char **pstr, size_t *plenstr)
{
	if (*plenstr >= 2 && (*pstr)[0] == '"' && (*pstr)[*plenstr - 1] == '"') {
		(*pstr)++;
		(*plenstr) -= 2;
	}
}

/* utility function for matching value  */
static bool matchstr(const char *value, size_t lenvalue, const char *patterns)
{
	size_t i = 0;
	for (;;) {
		if (i == lenvalue) {
			if (*patterns == 0)
				return true;
		}
		else {
			char v = value[i];
			if (v >= 'A' && v <= 'Z')
				v += 'a' - 'A';
			if (v == *patterns) {
				i++;
				patterns++;
				continue;
			}
		}
		while (*patterns++ != 0);
		if (*patterns == 0)
			return false;
		i = 0;
	}
}

/* get the led value */
static bool get_led()
{
	int st = gpio_pin_get_dt(&led);

	if (st < 0)
		RP_ERROR("get led failed %d", st);
	return st > 0;
}

/* set the led value */
static void set_led_bool(bool value)
{
	int st;
	uint8_t u8;
	afb_data_t data = NULL;

	if (value != get_led()) {
		st = gpio_pin_set_dt(&led, value);
		if (st < 0)
			RP_ERROR("set led failed %d", st);

		u8 = value;
		st = afb_create_data_copy(&data, AFB_PREDEFINED_TYPE_BOOL, &u8, 1);
		if (st < 0)
			RP_ERROR("creation of led data failed %d", st);
		else
			afb_event_push(evt_led, 1, &data);
	}
}

/* set led from string */
static void set_led_string(const char *value, size_t lenvalue)
{
	static char texts_on[] = "1\0on\0yes\0true\0";
	static char texts_off[] = "0\0off\0no\0false\0";
	static char texts_toggle[] = "toggle\0invert\0";

	if (strcmp(value, "null") == 0)
		return;

	unquote(&value, &lenvalue);
	RP_INFO("setting led for %s", value);
	if (matchstr(value, lenvalue, texts_on))
		set_led_bool(true);
	else if (matchstr(value, lenvalue, texts_off))
		set_led_bool(false);
	else if (matchstr(value, lenvalue, texts_toggle))
		set_led_bool(!get_led());
	else
		RP_WARNING("unknown led %s", value);
}

/* get the button value */
static bool get_button()
{
	int st = gpio_pin_get_dt(&button);
	if (st < 0)
		RP_ERROR("get button failed %d", st);
	return st > 0;
}

/* handle button state change */
void button_pressed(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
	int rc;
	afb_data_t data = NULL;
	uint8_t u8;

	u8 = get_button();
	RP_INFO("on button state changed %d", (int)u8);
	if ((u8 == 0 && edge != edge_rising) || (u8 != 0 && edge != edge_falling)) {
		rc = afb_create_data_copy(&data, AFB_PREDEFINED_TYPE_BOOL, &u8, 1);
		if (rc < 0)
			RP_ERROR("creation of button data failed %d", rc);
		else
			afb_event_push(evt_button, 1, &data);
	}
}

/* set led from string */
static void set_edge_string(const char *value, size_t lenvalue)
{
	static char texts_falling[] = "falling\0fall\0down\0";
	static char texts_rising[]  = "rising\0rise\0up\0";
	static char texts_any[]     = "both\0any\0";

	if (strcmp(value, "null") == 0)
		return;

	unquote(&value, &lenvalue);
	RP_INFO("setting edge for %s", value);
	if (matchstr(value, lenvalue, texts_falling))
		edge = edge_falling;
	else if (matchstr(value, lenvalue, texts_rising))
		edge = edge_rising;
	else if (matchstr(value, lenvalue, texts_any))
		edge = edge_any;
	else
		RP_WARNING("unknown edge %s", value);
}

/* set events from string */
static void set_events_string(afb_req_t req, const char *value, size_t lenvalue)
{
	static char texts_both[] = "*\0all\0both\0";
	static char texts_led[] = "led\0";
	static char texts_button[] = "button\0";

	int rc;
	bool add = false, sub = false;
	bool led = false, but = false;

	unquote(&value, &lenvalue);
	AFB_REQ_INFO(req, "setting event for %s", value);
	if (*value == '+') {
		add = true;
		value++;
		lenvalue--;
	}
	else if (*value == '-') {
		sub = true;
		value++;
		lenvalue--;
	}
	else
		add = true;

	if (*value == 0 || matchstr(value, lenvalue, texts_both))
		led = but = true;
	else if (matchstr(value, lenvalue, texts_led))
		led = true;
	else if (matchstr(value, lenvalue, texts_button))
		but = true;
	else
		RP_WARNING("unknown event %s", value);

	if (led) {
		if (add) {
			rc = afb_req_subscribe(req, evt_led);
			if (rc < 0)
				AFB_REQ_ERROR(req, "can't subscribe to led event: %d", rc);
		}
		else if (sub) {
			rc = afb_req_unsubscribe(req, evt_led);
			if (rc < 0)
				AFB_REQ_ERROR(req, "can't unsubscribe to led event: %d", rc);
		}
	}

	if (but) {
		if (add) {
			rc = afb_req_subscribe(req, evt_button);
			if (rc < 0)
				AFB_REQ_ERROR(req, "can't subscribe to button event: %d", rc);
		}
		else if (sub) {
			rc = afb_req_unsubscribe(req, evt_button);
			if (rc < 0)
				AFB_REQ_ERROR(req, "can't unsubscribe to button event: %d", rc);
		}
	}
}

/* led verb callback */
static void led_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int rc, sts = 0;
	afb_data_t reply = NULL;
	uint8_t val;

	if (nparams > 0) {
		/* interpret first parameter */
		size_t size;
		void *mem;
		afb_data_t arg0 = params[0];
		afb_type_t type = afb_data_type(arg0);
		afb_data_get_constant(arg0, &mem, &size);
		if (type == AFB_PREDEFINED_TYPE_BOOL)
			set_led_bool((bool)*(uint8_t*)mem);
		else if (type == AFB_PREDEFINED_TYPE_I32
			|| type == AFB_PREDEFINED_TYPE_U32)
			set_led_bool((bool)*(uint32_t*)mem);
		else if (type == AFB_PREDEFINED_TYPE_STRINGZ && size > 0)
			set_led_string((const char *)mem, size - 1);
		else if (type == AFB_PREDEFINED_TYPE_JSON && size > 0)
			set_led_string((const char *)mem, size - 1);
		else {
			AFB_REQ_ERROR(req, "unexpected type %s", afb_type_name(type));
			sts = AFB_ERRNO_INVALID_REQUEST;
		}
	}

	/* returns the current led value */
	val = get_led();
	rc = afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_BOOL, &val, 1);
	if (rc < 0)
		RP_ERROR("creation of led data failed %d", rc);
	afb_req_reply(req, sts, reply != NULL, &reply);
}

/* button verb callback */
static void button_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int rc;
	afb_data_t reply;
	uint8_t val;

	/* returns the current button value */
	val = get_button();
	rc = afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_BOOL, &val, 1);
	if (rc < 0)
		afb_req_reply(req, AFB_ERRNO_INTERNAL_ERROR, 0, NULL);
	else
		afb_req_reply(req, 0, 1, &reply);
}

/* edge verb callback */
static void edge_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int rc, sts = 0;
	afb_data_t reply = NULL;

	if (nparams > 0) {
		/* interpret first parameter */
		size_t size;
		void *mem;
		afb_data_t arg0 = params[0];
		afb_type_t type = afb_data_type(arg0);
		afb_data_get_constant(arg0, &mem, &size);
		if (type == AFB_PREDEFINED_TYPE_STRINGZ && size > 0)
			set_edge_string((const char *)mem, size - 1);
		else if (type == AFB_PREDEFINED_TYPE_JSON && size > 0)
			set_edge_string((const char *)mem, size - 1);
		else {
			AFB_REQ_ERROR(req, "unexpected type %s", afb_type_name(type));
			sts = AFB_ERRNO_INVALID_REQUEST;
		}
	}
	switch (edge) {
	case edge_falling:
		rc = afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "falling", 8, NULL, NULL);
		break;
	case edge_rising:
		rc = afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "rising", 7, NULL, NULL);
		break;
	default:
	case edge_any:
		rc = afb_create_data_raw(&reply, AFB_PREDEFINED_TYPE_STRINGZ, "any", 4, NULL, NULL);
		break;
	}
	if (rc < 0 && sts == 0)
		sts = AFB_ERRNO_INTERNAL_ERROR;
	afb_req_reply(req, sts, rc >= 0, &reply);
}

/* state verb callback */
static void state_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int rc, len;
	char buffer[100];
	afb_data_t reply = NULL;

	len = snprintf(buffer, sizeof buffer,
			"{\"led\":\"%s\",\"button\":\"%s\",\"edge\":\"%s\"}",
				get_led() ? "on" : "off",
				get_button() ? "down" : "up",
				edge == edge_falling ? "falling" :
					edge == edge_rising ? "rising" : "any");

	if (len < 0)
		rc = AFB_ERRNO_INTERNAL_ERROR;
	else {
		rc = afb_create_data_copy(&reply, AFB_PREDEFINED_TYPE_JSON, buffer, (size_t)(1 + len));
		if (rc < 0)
			rc = AFB_ERRNO_INTERNAL_ERROR;
	}
	afb_req_reply(req, rc, rc >= 0, &reply);
}

/* switch press event verb callback */
static void events_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	int sts = 0;

	if (nparams > 0) {
		/* interpret first parameter */
		size_t size;
		void *mem;
		afb_data_t arg0 = params[0];
		afb_type_t type = afb_data_type(arg0);
		afb_data_get_constant(arg0, &mem, &size);
		if (type == AFB_PREDEFINED_TYPE_STRINGZ && size > 0)
			set_events_string(req, (const char *)mem, size - 1);
		else if (type == AFB_PREDEFINED_TYPE_JSON && size > 0)
			set_events_string(req, (const char *)mem, size - 1);
		else {
			AFB_REQ_ERROR(req, "unexpected type %s", afb_type_name(type));
			sts = AFB_ERRNO_INVALID_REQUEST;
		}
	}
	afb_req_reply(req, sts, 0, NULL);
}

#if CHECK_PERM
/* set the loa to the given value for the requester */
static void reqloa(afb_req_t req, unsigned loa)
{
	RP_DEBUG("REQLOA %u", loa);
	afb_req_session_set_LOA(req, loa);
	afb_req_reply(req, 0, 0, NULL);
}

static void logoff_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	RP_DEBUG("LOGOFF");
	reqloa(req, 0);
}

static void on_perm_cb(void *closure, int status, afb_req_t req)
{
	RP_DEBUG("ONPERM %d", status);
	reqloa(req, status > 0 ? 2 : 1);
}

static void login_verb_cb(afb_req_t req, unsigned nparams, afb_data_t const *params)
{
	RP_DEBUG("LOGIN");
	afb_req_check_permission(req, "urn:redpesk:zephyr:partner:writer", on_perm_cb, NULL);
}
#endif


/*
 * static declaration of the verbs.
 * here two : blinky, switch_event
 */
#if CHECK_PERM
#  define CAN_READ   .session=AFB_SESSION_LOA_1
#  define CAN_WRITE  .session=AFB_SESSION_LOA_2
#else
#  define CAN_READ   .session=AFB_SESSION_NONE
#  define CAN_WRITE  .session=AFB_SESSION_NONE
#endif
static const afb_verb_t desc_verbs[] = {
	{ .verb = "led",    .callback = led_verb_cb,     CAN_WRITE },
	{ .verb = "button", .callback = button_verb_cb,  CAN_WRITE },
	{ .verb = "edge",   .callback = edge_verb_cb,    CAN_WRITE },
	{ .verb = "state",  .callback = state_verb_cb,   CAN_READ },
	{ .verb = "events", .callback = events_verb_cb,  CAN_READ },
#if CHECK_PERM
	{ .verb = "login",  .callback = login_verb_cb,   .session = AFB_SESSION_CHECK },
	{ .verb = "logoff", .callback = logoff_verb_cb },
#endif
	{ .verb = NULL }
};

/* static declaration of api tuto
 */
static const afb_binding_t tuto_desc_api = {
	.api = "zephyr",
	.verbs = desc_verbs
};

/******************************************************************/
/*** declaration of using the above API			 ***/
/******************************************************************/

/* start routine called when scheduler starts */
void start()
{
	int rc;
	afb_api_t api;

	/* show step */
	RP_INFO("Entering start");

#if CONFIG_AFB_ZEPHYR_MBEDTLS
	rc = zafb_tls_add_trust_list(trust, sizeof trust);
	if (rc < 0)
		RP_CRITICAL("not able to set trusted certificate(s): %d", rc);
#  if MTLS
	rc = zafb_tls_set_certificate(cert, sizeof cert);
	if (rc < 0)
		RP_CRITICAL("not able to set certificate: %d", rc);
	rc = zafb_tls_set_private_key(key, sizeof key);
	if (rc < 0)
		RP_CRITICAL("not able to set private key: %d", rc);
#  endif
#endif
#if CHECK_PERM
	/* add permission checker */
	RP_INFO("Adding permission checker ...");
	rc = zafb_add_rpc_client(perm_check_url);
	if (rc < 0)
		RP_ERROR("creation of permission checker failed: %d", rc);
#endif
	/* declaration of APIs */
	RP_INFO("Adding API ...");
	rc = zafb_binding_add(&api, &tuto_desc_api);
	if (rc < 0)
		RP_ERROR("creation of api failed: %d", rc);

	/* add RPC server */
	RP_INFO("Adding RCP server ...");
	rc = zafb_add_rpc_server("tcp:*:1234/zephyr");
	if (rc < 0)
		RP_ERROR("creation of server failed: %d", rc);

	/* create events */
	rc = afb_api_new_event(api, "button", &evt_button);
	if (rc < 0)
		RP_ERROR("creation of event button failed: %d", rc);
	rc = afb_api_new_event(api, "led", &evt_led);
	if (rc < 0)
		RP_ERROR("creation of event led failed: %d", rc);

	/* end */
	RP_INFO("end of start");
}

extern void init_dhcp(bool);

/* start program */
int main(void)
{
	int rc;

	/* set the log mask to higher verbosity (optional) */
	rp_set_logmask(rp_Log_Mask_Debug);

	/* Get IP address using DHCP */
	init_dhcp(true);

	/* Setup LED */
	if (!gpio_is_ready_dt(&led)) {
		RP_ERROR("LED %s is not ready", led.port->name);
		return 0;
	}
	rc = gpio_pin_configure_dt(&led, GPIO_OUTPUT_INACTIVE);
	if (rc < 0) {
		RP_ERROR("%d - Failed to configure %s", rc, led.port->name);
		return rc;
	}

	/* init button + button callback */
	rc = gpio_pin_configure_dt(&button, GPIO_INPUT);
	if (rc != 0) {
		RP_ERROR("Error %d: failed to configure %s pin %d",
		       rc, button.port->name, button.pin);
		return 0;
	}

	rc = gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_BOTH);
	if (rc != 0) {
		RP_ERROR("Error %d: failed to configure interrupt on %s pin %d",
			rc, button.port->name, button.pin);
		return 0;
	}

	gpio_init_callback(&button_cb_data, button_pressed, BIT(button.pin));
	gpio_add_callback(button.port, &button_cb_data);
	RP_INFO("Set up button at %s pin %d", button.port->name, button.pin);

	/* start AFB scheduler */
	return zafb_start(
		start /* start callback */,
		30 /* maximum count of jobs */,
		2 /* maximum count of sessions */,
		1 /* maximum count of threads */);
}

