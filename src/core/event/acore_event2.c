/*
 * event.c
 *
 *  Created on: Aug 20, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include <acore/acore_event.h>
#include <acore/acore_list.h>
#include <pjlib.h>
struct handle_list {
	ACORE_DECL_LIST_ELEMENT(struct handle_list)
	;
	void (*handle)(void *user_data, int event_id, int type, void *event_data);
};
struct event_list {
	pj_pool_t *pool;
	pj_str_t name;
	pj_lock_t *lock;
	acore_list_t *list;
};

static int max_evt;
static struct event_list **hlist;

void _acore_event_init(pj_pool_t *pool) {
	// create max event listener
	hlist = pj_pool_calloc(pool, acore_config()->max_event,
			sizeof(struct event_list*));
	max_evt = acore_config()->max_event;
}
pj_int32_t acore_event_create(pj_pool_t *pool, const char *event_name) {
	for (int i = 0; i < max_evt; i++) {
		if (hlist[i] == NULL) {
			hlist[i] = pj_pool_alloc(pool, sizeof(struct event_list));
			hlist[i]->pool = pool;
			pj_strdup2(pool, &hlist[i]->name, event_name);
			hlist[i]->list = acore_list_create(pool, NULL);
			pj_lock_create_simple_mutex(pool, event_name, &hlist[i]->lock);

			return i;
		}
	}
	return -1;
}
// call close funtion all event handle
pj_status_t acore_event_destroy(pj_uint32_t id) {
	if (id > max_evt || hlist[id] == NULL)
		return -1;
//	struct handle_list *point = hlist[id]->list;
	pj_lock_t *keep = hlist[id]->lock;
	pj_lock_acquire(keep);
	hlist[id]->lock = NULL;
//	if (point != NULL) {
//		do {
//			if (point->evt.close) {
//				point->evt.close(point->evt.user_data);
//			}
//			point = point->next;
//		} while (hlist[id]->list != point);
//	}
	acore_list_destroy(hlist[id]->list);
	pj_lock_release(keep);
	// release data
	pj_lock_destroy(keep);
	hlist[id] = NULL;
	return PJ_SUCCESS;
}

pj_int32_t acore_event_resister_handle(pj_uint32_t event_id, pj_pool_t *pool,
		const struct acore_event_listen *handle) {
	if (event_id > max_evt || hlist[event_id] == NULL)
		return -1;

	pj_int32_t ret_id = -1;
	if (hlist[event_id]->lock == NULL) {
		PJ_LOG(1, (ACORE_FUNC,"this event will be destroy soon"));
		return -1;
	}
	pj_lock_acquire(hlist[event_id]->lock);
	struct handle_list *po = pj_pool_alloc(pool, sizeof(struct handle_list));
	acore_list_init_element(po);
	ret_id = po->id;
//	pj_memcpy(&po->evt, handle, sizeof(struct acore_event_listen));
//	pj_list_init(po);
//	if (hlist[event_id]->list == NULL) {
//		hlist[event_id]->list = po;
//	} else {
//		pj_list_insert_after(hlist[event_id]->list, po);
//	}
	po->clear = handle->close;
	po->user_data = handle->user_data;
	po->handle = handle->handle;
	acore_list_add(hlist[event_id]->list, po);
	pj_lock_release(hlist[event_id]->lock);
	return ret_id;
}

pj_bool_t acore_event_unregister_handle(pj_uint32_t event_id,
		pj_uint32_t handle_id) {
	if (event_id > max_evt || hlist[event_id] == NULL)
		return -1;
	if (hlist[event_id]->lock == NULL) {
		PJ_LOG(1, (ACORE_FUNC,"this event will be destroy soon"));
		return PJ_FALSE;
	}
	if (hlist[event_id]->list == NULL)
		return PJ_FALSE;

	pj_bool_t ret = -1;
	pj_lock_acquire(hlist[event_id]->lock);
//	struct handle_list *point = hlist[event_id]->list;
//	do {
//		if (point->handle_id == handle_id) {
//			// remover first element
//			if (point == hlist[event_id]->list) {
//				// one element
//				if (point == point->next) {
//					hlist[event_id]->list = NULL;
//					break;
//				} else {
//					// change ancho to next element
//					hlist[event_id]->list = point->next;
//				}
//			}
//			pj_list_erase(point);
//			if (point->evt.close) {
//				point->evt.close(point->evt.user_data);
//			}
//			ret = PJ_SUCCESS;
//			break;
//		}
//		point = point->next;
//	} while (hlist[event_id]->list != point);
	ret = acore_list_remove_id(hlist[event_id]->list, handle_id);
	pj_lock_release(hlist[event_id]->lock);
	return ret;
}
struct evt_data {
	pj_uint32_t event_id;
	int type;
	void *data;
};

static pj_bool_t send_event(void *ele, void *user_data) {
	struct handle_list *han = (struct handle_list*) ele;
	struct evt_data *evt = (struct evt_data*) user_data;
	han->handle(han->user_data, evt->event_id, evt->type, evt->data);
	return PJ_FALSE;
}
void acore_event_send(pj_uint32_t event_id, int type, void *data) {
	if (event_id > max_evt || hlist[event_id] == NULL)
		return;

	if (hlist[event_id]->lock == NULL) {
		PJ_LOG(1, (ACORE_NAME,"this event will be destroy soon"));
		return;
	}
	pj_lock_acquire(hlist[event_id]->lock);
//	struct handle_list *point = hlist[event_id]->list;
//	do {
//		point->evt.handle(point->evt.user_data, event_id, type, data);
//		point = point->next;
//	} while (hlist[event_id]->list != point);
	struct evt_data em = { .event_id = event_id, .type = type, .data = data };
	acore_list_search(hlist[event_id]->list, send_event, &em);
	pj_lock_release(hlist[event_id]->lock);
}

void _acore_event_release(void) {
	for (int i = 0; i < max_evt; i++) {
		if (hlist[i] != NULL) {
			acore_event_destroy(i);
			hlist[i] = NULL;
		}
	}
}

