/*
 * event.c
 *
 *  Created on: Aug 20, 2022
 *      Author: amneiht
 */

#include <acore/event.h>
#include <acore/list.h>
#include "local_core.h"
struct acore_event_handle_t {
	ACORE_DECL_LIST_ELEMENT(struct handle_list)
	;
	void (*handle)(void *user_data, int type, void *event_data);
	acore_event_t *parrent;
};
struct acore_event_t {
	ACORE_DECL_LIST_ELEMENT(struct acore_event_t)
	;
	pj_str_t name;
	pj_lock_t *lock;
	pj_bool_t is_destroy;
	acore_list_t *list;
};

static acore_list_t *hlist;
ACORE_LOCAL void _acore_event_init(pj_pool_t *pool) {

	hlist = acore_list_create(pool, NULL);
}
static void clear_evt(void *arg) {
	acore_event_t *evt = arg;
	PJ_LOG(3,
			(ACORE_NAME, "destroy event name %.*s", (int) evt->name.slen, evt->name.ptr));
	evt->is_destroy = PJ_TRUE;
	pj_lock_acquire(evt->lock);
	acore_list_destroy(evt->list);
	pj_lock_release(evt->lock);
	// release data
	pj_lock_destroy(evt->lock);
}
acore_event_t* acore_event_create(pj_pool_t *pool, const char *event_name) {
	PJ_LOG(6, (ACORE_NAME,"create event with name %s", event_name));

	acore_event_t *evt = acore_list_ele_create(pool, sizeof(acore_event_t));
	pj_lock_create_recursive_mutex(pool, event_name, &evt->lock);
	evt->list = acore_list_create(pool, NULL);
	pj_strdup2(pool, &evt->name, event_name);
	evt->is_destroy = PJ_FALSE;
	evt->clear = clear_evt;
	evt->user_data = evt;
	CORE_START
		acore_list_add(hlist, evt);
	CORE_END
	return evt;
}

// call close funtion all event handle
pj_status_t acore_event_destroy(acore_event_t *evt) {
	PJ_LOG(5,
			(ACORE_NAME,"destroy event name %.*s",(int) evt->name.slen, evt->name.ptr));
	CORE_START
		acore_list_remove(hlist, evt);
	CORE_END
	return PJ_SUCCESS;
}

acore_event_handle_t* acore_event_resister_handle(pj_uint32_t event_id,
		pj_pool_t *pool, acore_handle_p handle, acore_callback_clear clear,
		void *data) {
	acore_event_t *evt = acore_list_get_id(hlist, event_id);
	if (evt == NULL || evt->is_destroy) {
		PJ_LOG(5,
				(ACORE_FUNC,"register to event %.*s",(int) evt->name.slen, evt->name.ptr));
		return NULL;
	}

	acore_event_handle_t *po = pj_pool_alloc(pool,
			sizeof(acore_event_handle_t));
	acore_list_init_element(po);
	po->clear = clear;
	po->user_data = data;
	po->handle = handle;
	po->parrent = evt;
	pj_lock_acquire(evt->lock);
	{
		acore_list_add(evt->list, po);
	}
	pj_lock_release(evt->lock);
	return po;
}
pj_status_t acore_event_unregister_handle(acore_event_handle_t *handle) {
	acore_event_t *evt = handle->parrent;
	if (evt == NULL || evt->is_destroy) {
		PJ_LOG(5,
				(ACORE_FUNC,"Remove handle to event %.*s",(int) evt->name.slen, evt->name.ptr));
		return 1;
	}
	pj_lock_acquire(evt->lock);
	acore_list_remove(evt->list, handle);
	pj_lock_release(evt->lock);
	return PJ_SUCCESS;
}
struct evt_data {

	int type;
	void *data;
};

static pj_bool_t send_event(void *data, const acore_list_element *list) {
	const acore_event_handle_t *han = list;
	struct evt_data *evt = (struct evt_data*) data;
	han->handle(han->user_data, evt->type, evt->data);
	return acore_ele_notfound;
}
void acore_event_send(pj_uint32_t event_id, int type, void *data) {
	acore_event_t *evt = acore_list_get_id(hlist, event_id);
	if (evt == NULL || evt->is_destroy) {
		PJ_LOG(5,
				(ACORE_FUNC,"Remove handle to event %.*s",(int) evt->name.slen, evt->name.ptr));
		return;
	}
	pj_lock_acquire(evt->lock);
	struct evt_data em = { .type = type, .data = data };
	acore_list_search(evt->list, send_event, &em);
	pj_lock_release(evt->lock);
}

pj_int32_t acore_event_get_id(acore_event_t *evt) {
	return evt->id;
}
ACORE_LOCAL void _acore_event_release(void) {
	CORE_START
		acore_list_destroy(hlist);
	CORE_END
}

