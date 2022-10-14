/*
 * local_sip.h
 *
 *  Created on: Oct 3, 2022
 *      Author: amneiht
 */

#ifndef SIP_LOCAL_SIP_H_
#define SIP_LOCAL_SIP_H_

#include <asip/call.h>
#include <asip/sip_base.h>
#include "../core/local_core.h"
/// begin lock
#define BEGIN  asip_start();
/// end lock
#define END  asip_end();

#define asip_strcpy(a,b) a->ptr = alloca(b->slen+1) ; \
pj_memcpy(a->ptr , b->ptr , b->slen ); \
a->ptr[b->slen] = 0 ; \
a->slen = b->slen

#define local_str(name , len ) pj_str_t * name = acore_alloca(pj_str_t); \
name->slen = len; \
name->ptr = alloca(len+4); \
name->ptr[len] = 0

typedef struct call_module call_module;

struct asip_sys {
	pj_pool_t *pool;
	void *event;
	pjsip_endpoint *endpt;
	acore_conf_t *conf;
	pj_lock_t *lock;
	struct {
		pjsip_tpfactory *tcp;
		pjsip_transport *udp;
	} transport;
	struct {
		acore_timer_t *timer;
		pj_time_val poll_time;
	} time;

	acore_work_t *call_check;
};
typedef struct asip_sys asip_sys;
ACORE_LOCAL asip_sys* _asip_get_sys();
ACORE_LOCAL pj_int32_t _asip_sys_work_id();
// sip_ua
struct asip_call {
	PJ_DECL_LIST_MEMBER(struct asip_call)
	;
	pj_str_t caller;
	pj_str_t callee;
	asip_ua *ua;
	pjsip_inv_session *inv;
	int state;
	call_module *mod;
	void *user_data;
};
struct asip_ua {
	PJ_DECL_LIST_MEMBER(struct asip_ua)
	;
	pj_pool_t *pool;
	int port;
	struct {
		pj_str_t uid;
		int localport;
		pj_str_t last_contact;
	} contact;
	struct {
		asip_ua_state state;
		pj_time_val tlogin;
		pjsip_cred_info cred;
		int login_time;
		int false_cont;
		pjsip_regc *regc;
	} login;
	struct {
		PJ_DECL_LIST_MEMBER(struct asip_call)
		;
	} call;

};

ACORE_LOCAL void _asip_init_ua();
ACORE_LOCAL void _asip_close_ua();
// call module
typedef enum app_call_state {
	app_call_state_null,
	app_call_state_wait,
	app_call_state_incall,
	app_call_state_handle,
	app_call_state_complete,
} app_call_state;

typedef struct inv_bind {
	ACORE_DECL_LIST_ELEMENT(struct inv_bind)
	;
	asip_call *call;
	pjsip_inv_session *inv;
} inv_bind;

struct call_module {
	ACORE_DECL_LIST_ELEMENT(struct module_list)
	;
	pj_str_t module_name;
	pj_pool_t *pool;
	enum pjsip_module_priority pri;
	pj_bool_t (*on_start_call)(asip_call *call, void *mod_data);
	pj_bool_t (*on_call_incoming)(asip_call *call,
			pjmedia_sdp_session *remote_sdp, void *user_data);
	pj_bool_t (*match_media)(const pj_str_t *media);
	pj_bool_t (*match_sdp)(const pjmedia_sdp_session *sdp);
	acore_event_handle_t *evt;
	int counter;
	struct {
		acore_callback_clear clear;
		void *user_data;
	} mod;
	pj_bool_t is_close;
};

ACORE_LOCAL void _asip_call_module_init(pj_pool_t *pool);
ACORE_LOCAL void _asip_call_module_close();
ACORE_LOCAL acore_list_t* _asip_call_module_list();
ACORE_LOCAL acore_list_t* _asip_call_inv_bind_list();
ACORE_LOCAL void _asip_inv_module(pjsip_endpoint *ep);
#endif /* SIP_LOCAL_SIP_H_ */
