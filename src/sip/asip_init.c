/*
 * asip_init.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#include <asip.h>
#include "local_sip.h"
static pj_bool_t sip_init = PJ_FALSE;

static struct asip_sys sip_sys;
static void sip_time_loop(acore_timer_t *entry, void *arg) {
	(void) entry;
	asip_sys *sys = arg;
	pjsip_endpt_handle_events(sys->endpt, &sys->time.poll_time);
}
pj_bool_t asip_init(acore_conf_t *conf) {

	if (sip_init)
		return sip_init;
	sip_init = PJ_TRUE;
	pj_status_t status = acore_init();

	pj_bzero(&sip_sys, sizeof(asip_sys));
	// init sip endpoint
	pjsip_endpoint *endpt;
	sip_sys.pool = acore_pool_create("sip_pool", 4096, 4096);
	status = pjsip_endpt_create(acore_pool_factory(), "asip.V1", &endpt);
	status |= pjsip_tsx_layer_init_module(endpt);
	status |= pjsip_ua_init_module(endpt, NULL);
	status |= pjsip_100rel_init_module(endpt);
	PJ_ASSERT_RETURN(status == PJ_SUCCESS, status);

	//init transpot it will block if network connect false
	//udp transport
	pj_uint16_t af = pj_AF_INET();
	pjsip_udp_transport_cfg udp_cfg;
	pjsip_udp_transport_cfg_default(&udp_cfg, af);
	if (PJ_AF_INET == af) {
		status = pjsip_udp_transport_start2(endpt, &udp_cfg,
				&sip_sys.transport.udp);
	}
	//ipv6 is not suport for now

	//tcp transport
	if (PJ_AF_INET == af) {
		pjsip_tcp_transport_cfg tcp_cfg;
		pjsip_tcp_transport_cfg_default(&tcp_cfg, af);
		status = pjsip_tcp_transport_start3(endpt, &tcp_cfg,
				&sip_sys.transport.tcp);
	}
	// register sip custom event
	sip_sys.event = acore_event_create(sip_sys.pool, "sip_event");
	sip_sys.endpt = endpt;
	sip_sys.conf = conf;

	acore_timer_t *timer = acore_timer_entry_create(sip_sys.pool, "sip_timer",
			PJ_TRUE, 500, sip_time_loop, &sip_sys);
	sip_sys.time.timer = timer;
	sip_sys.time.poll_time.sec = 0;
	sip_sys.time.poll_time.msec = 100;
	// create lock
	pj_lock_create_recursive_mutex(sip_sys.pool, "sip_lock", &sip_sys.lock);
	acore_main_timer_register(timer);
	_asip_init_ua(sip_sys.pool);
	_asip_call_module_init(sip_sys.pool);
	_asip_inv_module(endpt);
	// work check
	sip_sys.call_check = acore_work_create(sip_sys.pool);
	return PJ_TRUE;
}
void asip_close() {
	if (sip_init) {
		_asip_call_module_close();
		_asip_close_ua();
		acore_event_destroy(sip_sys.event);
		acore_main_timer_unregister(sip_sys.time.timer);
		pjsip_endpt_destroy(sip_sys.endpt);
		pj_lock_destroy(sip_sys.lock);
		acore_pool_release(sip_sys.pool);
	}
	sip_init = PJ_FALSE;
}
ACORE_LOCAL asip_sys* _asip_get_sys() {
	return &sip_sys;
}
ACORE_LOCAL pj_int32_t _asip_sys_work_id() {
	return acore_work_get_id(sip_sys.call_check);
}
pj_int32_t asip_event_id() {
	return acore_event_get_id((acore_event_t*) sip_sys.event);
}
pjsip_endpoint* asip_endpt() {
	if (sip_init) {
		return sip_sys.endpt;
	}
	return NULL;
}
