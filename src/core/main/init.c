/*
 * init.c
 *
 *  Created on: Aug 20, 2022
 *      Author: amneiht
 */

#include <acore.h>

#include <pjlib-util.h>
static pj_caching_pool acore_cp;
static pj_pool_t *core_pool;
static int evt_id;
extern void _acore_event_init(pj_pool_t *pool);
extern void _acore_event_release(void);

pj_status_t acore_init(void) {
	pj_status_t init = pj_init();
	if (init != PJ_SUCCESS)
		return init;
	init = pjlib_util_init();
	if (init != PJ_SUCCESS)
		return init;
	//init memory pool
	pj_caching_pool_init(&acore_cp, &pj_pool_factory_default_policy, 102400);
	core_pool = pj_pool_create(&acore_cp.factory, "acore", 4096, 4096, NULL);
	// random
	pj_time_val tm;
	pj_gettimeofday(&tm);
	pj_srand(tm.sec);
	// init event handle
	_acore_event_init(core_pool);
	evt_id = acore_event_create(core_pool, "core_project");
	return PJ_SUCCESS;
}

pj_int32_t acore_main_event_id() {
	return evt_id;
}
pj_status_t acore_close() {
	_acore_event_release();
	pj_pool_release(core_pool);
	pj_caching_pool_destroy(&acore_cp);
	pj_shutdown();
	return PJ_SUCCESS;
}

pj_pool_t* acore_pool_create(const char *name, int pool_size, int pool_inc) {
	return pj_pool_create(&acore_cp.factory, name, pool_size, pool_inc, NULL);
}
void acore_pool_release(pj_pool_t *pool) {
	pj_pool_release(pool);
}
