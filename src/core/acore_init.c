/*
 * init.c
 *
 *  Created on: Aug 20, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include "local_core.h"
#include <pjlib-util.h>

static pj_caching_pool acore_cp;
static pj_bool_t core_init = PJ_FALSE;
static acore_main *core;

pj_status_t acore_init(void) {
	if (core_init)
		return core_init;
	core_init = PJ_TRUE;
	pj_status_t init = pj_init();
	pj_log_set_level(3);
	if (init != PJ_SUCCESS)
		return init;
	init = pjlib_util_init();
	if (init != PJ_SUCCESS)
		return init;
	//init memory pool

	pj_caching_pool_init(&acore_cp, &pj_pool_factory_default_policy, 102400);
	pj_pool_t *pool = pj_pool_create(&acore_cp.factory, "acore", 10240, 10240,
	NULL);

	core = pj_pool_alloc(pool, sizeof(acore_main));
	core->core_pool = pool;
	// random
	pj_time_val tm;
	pj_gettimeofday(&tm);
	pj_srand(tm.sec);
	// core lock
	pj_lock_create_recursive_mutex(core->core_pool, "core mutex", &core->lock);
	// init event handle
	_acore_runtime_init();
	_acore_event_init(core->core_pool);
	core->event = acore_event_create(core->core_pool, "core_project");
	_acore_ui_init(core->core_pool);
	core->time_c = acore_timer_control_create(core->core_pool, 1000);
	// woker
	_acore_work_init(pool);
	//init pasre
	_acore_context_init(pool);
	// mem
	_acore_mem_init(pool);
	return PJ_SUCCESS;
}

pj_int32_t acore_main_event_id() {
	return acore_event_get_id((acore_event_t*) core->event);
}
pj_status_t acore_close() {
	if (core_init) {
		_acore_event_release();
		_acore_ui_close();
		_acore_work_close();
		_acore_mem_close();
		acore_timer_control_destroy(core->time_c);
		pj_lock_destroy(core->lock);
		pj_pool_release(core->core_pool);
		pj_caching_pool_destroy(&acore_cp);
		pj_shutdown();
	}
	core_init = PJ_FALSE;
	return PJ_SUCCESS;
}

pj_pool_t* acore_pool_create(const char *name, int pool_size, int pool_inc) {
	return pj_pool_create(&acore_cp.factory, name, pool_size, pool_inc, NULL);
}
void acore_pool_release(pj_pool_t *pool) {
	pj_pool_release(pool);
}
pj_pool_factory* acore_pool_factory() {
	return &acore_cp.factory;
}
ACORE_LOCAL acore_main* _acore_main_get() {
	return core;
}
void acore_start() {
	pj_lock_acquire(core->lock);
}
void acore_end() {
	pj_lock_release(core->lock);
}

