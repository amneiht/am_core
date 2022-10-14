/*
 * stdio.c
 *
 *  Created on: Oct 12, 2022
 *      Author: amneiht
 */

#include <acore/base.h>
#include <acore/ui.h>
#include <fcntl.h>
struct mod_std {
	pj_pool_t *pool;
	acore_timer_t *timer;
};

static void std_timer(acore_timer_t *entry, void *arg) {
	int c = getc(stdin);
	if (c != EOF) {
		char buffer[1024];
		fgets(buffer + 1, 1024, stdin);
		buffer[0] = c;
		pj_str_t in = pj_str(buffer);
		acore_user_input(&in);
		printf("129> ");
	}
}
void* mod_stdio_load() {
	pj_pool_t *pool = acore_pool_create("stdio", 2048, 2048);
	struct mod_std *mod = PJ_POOL_ALLOC_T(pool, struct mod_std);
	mod->timer = acore_timer_entry_create(pool, "std", PJ_TRUE, 200, std_timer,
	NULL);
#if __linux__
	fcntl(0, F_SETFL, fcntl(0, F_GETFL) | O_NONBLOCK);
#endif
	printf("129> ");
	acore_main_timer_register(mod->timer);
	return mod;
}
