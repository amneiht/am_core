/*
 * asip_inc.c
 *
 *  Created on: Oct 5, 2022
 *      Author: amneiht
 */
#include "local_sip.h"
#include <pjmedia.h>

static pj_bool_t cb_on_invite(pjsip_rx_data *rdata);
static void call_on_tsx_state_changed(pjsip_inv_session *inv,
		pjsip_transaction *tsx, pjsip_event *e);
static pjsip_module invite_module = {
NULL, NULL, /* prev, next.		*/
{ "asip_invite_module", 18 }, /* Name.			*/
-1, /* Id			*/
PJSIP_MOD_PRIORITY_APPLICATION - 1, /* Priority			*/
NULL, /* load()			*/
NULL, /* start()			*/
NULL, /* stop()			*/
NULL, /* unload()			*/
&cb_on_invite, /* on_rx_request()		*/
NULL, /* on_rx_response()		*/
NULL, /* on_tx_request.		*/
NULL, /* on_tx_response()		*/
NULL, /* on_tsx_state()		*/
};

struct inv_object {
	pjsip_rx_data *rdata;
	pjsip_inv_session *inv;
	pj_str_t caller;
	pjmedia_sdp_session *sdp;
};
static int match_sdp(void *data, const acore_list_element *list) {
	pjmedia_sdp_session *sdp = data;
	const call_module *call = (const call_module*) list;
	pj_bool_t res = call->match_sdp(sdp);
	if (res == PJ_TRUE)
		return acore_ele_found;
	return acore_ele_notfound;
}
static pj_bool_t cb_on_invite(pjsip_rx_data *rdata) {
	pj_status_t status;
	pjmedia_sdp_session *sdp;
	pjsip_dialog *dlg;
	pjsip_inv_session *inv;
	pjsip_tx_data *tdata;
	pj_str_t caller = { NULL, 0 };
	pj_str_t callee = { NULL, 0 };

	struct inv_object *inc;
	asip_call *acall = NULL;
	local_str(contact, 300);

	if (rdata->msg_info.msg->line.req.method.id != pjsip_invite_method.id)
		return PJ_FALSE;
	// find ua
	asip_ua *ua = asip_ua_find_by_uri(rdata->msg_info.to->uri);
	if (ua == NULL) {
		PJ_LOG(3, ("inv","No matching ua"));
		return PJ_FALSE;
	}
	// check control
	pjsip_uri *uri = rdata->msg_info.from->uri;
	if (PJSIP_URI_SCHEME_IS_SIP(uri) || PJSIP_URI_SCHEME_IS_SIPS(uri)) {
		pjsip_sip_uri *suri = (pjsip_sip_uri*) pjsip_uri_get_uri(uri);
		caller = suri->user;
	}
	callee = ua->login.cred.username;
	struct asip_call_check *check = acore_alloca(struct asip_call_check);
	check->ua = ua;
	check->callee = callee;
	check->caller = caller;
	pj_bool_t vaild = acore_work_test(_asip_sys_work_id(), PJ_FALSE,
			ASIP_EVENT_INV_STATE_INCOMING, check);

	if (!vaild) {
		PJ_LOG(3, ("inv","check incoming call is not pass"));
		return PJ_FALSE;
	}
	// print contact
	asip_ua_print_contact(ua, contact);
	// create inv session
	status = pjmedia_sdp_parse(rdata->tp_info.pool,
			rdata->msg_info.msg->body->data, rdata->msg_info.msg->body->len,
			&sdp);
	status = pjsip_dlg_create_uas_and_inc_lock(pjsip_ua_instance(), rdata,
			contact, /* contact */
			&dlg);

	status |= pjsip_inv_create_uas(dlg, rdata, NULL, 0, &inv);
	if (status != PJ_SUCCESS) {
		PJ_LOG(5, (ACORE_NAME,"cannot create inv"));
		pjsip_dlg_create_response(dlg, rdata, 400, NULL, &tdata);
		pjsip_dlg_send_response(dlg, pjsip_rdata_get_tsx(rdata), tdata);
		pjsip_dlg_dec_lock(dlg);
		return PJ_TRUE;
	}
	// call module data
	inc = acore_alloca(struct inv_object);
	inc->caller = caller;
	inc->inv = inv;
	inc->rdata = rdata;
	inc->sdp = sdp;
	BEGIN
	inv_bind *inb = pj_pool_alloc(inv->pool, sizeof(inv_bind));
	// create get media
	call_module *callm = acore_list_search(_asip_call_module_list(), match_sdp,
			sdp);
	if (callm == NULL) {
		pj_str_t respone = pj_str("No Support media");
		pjsip_tx_data *tdata;
		if (inv != NULL) {
			pjsip_inv_end_session(inv, 400, &respone, &tdata);
			pjsip_inv_send_msg(inv, tdata);
		}
	} else {
		acall = pj_pool_alloc(inv->pool, sizeof(asip_call));
		pj_strdup(inv->pool, &acall->caller, &inc->caller);
		acall->callee = ua->login.cred.username;
		acall->mod = callm;
		acall->state = app_call_state_wait;
		acall->ua = ua;
		acall->user_data = inb;
		inb->call = acall;
		inb->inv = inv;
		acore_list_add(_asip_call_inv_bind_list(), inb);
		pj_list_insert_after(&ua->call, acall);
		// callback for call module
		callm->on_call_incoming(acall, sdp, callm->mod.user_data);
	}
	END
	if (acall) {
		acore_event_send(asip_event_id(), ASIP_EVENT_INV_STATE_INCOMING, acall);
	}
	return PJ_TRUE;
}
static void call_on_media_update(pjsip_inv_session *inv, pj_status_t status) {
	if (!inv)
		return;
	asip_call *call = asip_call_find_by_inv(inv);
	if (call == NULL) {
		PJ_LOG(3, ("on_media_update" , "can't not find asip_call"));
		return;
	}
	BEGIN
	if (call->state < app_call_state_handle) {
		acore_event_send(asip_event_id(), ASIP_EVENT_INV_STATE_MEDIA, call);
		call->state = app_call_state_incall;
	}
	END
}

