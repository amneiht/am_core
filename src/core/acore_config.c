#include <config.h>

static acore_config_t default_conf = { // config object
		.max_event = ACORE_MAX_EVENT, //
				.timer_limit_callback = ACORE_TIMER_LIMITE_CALLBACK, //
				.thread_debug = PJ_FALSE };

/**
 * Remember never change value after acore_init funtion is call
 * @return
 */
acore_config_t* acore_config(void) {
	return &default_conf;
}
