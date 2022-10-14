#include <acore.h>
#include "test.h"

static void test_handle(void *user_data, int event_id, int type,
		void *event_data) {
	const char *lms = event_data;
	PJ_LOG(1, (ACORE_NAME , "ok %s",lms));
}
int main(int argc, char **argv) {
	acore_init();
	// register some help funtion
	pj_pool_t *pool = acore_pool_create(NULL, 2048, 2048);
	test_init(pool);

	struct test_api *test = acore_runtime_get_api("test");
	if (test != NULL) {
		test->print();
		test->pow(3);
	}
	struct acore_event_listen *hand = alloca(sizeof(struct acore_event_listen));
	hand->close = NULL;
	hand->handle = test_handle;
	acore_event_resister_handle(acore_main_event_id(), pool, hand);
	test_close();
	acore_pool_release(pool);
	acore_close();
	return 0;
}
