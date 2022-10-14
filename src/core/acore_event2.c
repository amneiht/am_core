///*
// * event.c
// *
// *  Created on: Aug 20, 2022
// *      Author: amneiht
// */
//
//#include <acore/event.h>
//#include <acore.h>
//#include <acore/list.h>
//#include <pjlib.h>
//struct handle_list {
//	ACORE_DECL_LIST_ELEMENT(struct handle_list)
//	;
//	void (*handle)(void *user_data, int type, void *event_data);
//};
//struct event_list {
//	pj_pool_t *pool;
//	pj_str_t name;
//	pj_lock_t *lock;
//	pj_bool_t is_destroy;
//	acore_list_t *list;
//};
//
////static struct event_list **hlist;
//static acore_list_t *hist;
//ACORE_LOCAL void _acore_event_init(pj_pool_t *pool) {
////	hlist = pj_pool_calloc(pool, acore_config()->max_event,
////			sizeof(struct event_list*));
//	hlist = acore_list_create(pool, NULL);
//}
//pj_int32_t acore_event_create(pj_pool_t *pool, const char *event_name) {
//	PJ_LOG(6, (ACORE_NAME,"create event with name %s", event_name));
//	for (int i = 0; i < max_evt; i++) {
//		if (hlist[i] == NULL) {
//			hlist[i] = pj_pool_alloc(pool, sizeof(struct event_list));
//			hlist[i]->pool = pool;
//			pj_strdup2(pool, &hlist[i]->name, event_name);
//			hlist[i]->list = acore_list_create(pool, NULL);
//			pj_lock_create_recursive_mutex(pool, event_name, &hlist[i]->lock);
//			hlist[i]->is_destroy = PJ_FALSE;
//			return i;
//		}
//	}
//	return -1;
//}
//static pj_bool_t is_not_avaiable(pj_uint32_t id) {
//	if (id > max_evt || hlist[id] == NULL)
//		return PJ_TRUE;
//	return hlist[id]->is_destroy;
//}
//// call close funtion all event handle
//pj_status_t acore_event_destroy(pj_uint32_t id) {
//
//	if (is_not_avaiable(id))
//		return 1;
//	PJ_LOG(6,
//			(ACORE_NAME,"destroy event name %.*s",(int) hlist[id]->name, hlist[id]->name.ptr));
//	hlist[id]->is_destroy = PJ_TRUE;
//	pj_lock_acquire(hlist[id]->lock);
//	acore_list_destroy(hlist[id]->list);
//	pj_lock_release(hlist[id]->lock);
//	// release data
//	pj_lock_destroy(hlist[id]->lock);
//	hlist[id] = NULL;
//	return PJ_SUCCESS;
//}
//
//pj_int32_t acore_event_resister_handle(pj_uint32_t event_id, pj_pool_t *pool,
//		acore_handle_p handle, acore_callback_clear clear, void *data) {
//	if (is_not_avaiable(event_id))
//		return -1;
//	pj_lock_acquire(hlist[event_id]->lock);
//	struct handle_list *po = pj_pool_alloc(pool, sizeof(struct handle_list));
//	acore_list_init_element(po);
//	pj_int32_t ret_id = po->id;
//	po->clear = clear;
//	po->user_data = data;
//	po->handle = handle;
//	acore_list_add(hlist[event_id]->list, po);
//	pj_lock_release(hlist[event_id]->lock);
//	return ret_id;
//}
//pj_bool_t acore_event_unregister_handle(pj_uint32_t event_id,
//		pj_uint32_t handle_id) {
//	if (is_not_avaiable(event_id))
//		return PJ_FALSE;
//	pj_bool_t ret = -1;
//	pj_lock_acquire(hlist[event_id]->lock);
//	ret = acore_list_remove_id(hlist[event_id]->list, handle_id);
//	pj_lock_release(hlist[event_id]->lock);
//	return ret;
//}
//struct evt_data {
//
//	int type;
//	void *data;
//};
//
//static pj_bool_t send_event(void *data, const acore_list_element *list) {
//	const struct handle_list *han = list;
//	struct evt_data *evt = (struct evt_data*) data;
//	han->handle(han->user_data, evt->type, evt->data);
//	return acore_ele_notfound;
//}
//void acore_event_send(pj_uint32_t event_id, int type, void *data) {
//	if (is_not_avaiable(event_id))
//		return;
//	pj_lock_acquire(hlist[event_id]->lock);
//	struct evt_data em = { .type = type, .data = data };
//	acore_list_search(hlist[event_id]->list, send_event, &em);
//	pj_lock_release(hlist[event_id]->lock);
//}
//
//ACORE_LOCAL void _acore_event_release(void) {
//	for (int i = 0; i < max_evt; i++) {
//		if (hlist[i] != NULL) {
//			acore_event_destroy(i);
//			hlist[i] = NULL;
//		}
//	}
//}
//
