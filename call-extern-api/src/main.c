/*
 * Copyright (c) 2015-2025 IoT.bzh Company
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>

#include <sys/rp-verbose.h>
#include <afb-v4.h>
#include <zafb-helpers.h>

/**********************************************************************/
/* configuration */
/**********************************************************************/

/* configure the server IP */
#if !defined(SERVER_IP)
//#define SERVER_IP "192.168.0.35"
#define SERVER_IP "10.18.127.169"
#endif

/* configure the server port */
#if !defined(SERVER_PORT)
#define SERVER_PORT "4444"
#endif

/* configure the led */
//#define LED_NODE DT_ALIAS(led0)
//#define LED_NODE DT_NODELABEL(red_led) // led 3
//#define LED_NODE DT_NODELABEL(yellow_led) // led 2
//#define LED_NODE DT_NODELABEL(green_led) // led 1
#define LED_NODE DT_NODELABEL(green_led_1) // led 1

/* configure handling of response */
#if !defined(IGNORE_RESPONSE)
#define IGNORE_RESPONSE 0
#endif

/**********************************************************************/
/**********************************************************************/
/**********************************************************************/
/* demo timers */
/**********************************************************************/
/**********************************************************************/
/**********************************************************************/

static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET(LED_NODE, gpios);

uint8_t  state = 0;
uint32_t counter = 0;

afb_data_t state_data;
afb_data_t counter_data;
struct ev_timer *timer;

afb_api_t global;

static void on_reply_cb(
		void *closure,
		int status,
		unsigned nreplies,
		afb_data_t const replies[],
		afb_api_t api)
{
	afb_data_t name;
	int rc = afb_data_convert(replies[0], AFB_PREDEFINED_TYPE_STRINGZ, &name);
	const char *str = rc >= 0 ? afb_data_ro_pointer(name) : "?";
	RP_INFO("replied %d, %s", rc, str);
	afb_data_unref(name);
}

void timer_cb(struct ev_timer *timer, void *closure, unsigned decount)
{
	afb_data_t data[2];
	int st;

	/* toggle the led pin */
	st = gpio_pin_get_dt(&led);
	if (st < 0)
		RP_ERROR("get led failed %d\n", st);
	st = state = !st;
	st = gpio_pin_set_dt(&led, st);
	if (st < 0)
		RP_ERROR("set led failed %d\n", st);

	/* send the counter and the state */
	counter++;
	data[0] = afb_data_addref(state_data);
	data[1] = afb_data_addref(counter_data);

	afb_api_call(zafb_root_api(), "extern", "ping", 2, data,
#if IGNORE_RESPONSE
			NULL, NULL);
#else
			on_reply_cb, NULL);
#endif
}

void start(int signum, void* arg)
{
	int rc;

	RP_INFO("STARTING START");

	/* create data for state once */
	rc = afb_create_data_raw(&state_data,
			AFB_PREDEFINED_TYPE_BOOL, &state, 1, NULL, NULL);
	if (rc < 0) {
		RP_CRITICAL("not able to create the data for state: %d", rc);
		zafb_exit(rc);
		return;
	}

	/* create data for counter once */
	rc = afb_create_data_raw(&counter_data,
			AFB_PREDEFINED_TYPE_U32, &counter, 4, NULL, NULL);
	if (rc < 0) {
		RP_CRITICAL("not able to create the data for counter: %d", rc);
		zafb_exit(rc);
		return;
	}

	/* create the client link */
	rc = afb_api_rpc_add_client_strong(
			"tcp:"SERVER_IP":"SERVER_PORT"/extern",
			zafb_apiset(),
			zafb_apiset());
	if (rc < 0) {
		RP_CRITICAL("not able to create the data for counter: %d", rc);
		zafb_exit(rc);
		return;
	}

	/* create the timer */
	rc = afb_ev_mgr_add_timer(
		&timer,
		0, /* absolute */
		0, /* start sec */
		500, /* start_ms */
		0, /* count */
		500, /* period */
		10, /* accuracy */
		timer_cb, /* callback */
		NULL, /* closure */
		1 /* auto unref */
	);
	if (rc < 0) {
		RP_CRITICAL("not able to create timer: %d", rc);
		zafb_exit(rc);
		return;
	}
}

#define DHCP 1
#if DHCP
extern void init_dhcp(bool);
#else
static inline void init_dhcp(bool x) {}
#endif

int main(void)
{
	int rc;

        /* set the log mask to higher verbosity (optional) */
        rp_set_logmask(rp_Log_Mask_Debug);

	RP_INFO("ENTERING MAIN");

	/* initialize DHCP */
	init_dhcp(true);

	/* setup GPIO for LED */
	rc = gpio_is_ready_dt(&led);
	if (!rc) {
		RP_ERROR("led isn't ready\n");
		return rc;
	}
	rc = gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
	if (rc < 0) {
		RP_ERROR("led configuration failed %d\n", rc);
		return rc;
	}

	RP_INFO("STARTING AFB");
	zafb_start(start, 30, 4, 1);

	RP_INFO("ENDING AFB status %d", rc);
	return rc;
}

