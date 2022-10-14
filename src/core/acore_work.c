/*
 * work.c
 *
 *  Created on: Oct 10, 2022
 *      Author: amneiht
 */

#include <acore/work.h>
#include <acore/list.h>
#include "local_core.h"

static acore_list_t *work_list;

struct acore_check_t {
	ACORE_DECL_LIST_ELEMENT(struct acore_check_t)
	;
	pj_str_t name;
	acore_work_check check;
	acore_work_t *parent;
};
struct acore_work_t {
	ACORE_DECL_LIST_ELEMENT(struct acore_work_t)
	;
	acore_list_t *check;
};
ACORE_LOCAL void _acore_work_init(pj_pool_t *pool) {
	work_list = acore_list_create(pool, NULL);
}
ACORE_LOCAL void _acore_work_close() {
	CORE_START
		acore_list_destroy(work_list);
	CORE_END
}

static void work_clear(void *work) {
	acore_work_t *list = work;
	acore_list_destroy(list->check);
}
acore_work_t* acore_work_create(pj_pool_t *pool) {
	acore_work_t *work = PJ_POOL_ZALLOC_T(pool, acore_work_t);
	acore_list_init_element(work);
	work->check = acore_list_create(pool, NULL);
	work->clear = work_clear;
	work->user_data = work;
	CORE_START
		acore_list_add(work_list, work);
	CORE_END
	return work;
}

pj_int32_t acore_work_get_id(acore_work_t *work) {
	return work->id;
}
acore_check_t* acore_work_add_check(pj_pool_t *pool, pj_int32_t work_id,
		const char *name, acore_work_check check, acore_callback_clear clear,
		void *user_data) {
	acore_work_t *wk = acore_list_get_id(work_list, work_id);
	if (wk == NULL) {
		PJ_LOG(5, (ACORE_NAME, "cannot found id :%d",work_id));
		return NULL;
	}
	acore_check_t *wc = PJ_POOL_ALLOC_T(pool, acore_check_t);
	acore_list_init_element(wc);
	wc->check = check;
	wc->clear = clear;
	pj_strdup2_with_null(pool, &wc->name, name);
	wc->user_data = user_data;
	CORE_START
		wc->parent = wk;
		acore_list_add(wk->check, wc);
	CORE_END
	return wc;
}
pj_bool_t acore_work_remove_check(acore_check_t *check) {
	if (check == NULL) {
		PJ_LOG(1, (ACORE_FUNC, "cannot remove NULL"));
		return PJ_FALSE;
	}
	CORE_START
		acore_list_remove(check->parent->check, check);
	CORE_END
	return PJ_TRUE;
}
pj_bool_t acore_work_destroy(acore_work_t *work) {
	if (work == NULL) {
		PJ_LOG(1, (ACORE_FUNC, "cannot remove NULL"));
		return PJ_FALSE;
	}
	PJ_LOG(5, (ACORE_FUNC, "Remove Work with id:%d",work->id));
	CORE_START
		acore_list_remove(work_list, work);
	CORE_END
	return PJ_TRUE;
}
struct check_value {
	int type;
	pj_bool_t mode;
	void *data;
};
static int work_check(void *value, const void *node) {
	struct check_value *sc = value;
	const acore_check_t *check = node;
	pj_bool_t res = check->check(sc->type, sc->data, check->user_data);
	if (sc->mode) {
		// test is pass
		return !res;
	}

	return res;
}
pj_bool_t acore_work_test(pj_int32_t work_id, pj_bool_t mode, int type,
		void *data) {
	acore_work_t *wk = acore_list_get_id(work_list, work_id);
	if (wk == NULL) {
		PJ_LOG(5, (ACORE_NAME, "cannot found id :%d",work_id));
		return PJ_TRUE;
	}
	struct check_value *sc = acore_alloca(struct check_value);
	sc->type = type;
	sc->data = data;
	sc->mode = mode;
	acore_check_t *check = NULL;
	CORE_START
		check = acore_list_search(wk->check, work_check, sc);
	CORE_END
	if (mode)
		return check != NULL;
	else
		return check == NULL;
}
