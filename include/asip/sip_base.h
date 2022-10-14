/*
 * sip_base.h
 *
 *  Created on: Sep 30, 2022
 *      Author: amneiht
 */

#ifndef ASIP_SIP_BASE_H_
#define ASIP_SIP_BASE_H_

#include <acore.h>
#include <pjsip.h>

enum ASIP_EVENT_INV_STATE {
	// register event
	// data is asip_ua pointer
	ASIP_EVENT_REGISTERING = 0,
	ASIP_EVENT_REGISTER_OK,
	ASIP_EVENT_REGISTER_FAILSE,
	// Call event
	ASIP_EVENT_INV_STATE_START, // send event when call asip_ua_call()
	ASIP_EVENT_INV_STATE_START_FALSE, // send event when no match call and can not send invite
	ASIP_EVENT_INV_STATE_INCOMING, // send event when recieve SIP IVITE
	ASIP_EVENT_INV_STATE_MEDIA, // send event when sdp exchange OK
	ASIP_EVENT_INV_STATE_DISCONNECTED, // send event when call is close
	ASIP_EVENT_MAX
};
pj_bool_t asip_init(acore_conf_t *conf);
void asip_close();
pjsip_endpoint* asip_endpt();
pj_int32_t asip_event_id();
int asip_get_transport_port(const pj_str_t *tran);

/// to avoid conflict of resource the application should call asip_start() when call  some register some thing etc ..
void asip_start();
/// the application MUST call this after call asip_start()
void asip_end();

#endif /* ASIP_SIP_BASE_H_ */
