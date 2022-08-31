/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */
// test work mechaist
#include <acore.h>
struct pram_object {
	int id;
	int pri;
};
pj_bool_t work_1(void *user, void *param_object) {
	PJ_LOG(1, (ACORE_NAME , "work 1"));
	return PJ_FALSE;
}
pj_bool_t work_2(void *user, void *param_object) {
	PJ_LOG(1, (ACORE_NAME , "work 2"));
	return PJ_FALSE;
}
pj_bool_t work_4(void *user, void *param_object) {
	PJ_LOG(1, (ACORE_NAME , "work 4"));
	return PJ_FALSE;
}
pj_bool_t work_5(void *user, void *param_object) {
	PJ_LOG(1, (ACORE_NAME , "work 5"));
	return PJ_FALSE;
}
pj_bool_t work_3(void *user, void *param_object) {
	PJ_LOG(1, (ACORE_NAME , "work 3"));
	return PJ_TRUE;
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("pika", 2048, 2048);
	acore_job *job = acore_job_create(pool, "sip_job");
	acore_worker *w1 = acore_jod_add_worker(pool, job, 10, work_1, NULL,
	NULL);
	acore_worker *w2 = acore_jod_add_worker(pool, job, 1, work_2, NULL,
	NULL);
	acore_worker *w3 = acore_jod_add_worker(pool, job, 9, work_3, NULL,
	NULL);
	acore_jod_add_worker(pool, job, 9, work_4, NULL,
	NULL);
	acore_jod_add_worker(pool, job, 1, work_5, NULL,
	NULL);
	acore_job_do(job, NULL);
	acore_worker_leave(w3);
	acore_worker_leave(w2);
	acore_worker_leave(w3);
	puts("");
	acore_job_do(job, NULL);
//	acore_job_release(job);
	acore_job_do(job, NULL);
	pj_pool_release(pool);
	acore_close();
	return 0;
}