static void call_on_forked(pjsip_inv_session *inv, pjsip_event *e) {
	(void) inv;
	(void) e;
}
static void call_on_state_changed(pjsip_inv_session *inv, pjsip_event *e) {
	asip_call *call = asip_call_find_by_inv(inv);
	if (call == NULL) {
		PJ_LOG(3, (ACORE_NAME , "cant not find asip_call"));
		return;
	}
	if (inv->state == PJSIP_INV_STATE_DISCONNECTED) {
		call->state = app_call_state_complete;
		acore_event_send(asip_event_id(), ASIP_EVENT_INV_STATE_DISCONNECTED,
				call);
		BEGIN
		acore_list_remove(_asip_call_inv_bind_list(), call->user_data);
		END
	}
}
static void call_on_tsx_state_changed(pjsip_inv_session *inv,
		pjsip_transaction *tsx, pjsip_event *e) {
	asip_call *call = asip_call_find_by_inv(inv);
	if (call == NULL) {
		PJ_LOG(3, ("on_tsx_state" , "cant not find asip_call"));
		return;
	}
	if (tsx->state == PJSIP_TSX_STATE_COMPLETED
			&& call->state < app_call_state_handle) {
		call->state = app_call_state_handle;
	}
}
ACORE_LOCAL void _asip_inv_module(pjsip_endpoint *ep) {
	pjsip_inv_callback inv_cb;
	pjsip_endpt_register_module(ep, &invite_module);
	pj_bzero(&inv_cb, sizeof(inv_cb));
	inv_cb.on_state_changed = call_on_state_changed;
	inv_cb.on_new_session = call_on_forked;
	inv_cb.on_media_update = call_on_media_update;
	inv_cb.on_tsx_state_changed = call_on_tsx_state_changed;
	/* Initialize invite session module:  */
	pj_status_t st = pjsip_inv_usage_init(ep, &inv_cb);
	if (st != PJ_SUCCESS) {
		PJ_LOG(1, (ACORE_FUNC, "loi set sup inv call back\n"));
	}
}
static int inv_search(void *value, const void *node) {

	pjsip_inv_session *inv = value;
	const inv_bind *inb = node;
	if (inb->inv == inv)
		return acore_ele_found;
	return acore_ele_notfound;
}
asip_call* asip_call_find_by_inv(pjsip_inv_session *inv) {
	return (asip_call*) acore_list_search(_asip_call_inv_bind_list(),
			inv_search, inv);
}
pj_pool_t* asip_call_get_pool(asip_call *call) {
	return call->inv->pool;
}
