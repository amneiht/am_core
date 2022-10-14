/*
 * local_core.h
 *
 *  Created on: Oct 4, 2022
 *      Author: amneiht
 */

#ifndef CORE_LOCAL_CORE_H_
#define CORE_LOCAL_CORE_H_

#include <acore/base.h>
#define CORE_START acore_start() ; {
#define CORE_END  } acore_end();

// create pj_str_t from const string
#define convert_str(a,b) pj_str_t* a = alloca(sizeof(pj_str_t)) ; \
a->slen = strlen(b)+1; \
a->ptr = alloca(a->slen); \
pj_memcpy(a->ptr,b,a->slen)

typedef struct acore_main {
	pj_pool_t *core_pool;
	pj_lock_t *lock;
	void *event;
	acore_timer_c *time_c;
} acore_main;

struct acore_list_t {
	acore_list_element_cmp *cmp; // default null
	struct {
		PJ_DECL_LIST_MEMBER(struct core_list)
		;
	} array;
};
ACORE_LOCAL acore_main* _acore_main_get();

// event
ACORE_LOCAL void _acore_event_init(pj_pool_t *pool);
ACORE_LOCAL void _acore_event_release(void);
//ui
ACORE_LOCAL void _acore_ui_init(pj_pool_t *pool);
ACORE_LOCAL void _acore_ui_close();
// runtime
ACORE_LOCAL void _acore_runtime_init();

//work
ACORE_LOCAL void _acore_work_init(pj_pool_t *pool);
ACORE_LOCAL void _acore_work_close();
#endif /* CORE_LOCAL_CORE_H_ */
