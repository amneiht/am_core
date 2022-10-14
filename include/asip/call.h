/*
 * inv.h
 *
 *  Created on: Oct 5, 2022
 *      Author: amneiht
 */

#ifndef ASIP_CALL_H_
#define ASIP_CALL_H_

#include <pjlib.h>
#include <asip/ua.h>
#include <acore/event.h>
#include <acore/work.h>

typedef struct asip_call asip_call;

enum asip_media_priority {
	// highest priority level
	asip_priority_high = 0,
	// normal level
	asip_priority_normal = 4,
	// low level
	asip_priority_low = 16,
};
// media control for inv session
typedef struct asip_call_media {
	pj_str_t module_name;
	enum asip_media_priority pri;
	pj_bool_t (*on_start_call)(asip_call *call, void *mod_data);
	pj_bool_t (*on_call_incoming)(asip_call *call,
			pjmedia_sdp_session *remote_sdp, void *user_data);
	pj_bool_t (*match_media)(const pj_str_t *media);
	pj_bool_t (*match_sdp)(const pjmedia_sdp_session *sdp);
	acore_handle_p event_handle;
	acore_callback_clear clear;
	void *user_data;
} asip_call_media;
struct asip_call_check {
	asip_ua *ua;
	pj_str_t caller;
	pj_str_t callee;
	const char *media_type;
};
// call funtion

void asip_ua_call(asip_ua *ua, const pj_str_t *callee, const char *media_type,
		void *data);
void asip_ua_call2(asip_ua *ua, const char *callee, const char *media_type,
		void *data);

asip_ua* asip_call_get_ua(asip_call *call);
const pj_str_t* asip_call_get_caller(asip_call *call);
const pj_str_t* asip_call_get_callee(asip_call *call);

asip_call* asip_call_find_by_inv(pjsip_inv_session *inv);
pj_pool_t* asip_call_get_pool(asip_call *call);
// call control
void asip_call_accept(asip_call *call);
void asip_call_reject(asip_call *call);
void asip_call_close(asip_call *call);

pjsip_inv_session* asip_call_get_inv(asip_call *call);
// call media

//
void* asip_call_add_media_support(const asip_call_media *module);
pj_bool_t asip_call_remove_media_support(void *module);
void* asip_call_module_get_userdata(void *module);
/*			 CALL CONTROl SUUPORT 					*/
// add a control when make or revice all
// it help fom create black list module, etc ...
acore_check_t* asip_call_add_control(pj_pool_t *pool, const char *name,
		acore_work_check check, acore_callback_clear clear, void *user_data);
void asip_call_remove_control(acore_check_t *check);
#endif /* ASIP_CALL_H_ */
