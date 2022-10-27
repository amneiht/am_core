/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>

void test_clear(void *data) {
	puts("cmnd");
}
int main(int argc, char **argv) {
	acore_init();

	pj_pool_t *pool = acore_pool_create("nULL", ACORE_POOL_SIZE,
	ACORE_POOL_SIZE);

	pj_str_t *ddd = pj_pool_alloc(pool, sizeof(pj_str_t));
	acore_mem_bind(pool, ddd, test_clear);

	if (acore_mem_is_bind(ddd)) {
		puts("test 2");
	}
	acore_mem_dec_ref(ddd);
	acore_mem_dec_ref(ddd);
	acore_mem_dec_ref(ddd);
	acore_mem_dec_ref(ddd);
	if (acore_mem_is_bind(ddd)) {
		puts("test 3");
	}
	acore_pool_release(pool);
	acore_close();
	return 0;
}
