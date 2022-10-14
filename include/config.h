/*
 * avoip_conf.h
 *
 *  Created on: Aug 17, 2022
 *      Author: amneiht
 */

#ifndef CONFIG_H_
#define CONFIG_H_

#include <pjlib.h>

#ifndef ACORE_MAX_EVENT
#define ACORE_MAX_EVENT 32
#endif

#ifndef ACORE_TIMER_LIMITE_CALLBACK
#define ACORE_TIMER_LIMITE_CALLBACK 512
#endif

#ifndef ACORE_MAX_UI
#define  ACORE_MAX_UI 32
#endif

typedef struct acore_config_t {
	pj_uint32_t max_event;  //  ACORE_MAX_EVENT
	pj_uint32_t timer_limit_callback; // ACORE_TIMER_LIMITE_CALLBACK
	pj_uint32_t max_ui; // ACORE_MAX_UI
	pj_bool_t thread_debug;
} acore_config_t;

acore_config_t* acore_config(void);
#endif /* CONFIG_H_ */
