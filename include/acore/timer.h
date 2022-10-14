/*
 * timer.h
 *
 *  Created on: Aug 19, 2022
 *      Author: amneiht
 */

#ifndef ACORE_TIMER_H_
#define ACORE_TIMER_H_

#include <config.h>
#include <pjlib.h>

typedef struct acore_timer_controler acore_timer_c; // timer controller
typedef struct acore_timer_type acore_timer_t;

acore_timer_c* acore_timer_control_create(pj_pool_t *pool, pj_int32_t ms);
void acore_timer_control_destroy(acore_timer_c *timer);
/**
 * triger all time handle int timer_controler if you done want create a new thread
 * @param control timerc controller
 */
void acore_timer_poll(acore_timer_c *control);

/// loop the timer
void acore_timer_loop(acore_timer_c *control, void *user_data,
		pj_bool_t (*is_stop)(void *data));
/**
 * get the waiting time by ms in timer loop
 */
pj_int64_t acore_timer_get_time(acore_timer_c*);

/*timer entry funtion */
/**
 * create timer entry
 * @param pool data pool
 * @param name name of timer for debug
 * @param timer_func call when timer is trigger
 * @param user_data
 * @return
 */
acore_timer_t* acore_timer_entry_create(pj_pool_t *pool, char *name,
		pj_bool_t is_loop, pj_int32_t delay,
		void (*timer_func)(acore_timer_t *entry, void *data), void *user_data);

void acore_timer_entry_setloop(acore_timer_t *entry, pj_bool_t is_loop);
pj_bool_t acore_timer_entry_isloop(acore_timer_t *entry);
void acore_timer_entry_change_trigger_time(acore_timer_t *entry,
		pj_int32_t delay);
pj_int32_t acore_timer_entry_get_trigger_time(acore_timer_t *entry);
pj_bool_t acore_timer_entry_is_active(acore_timer_t *entry);
/*end timer entry funtion */

/**
 * register new timer to timer controller
 * @param control
 * @param type
 * @return
 */
void acore_timer_register(acore_timer_c *control, acore_timer_t *type);

/**
 * unregister new timer to timer controller
 */
pj_bool_t acore_timer_unregister(acore_timer_t *timer_entry);

#endif /* ACORE_TIMER_H_ */
