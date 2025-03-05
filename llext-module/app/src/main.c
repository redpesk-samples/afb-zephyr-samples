#include <zephyr/kernel.h>
#include <zephyr/app_memory/mem_domain.h>

#include <rp-utils/rp-verbose.h>
#include <zafb-helpers.h>
__attribute__ ((aligned (8))) static const
#include "../../ext1/build/ext1.inc"
#define ext1_inc ext1_llext
#define ext1_len ext1_llext_len

__attribute__ ((aligned (8))) static const
#include "../../ext2/build/ext2.inc"
#define ext2_inc ext2_llext
#define ext2_len ext2_llext_len

#ifdef CONFIG_USERSPACE
struct k_mem_domain domain;
#endif

struct llext *ext1, *ext2;

/* start routine called when scheduler starts */
void start()
{
	int rc;

	/* show step */
	RP_INFO("Entering start");

#ifdef CONFIG_USERSPACE
	zafb_load_llext_binding("ext1", &ext1, ext1_inc, ext1_len, domain);
	zafb_load_llext_binding("ext2", &ext2, ext2_inc, ext2_len, domain);
#else
	zafb_load_llext_binding("ext1", &ext1, ext1_inc, ext1_len);
	zafb_load_llext_binding("ext2", &ext2, ext2_inc, ext2_len);
#endif

	/* add RPC server */
	RP_INFO("Adding RCP server ...");
	rc = zafb_add_rpc_server("tcp:*:1111/ext1");
	if (rc < 0)
		RP_ERROR("creation of server failed: %d", rc);
	rc = zafb_add_rpc_server("tcp:*:1112/ext2");
	if (rc < 0)
		RP_ERROR("creation of server failed: %d", rc);

	/* end */
	RP_INFO("end of start");
}

extern void init_dhcp(bool);

int main(void)
{
#ifdef CONFIG_USERSPACE
	k_mem_domain_init(&domain, 0, NULL);
	k_mem_domain_add_thread(&domain, k_current_get());
#endif

	/* set the log mask to higher verbosity (optional) */
	rp_set_logmask(rp_Log_Mask_Debug);

        /* Get IP address using DHCP */
        init_dhcp(true);


	/* start AFB scheduler */
	return zafb_start(
		start /* start callback */,
		30 /* maximum count of jobs */,
		2 /* maximum count of sessions */,
		1 /* maximum count of threads */);
}
