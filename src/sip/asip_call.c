/*
 * asip_call.c
 *
 *  Created on: Oct 5, 2022
 *      Author: amneiht
 */

#include <asip/call.h>
#include "local_sip.h"

static acore_list_t *type_list;
static acore_mmap_t *inv_map;

static void clear_module(void *mcall);
static int prioty_cmp(const acore_list_element *ele1,
		const acore_list_element *ele2) {
	const call_module *call_1 = ele1;
	const call_module *call_2 = ele2;
	return (int) (call_1->pri - call_2->pri);
}
ACORE_LOCAL acore_list_t* _asip_call_module_list() {
	return type_list;
}
ACORE_LOCAL acore_mmap_t* _asip_call_inv_map() {
	return inv_map;
}
ACORE_LOCAL void _asip_call_module_init(pj_pool_t *pool) {
	type_list = acore_list_create(pool, prioty_cmp);
	inv_map = acore_mmap_create(pool);
}
ACORE_LOCAL void _asip_call_module_close() {
	BEGIN
		inv_map = NULL;
		acore_list_destroy(type_list);
	END
}
void* asip_call_add_media_support(const asip_call_media *module) {

	pj_pool_t *pool = acore_pool_create(module->module_name.ptr, 2048, 2048);
	call_module *call = PJ_POOL_ALLOC_T(pool, call_module);
	acore_list_init_element(call);
	call->pool = pool;
	pj_strdup(pool, &call->module_name, &module->module_name);
	call->on_start_call = module->on_start_call;
	//user clear
	call->clear = module->clear;
	call->user_data = module->user_data;
	// media
	call->match_media = module->match_media;
	call->match_sdp = module->match_sdp;
	call->evt = acore_event_resister_handle(asip_event_id(), pool,
			module->event_handle,
			NULL, module->user_data);
	acore_mem_bind(pool, call, clear_module);
	BEGIN
		acore_list_add(type_list, call);
	END
	return call;
}

static void clear_module(void *mcall) {
	call_module *call = mcall;
	acore_event_unregister_handle(call->evt);
	acore_list_remove(type_list, mcall);
	acore_pool_release(call->pool);
}
pj_bool_t asip_call_remove_media_support(void *data) {
	pj_bool_t res = PJ_TRUE;
	BEGIN
		acore_mem_mask_destroy(data);
	END
	return res;
}
pjsip_inv_session* asip_call_get_inv(asip_call *call) {
	return call->inv;
}
void* asip_call_module_get_userdata(void *module) {
	call_module *data = (call_module*) module;
	return data->user_data;
}

acore_check_t* asip_call_add_control(pj_pool_t *pool, const char *name,
		acore_work_check check, acore_callback_clear clear, void *user_data) {
	acore_check_t *res = NULL;
	BEGIN
		res = acore_work_add_check(pool,
				acore_work_get_id(_asip_get_sys()->call_check), name, check,
				clear, user_data);
	END
	return res;
}
void asip_call_remove_control(acore_check_t *check) {
	BEGIN
		acore_work_remove_check(check);
	END
}

static int find_by_type(void *type, const acore_list_element *list) {

	pj_str_t *name = type;
	const call_module *call = list;
	if (call->match_media(name))
		return acore_ele_found;
	return acore_ele_notfound;
}
void asip_ua_call(asip_ua *ua, const pj_str_t *callee, const char *media_type,
		void *data) {
	if (ua->login.state != asip_state_login)
		return;
	pj_status_t status;
	pjsip_dialog *dlg;
	pjsip_inv_session *inv;
	pjsip_tx_data *tdata;

	struct asip_call_check *scall = acore_alloca(struct asip_call_check);
	pj_str_t *tmp_str = &scall->callee;
	asip_strcpy(tmp_str, callee);
	scall->caller = ua->login.cred.username;
	scall->ua = ua;
	scall->media_type = media_type;
	pj_bool_t res = acore_work_test(_asip_sys_work_id(), PJ_FALSE,
			ASIP_EVENT_INV_STATE_START, scall);
	if (res == PJ_FALSE) {
		PJ_LOG(2, (ACORE_FUNC,"call is reject by sip control"));
		return;
	}
	convert_str(type, media_type);
	BEGIN
		call_module *call_mod = acore_list_search(type_list, find_by_type,
				type);
		if (acore_mem_is_available(call_mod)) {
			local_str(remote_uri, 300);
			local_str(local_uri, 300);
			local_str(contact, 300);
			asip_ua_print_call_uri(ua, callee, remote_uri);
			asip_ua_print_uri(ua, local_uri);
			asip_ua_print_contact(ua, contact);
			// tmp call struct for if creat inv false
			asip_call *tmp = acore_alloca(asip_call);
			tmp->callee = scall->callee;
			tmp->caller = scall->caller;
			tmp->ua = ua;
			tmp->inv = NULL;
			tmp->mod = NULL;
			// create dialog
			status = pjsip_dlg_create_uac(pjsip_ua_instance(), local_uri, /* local URI */
			contact, /* local Contact */
			remote_uri, /* remote URI */
			remote_uri, /* remote target */
			&dlg); /* dialog */
			status = pjsip_inv_create_uac(dlg, NULL, 0, &inv);
			if (status != PJ_SUCCESS) {
				PJ_LOG(2, (ACORE_FUNC,"Can't not create inv"));
				pjsip_dlg_terminate(dlg);
				// tmp call
				acore_event_send(asip_event_id(),
						ASIP_EVENT_INV_STATE_START_FALSE, tmp);
				return;
			}
			status = pjsip_auth_clt_set_credentials(&dlg->auth_sess, 1,
					&ua->login.cred);
			if (status != PJ_SUCCESS) {
				PJ_LOG(2, (ACORE_FUNC,"Can't not create inv"));
				pjsip_dlg_terminate(dlg);
				// tmp call
				pjsip_inv_terminate(inv, 400, 0);
				acore_event_send(asip_event_id(),
						ASIP_EVENT_INV_STATE_START_FALSE, tmp);
				return;
			}
			// create call add to bind list
			asip_call *scall = pj_pool_alloc(inv->pool, sizeof(asip_call));
			scall->inv = inv;
			scall->ua = ua;
			scall->caller = ua->login.cred.username;
			pj_strdup(inv->pool, &scall->callee, callee);
			pj_list_insert_after(&ua->call, scall);
			scall->mod = call_mod;
			acore_mmap_set(inv->pool, inv_map, inv, scall);
			// add refrence
			acore_mem_add_ref(call_mod);
			// modyfine invite session
			call_mod->on_start_call(scall, call_mod->user_data);
			status = pjsip_inv_invite(inv, &tdata);
			status = pjsip_inv_send_msg(inv, tdata);
			scall->state = app_call_state_wait;

			acore_event_send(asip_event_id(), ASIP_EVENT_INV_STATE_START,
					scall);
		} else {
			PJ_LOG(2, (ACORE_FUNC,"Mo Match Media Type"));
			return;
		}

	END
}

void asip_ua_call2(asip_ua *ua, const char *callee, const char *media_type,
		void *data) {
	convert_str(callee2, callee);
	asip_ua_call(ua, callee2, media_type, data);
}
