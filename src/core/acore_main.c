/*
 * acore_main.c
 *
 *  Created on: Oct 4, 2022
 *      Author: amneiht
 */

#include <acore.h>

#include "local_core.h"
#if 0
/**
 * enter core thread
 * it must be call to avoid conflict data
 */
static void acore_main_enter() {

	if (acore_config()->thread_debug
			&& pj_lock_tryacquire(_acore_main_get()->lock) != PJ_SUCCESS) {
		PJ_LOG(1, (ACORE_NAME ,"thread is lock in anoter thread"));
	}
	pj_lock_acquire(_acore_main_get()->lock);
}
/**
 * leave core thread
 * it must be call after call acore_main_enter
 */
static void acore_main_leave() {
	pj_lock_release(_acore_main_get()->lock);
}

#endif
void acore_main_timer_register(acore_timer_t *timer) {
	acore_timer_register(_acore_main_get()->time_c, timer);
}
void acore_main_timer_unregister(acore_timer_t *timer_entry) {
	acore_timer_register(_acore_main_get()->time_c, timer_entry);

}
static pj_bool_t loop = PJ_FALSE;

static void evt_clear(void *data) {
	pj_pool_t *pool = data;
	acore_pool_release(pool);
}
static void evt_handle(void *user_data, int type, void *event_data) {
	if (type == ACORE_CLOSE) {
		loop = PJ_FALSE;
	}
}
static pj_bool_t is_close(void *arg) {
	return loop;
}
void acore_loop() {
	pj_pool_t *pool = acore_pool_create(NULL, 2048, 1024);
	acore_event_resister_handle(acore_main_event_id(), pool, evt_handle,
			evt_clear, pool);
	loop = PJ_TRUE;
	acore_timer_loop(_acore_main_get()->time_c, NULL, is_close);
}

