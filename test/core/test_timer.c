
/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>
static void test_time(acore_timer_t *entry, void *arg) {
	(void) arg;
	(void) entry;
	PJ_LOG(1, ("thienma" ,"lll"));
	pj_thread_sleep(1200);
}

pj_bool_t ccc = PJ_TRUE;
pj_bool_t tloop(void *data) {
	return ccc;
}
static int p_thread(void *arg) {
	acore_timer_c *control = arg;
	acore_timer_loop(control, NULL, tloop);
	return 0;
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 2048, 2048);
	acore_timer_c *control = acore_timer_control_create(pool, 1000);
	acore_timer_t *timer = acore_timer_entry_create(pool, "test_timer", PJ_TRUE,
			1000, test_time, NULL);
	pj_thread_t *pthread;
	pj_thread_create(pool, "log_name", p_thread, control,
	PJ_THREAD_DEFAULT_STACK_SIZE, 0, &pthread);
	pj_thread_sleep(1000);
	acore_timer_register(control, timer);
	pj_thread_sleep(10000);
	acore_timer_entry_change_trigger_time(timer, 1000);
	pj_thread_sleep(10000);
	ccc = PJ_FALSE;
	pj_thread_join(pthread);
	acore_timer_control_destroy(control);
	acore_pool_release(pool);
	acore_close();
	return 0;
}
