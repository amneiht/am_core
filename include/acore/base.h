/*
 * main.h
 *
 *  Created on: Aug 19, 2022
 *      Author: amneiht
 */

#ifndef ACORE_BASE_H_
#define ACORE_BASE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <config.h>
#include <acore/timer.h>
#include <pjlib.h>
#include <pjlib-util.h>

typedef pj_bool_t (*acore_callback_func)(void *user_data, void *prarams);
typedef void (*acore_callback_clear)(void *user_data);
// hiden funtion to other
#define ACORE_LOCAL __attribute__ ((visibility ("hidden")))
#define acore_alloca(type) (type *) alloca( sizeof(type))

#include <string.h>
#define ACORE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)  // file name
#define ACORE_LINE   __LINE__  // line in name
#define ACORE_FUNC	 __func__  // funtion name_

pj_status_t acore_init(void);
pj_status_t acore_close();

// data pool funtion
pj_pool_t* acore_pool_create(const char *name, int pool_size, int pool_inc);
void acore_pool_release(pj_pool_t *pool);
pj_pool_factory* acore_pool_factory();

pj_int32_t acore_main_event_id();

void acore_main_timer_register(acore_timer_t *timer);
void acore_main_timer_unregister(acore_timer_t *timer_entry);
void acore_loop();

/// lock core thread to avoid conflft data
void acore_start();
/// application must call it affter call acore_start()
void acore_end();

#ifdef __cplusplus
}
#endif

#endif /* ACORE_BASE_H_ */
