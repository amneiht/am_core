#include <acore.h>

static void clear_check(void *user_data) {
	puts("clear");
}
static pj_bool_t test_check1(int type, void *work_data, void *user_data) {

	int z = *((int*) work_data);
	if (z > 10) {
		puts("check 1 ok");
		return PJ_TRUE;
	} else {
		puts("check 1 false");
		return PJ_FALSE;
	}
}
static pj_bool_t test_check2(int type, void *work_data, void *user_data) {
	int z = *((int*) work_data);
	if (z > 8) {
		puts("check 2 ok");
		return PJ_TRUE;
	} else {
		puts("check 2 false");
		return PJ_FALSE;
	}
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 2048, 2048);
	acore_work_t *work = acore_work_create(pool);
	acore_check_t *check = acore_work_add_check(pool, acore_work_get_id(work),
			"sss", test_check1, clear_check, NULL);
	acore_check_t *check2 = acore_work_add_check(pool, acore_work_get_id(work),
			"ppp", test_check2, clear_check, NULL);

	int z = 20;
	pj_bool_t res = acore_work_test(acore_work_get_id(work), PJ_FALSE, 0, &z);
	z = 26;
	res = acore_work_test(acore_work_get_id(work), PJ_FALSE, 0, &z);
	(void) check;
	PJ_UNUSED_ARG(check2);
	PJ_UNUSED_ARG(res);
	acore_work_destroy(work);
	acore_pool_release(pool);
	acore_close();
}
