/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>

pj_bool_t ccc = PJ_TRUE;
pj_int32_t event_id;
pj_bool_t tloop(void *data) {
	return ccc;
}
static void test_time(acore_timer_t *entry, void *arg) {
	(void) arg;
	(void) entry;
	acore_event_send(event_id, 0, NULL);

}
void test_handle(void *user_data, int event_id, int type, void *event_data) {
	PJ_LOG(1, (ACORE_NAME , "ok"));
}
void test_handle2(void *user_data, int event_id, int type, void *event_data) {
	PJ_LOG(1, (ACORE_NAME , "ok2 "));
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 4096, 2048);
	acore_timer_c *control = acore_timer_control_create(pool, 1000);
	acore_timer_t *timer = acore_timer_entry_create(pool, "test_timer", PJ_TRUE,
			1000, test_time, NULL);
	pj_int32_t id1, id2;
	event_id = acore_event_create(pool, "test_event");

	struct acore_event_listen evt;
	evt.close = NULL;
	evt.user_data = NULL;
	evt.handle = test_handle;
	id1 = acore_event_resister_handle(event_id, pool, &evt);

	evt.handle = test_handle2;
	id2 = acore_event_resister_handle(event_id, pool, &evt);

	acore_timer_register(control, timer);

//	acore_event_destroy(event_id);
//	acore_event_destroy(id2);
	acore_event_unregister_handle(event_id, id1);
	acore_timer_loop(control, NULL, tloop);
	acore_timer_control_destroy(control);
	acore_pool_release(pool);
	acore_close();

	return 0;
}
