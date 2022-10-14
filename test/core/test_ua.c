/*
 * core.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */
#include <acore.h>
#include <asip.h>

static void log_handle(void *user_data, int type, void *event_data) {
	(void) user_data;
	if (type == ASIP_EVENT_REGISTERING) {
		PJ_LOG(1, (ACORE_NAME,"is restering"));
	} else if (type == ASIP_EVENT_REGISTER_OK) {
		PJ_LOG(1, (ACORE_NAME,"sip is resgister ok"));
	} else if (type == ASIP_EVENT_REGISTER_FAILSE) {
		PJ_LOG(1, (ACORE_NAME,"sip is resgister false"));
	}

	if (event_data == NULL) {
		PJ_LOG(1, (ACORE_NAME,"NULL data"));
	}
}
int main(int argc, char **argv) {
	acore_init();
	pj_log_set_level(1);
	asip_init(NULL);
	pj_pool_t *pool = acore_pool_create(NULL, 4096, 4096);
	acore_event_resister_handle(asip_event_id(), pool, log_handle, NULL, NULL);
	asip_ua *ua = asip_ua_create2(NULL, "5032", "5032", "192.168.1.238", 22001);
	asip_ua_register(ua);
	acore_loop();
	asip_close();

	acore_close();
}
