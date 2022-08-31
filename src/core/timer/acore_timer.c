/*
 * time_control.c
 *
 *  Created on: Aug 19, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include <pjlib.h>
struct acore_timer_controler {
	pj_timer_heap_t *ht;
	pj_pool_t *pool;
	pj_lock_t *lock;
	pj_bool_t time_change;
	pj_int32_t delay;
};
struct acore_timer_type {
	pj_timer_entry entry;
	pj_str_t name;
	pj_bool_t is_loop;
	pj_timer_heap_t *ht;
	pj_uint64_t triger_time;
	pj_time_val last_triger;
	void (*timer_func)(struct acore_timer_type *entry, void *data);
	void *timer_data;
};

static void timer_callback(pj_timer_heap_t *timer_heap,
		struct pj_timer_entry *entry) {
	acore_timer_t *acore = entry->user_data;
	acore->timer_func(acore, acore->timer_data);
	// count ting jitter
	if (!acore->is_loop)
	{
		acore->last_triger.sec = 0;
		return;
	}
	pj_time_val delay, now;
	// giam so lan tinh toan
	delay.sec = (acore->triger_time) / 1000;
	delay.msec = (acore->triger_time) % 1000;
	pj_gettimeofday(&now);
	PJ_TIME_VAL_ADD(acore->last_triger, delay);
	PJ_TIME_VAL_ADD(delay, acore->last_triger);
	PJ_TIME_VAL_SUB(delay, now);

	if (PJ_TIME_VAL_MSEC(delay) < 0) {
		// vuot qua time out
		PJ_LOG(5,
				(ACORE_NAME, "\"%.*s\" has handle time is more than delay",(int)acore->name.slen , acore->name.ptr));
		delay.sec = (acore->triger_time) / 1000;
		delay.msec = (acore->triger_time) % 1000;
		acore->last_triger = now;
	}
	pj_timer_heap_schedule(timer_heap, entry, &delay);
}
acore_timer_c* acore_timer_control_create(pj_pool_t *pool, pj_int32_t ms) {

	acore_timer_c *heap = pj_pool_alloc(pool,
			sizeof(struct acore_timer_controler));
	heap->pool = pool;
	if (ms == 0)
		ms = 100;
	heap->delay = ms;
	heap->time_change = pj_lock_create_simple_mutex(pool, "heap_lock",
			&heap->lock);
	pj_timer_heap_create(pool, acore_config()->timer_limit_callback, &heap->ht);
	pj_timer_heap_set_lock(heap->ht, heap->lock, PJ_TRUE);
	return heap;

}
void acore_timer_control_destroy(acore_timer_c *timer) {
	pj_timer_heap_destroy(timer->ht);
}
void acore_timer_poll(acore_timer_c *control) {
	pj_time_val tm;
	tm.sec = control->delay / 1000;
	tm.msec = control->delay % 1000;
	if (pj_timer_heap_count(control->ht) > 0)
		pj_timer_heap_poll(control->ht, &tm);
}
void acore_timer_loop(acore_timer_c *control, void *user_data,
		pj_bool_t (*is_stop)(void *data)) {
	pj_time_val delay, jiter;
	delay.sec = control->delay / 1000;
	delay.msec = control->delay % 1000;
	control->time_change = PJ_FALSE;
	pj_time_val now, pre;
	// precalate caculate for next poll time
	pj_gettimeofday(&pre);
	while (is_stop(user_data)) {
		if (control->time_change) {
			delay.sec = control->delay / 1000;
			delay.msec = control->delay % 1000;
			control->time_change = PJ_FALSE;
		}
		if (pj_timer_heap_count(control->ht) > 0)
			pj_timer_heap_poll(control->ht, &delay);
		pj_gettimeofday(&now);
		// thoi gian du kien lan key tiep
		PJ_TIME_VAL_ADD(pre, delay);
		jiter = pre;
		PJ_TIME_VAL_SUB(jiter, now);
		if (PJ_TIME_VAL_MSEC(jiter) < 0) {
			pre = now;
			PJ_TIME_VAL_ADD(pre, delay);
		} else
			pj_thread_sleep(PJ_TIME_VAL_MSEC(jiter));
//		PJ_LOG(1, (ACORE_NAME ,"loop"));
	}

}
void acore_timer_register(acore_timer_c *control, acore_timer_t *entry) {
	pj_time_val delay;
	delay.sec = entry->triger_time / 1000;
	delay.msec = entry->triger_time % 1000;

	entry->ht = control->ht;
	pj_gettimeofday(&entry->last_triger);
	pj_timer_heap_schedule(control->ht, &entry->entry, &delay);

}

pj_bool_t acore_timer_unregister(acore_timer_t *timer_entry) {
	pj_timer_heap_cancel(timer_entry->ht, &timer_entry->entry);
	timer_entry->last_triger.sec = 0;
	return PJ_TRUE;
}

acore_timer_t* acore_timer_entry_create(pj_pool_t *pool, char *name,
		pj_bool_t is_loop, pj_int32_t delay,
		void (*timer_func)(acore_timer_t *entry, void *data),
		void *user_data) {
	struct acore_timer_type *entry = pj_pool_alloc(pool,
			sizeof(struct acore_timer_type));
	// init pj_entry
	pj_timer_entry_init(&entry->entry, 0, entry, timer_callback);
	entry->is_loop = is_loop;
	entry->timer_func = timer_func;
	entry->timer_data = user_data;
	entry->triger_time = delay;
	entry->last_triger.sec = 0;

	if (name != NULL) {
		pj_strdup2(pool, &entry->name, name);
	} else {
		pj_strdup2(pool, &entry->name, "timer_entry");
	}
	return entry;
}
void acore_timer_entry_setloop(acore_timer_t *entry, pj_bool_t is_loop) {
	entry->is_loop = is_loop;
}
pj_bool_t acore_timer_entry_isloop(acore_timer_t *entry) {
	return entry->is_loop;
}
void acore_timer_entry_change_trigger_time(acore_timer_t *entry,
		pj_int32_t delay) {
	entry->triger_time = delay;
}
pj_int32_t acore_timer_entry_get_trigger_time(acore_timer_t *entry) {
	return entry->triger_time;
}
pj_bool_t acore_timer_entry_is_active(acore_timer_t *entry) {
	return entry->last_triger.sec == 0;
}
