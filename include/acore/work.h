/*
 * control.h
 *
 *  Created on: Oct 10, 2022
 *      Author: amneiht
 */

#ifndef CORE_WORK_H_
#define CORE_WORK_H_

#include <pjlib.h>
#include <acore/base.h>
typedef struct acore_check_t acore_check_t;
typedef struct acore_work_t acore_work_t;

typedef int (*acore_work_check)(int type, void *work_data, void *user_data);
acore_work_t* acore_work_create(pj_pool_t *pool);
pj_bool_t acore_work_destroy(acore_work_t *work);
pj_int32_t acore_work_get_id(acore_work_t *work);

acore_check_t* acore_work_add_check(pj_pool_t *pool, pj_int32_t work_id,
		const char *name, acore_work_check check, acore_callback_clear clear,
		void *user_data);
pj_bool_t acore_work_remove_check(acore_check_t *check);

/**
 * Check if the job can be processed
 * @param work_id
 * @param mode
 * @param type
 * @param data
 * @return
 * mode = PJ_TRUE if more than one check is pass , the work can be proccess
 * mode = PJ_FALSE if only one check is not pass , the work can't be proccess
 */
pj_bool_t acore_work_test(pj_int32_t work_id, pj_bool_t mode, int type,
		void *data);
#endif /* CORE_WORK_H_ */
