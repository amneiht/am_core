/*
 * teet_runtime.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */


#include <acore.h>
#include "test.h"
static void t_print() {
	puts("sgbakjsciuas");
}
static void t_pow(int i) {
	int z = i * i;
	PJ_LOG(1, (ACORE_NAME,"pow of i is:%d",z));
}
void test_init(pj_pool_t *pool) {
	struct test_api *tp = pj_pool_alloc(pool, sizeof(struct test_api));
	tp->pow = t_pow;
	tp->print = t_print;
	acore_runtime_register_api(pool, "test", tp, NULL);
}
void test_close() {
	acore_runtime_unregister_api("test");
}
