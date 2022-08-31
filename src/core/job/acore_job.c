/*
 * acore_job.c
 *
 *  Created on: Aug 27, 2022
 *      Author: amneiht
 */

#include <acore.h>
struct acore_worker {
	PJ_DECL_LIST_MEMBER(struct acore_worker)
	;
	pj_uint32_t prioty;
	acore_job *job;
	pj_bool_t (*fun)(void *user_data, void *prarams);
	pj_bool_t (*clear)(void *user_data);
	void *user_data;
};
struct acore_job {
	pj_pool_t *pool;
	pj_str_t name;
	pj_lock_t *lock;

	struct acore_worker *wlist;
};

acore_job* acore_job_create(pj_pool_t *pool, const char *name) {
	acore_job *job = pj_pool_alloc(pool, sizeof(acore_job));
	job->wlist = NULL;
	pj_lock_create_simple_mutex(pool, name, &job->lock);
	pj_strdup2(pool, &job->name, name);
	job->pool = pool;
	return job;
}

void acore_job_release(acore_job *job) {
	pj_lock_t *lock = job->lock;
	pj_lock_acquire(job->lock);
	job->lock = NULL;
	if (job->wlist != NULL) {
		acore_worker *point = job->wlist;
		do {
			if (point->clear) {
				point->clear(point->user_data);
			}
			point->job = NULL;
			point = point->next;
		} while (job->wlist != point);
	}
	job->wlist = NULL;
	pj_lock_release(lock);

	pj_lock_destroy(lock);
}
static acore_worker* list_insert(acore_worker *wlist, acore_worker *p) {
	pj_list_init(p);
	if (wlist == NULL)
		return p;
	if (p->prioty < wlist->prioty) {
		pj_list_insert_before(wlist, p);
		return p;
	}
	acore_worker *tmp = wlist->next;
	// check list
	while (tmp != wlist) {
		if (p->prioty < tmp->prioty) {
			pj_list_insert_before(tmp, p);
			return wlist;
		}
		tmp = tmp->next;
	}
	// create new note
	pj_list_insert_after(tmp, p);
	return wlist;
}
static acore_worker* list_del(acore_worker *wlist, acore_worker *p) {
	if (wlist == NULL)
		return NULL;
	acore_worker *ret = wlist;

	if (wlist == p) {
		if (wlist->next == p) {
			ret = NULL;
		} else {
			ret = p->next;
		}
	}
	// check list
	pj_list_erase(p);
	if (p->clear != NULL) {
		p->clear(p->user_data);
	}
	return ret;
}
acore_worker* acore_jod_add_worker(pj_pool_t *pool, acore_job *job,
		int priority, acore_callback_func fun, acore_callback_clear clear,

		void *user_data) {
	if (job->lock == NULL)
		return NULL;
	pj_lock_acquire(job->lock);
	acore_worker *p = PJ_POOL_ALLOC_T(pool, acore_worker);
	p->job = job;
	p->prioty = priority;
	p->fun = fun;
	p->clear = clear;
	pj_list_init(p);

	job->wlist = list_insert(job->wlist, p);
	pj_lock_release(job->lock);
	return p;
}

pj_bool_t acore_worker_leave(acore_worker *woker) {
	if (woker->job == NULL || woker->job->lock == NULL)
		return PJ_FALSE;

	pj_lock_acquire(woker->job->lock);
	woker->job->wlist = list_del(woker->job->wlist, woker);
	pj_lock_release(woker->job->lock);
	woker->job = NULL;
	return PJ_TRUE;
}

int acore_job_do(acore_job *job, void *pram_object) {
	if (job->lock == NULL || job->wlist == NULL)
		return -1;
	pj_lock_acquire(job->lock);
	acore_worker *tmp = job->wlist;
	pj_bool_t res = PJ_FALSE;
	do {
		res = tmp->fun(tmp->user_data, pram_object);
		tmp = tmp->next;
	} while (tmp != job->wlist && !res);
	pj_lock_release(job->lock);
	return 0;
}
