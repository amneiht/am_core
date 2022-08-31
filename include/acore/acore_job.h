/*
 * acore_work.h
 *
 *  Created on: Aug 26, 2022
 *      Author: amneiht
 */

#ifndef ACORE_ACORE_JOB_H_
#define ACORE_ACORE_JOB_H_
#include <acore.h>
// this is simplest simulation for QT signal and slots
// it help for loadable module work more esasy
// it very stupid
typedef pj_bool_t acore_callback_func(void *user_data, void *prarams);
typedef pj_bool_t acore_callback_clear(void *user_data);
typedef struct acore_job acore_job;
typedef struct acore_worker acore_worker;

acore_job* acore_job_create(pj_pool_t *pool,
		const char *name);

void acore_job_release(acore_job *chain);

acore_worker* acore_jod_add_worker(pj_pool_t *pool, acore_job *job,
		int priority,
		acore_callback_func fun, acore_callback_clear clear, void *user_data);

pj_bool_t acore_worker_leave(acore_worker *woker);

int acore_job_do(acore_job *job, void *pram_object);

#endif /* ACORE_ACORE_JOB_H_ */
