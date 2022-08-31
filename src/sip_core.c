/*
 ============================================================================
 Name        : sip_core.c
 Author      : amneiht
 Version     :
 Copyright   : Your copyright notice
 Description : Hello World in C, Ansi-style
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include <pjlib.h>
#include <pjlib-util.h>


#define LOOP		16
#define MIN_COUNT	250
#define MAX_COUNT	(LOOP * MIN_COUNT)
#define MIN_DELAY	2
#define	D		(MAX_COUNT / 32000)
#define DELAY		(D < MIN_DELAY ? MIN_DELAY : D)
#define THIS_FILE	"timer_test"

pj_caching_pool cp;
pj_pool_factory *mem;

static void timer_callback(pj_timer_heap_t *ht, pj_timer_entry *e) {
	PJ_UNUSED_ARG(ht);
	PJ_UNUSED_ARG(e);
	puts("lol");
}

static int test_timer_heap(void) {
	int i, j;
	pj_timer_entry *entry;
	pj_pool_t *pool;
	pj_timer_heap_t *timer;
	pj_time_val delay;
	pj_status_t status;
	int err = 0;
	pj_size_t size;
	unsigned count;

	PJ_LOG(3, ("test", "...Basic test"));

	size = pj_timer_heap_mem_size(MAX_COUNT)
			+ MAX_COUNT * sizeof(pj_timer_entry);
	pool = pj_pool_create(mem, NULL, size, 4000, NULL);
	if (!pool) {
		PJ_LOG(3,
				("test", "...error: unable to create pool of %u bytes", size));
		return -10;
	}

	entry = (pj_timer_entry*) pj_pool_calloc(pool, MAX_COUNT, sizeof(*entry));
	if (!entry)
		return -20;

	for (i = 0; i < MAX_COUNT; ++i) {
		entry[i].cb = &timer_callback;
	}
	status = pj_timer_heap_create(pool, MAX_COUNT, &timer);
	if (status != PJ_SUCCESS) {
		PJ_LOG(1, (THIS_FILE ,"...error: unable to create timer heap"));
		return -30;
	}

	count = MIN_COUNT;
	for (i = 0; i < LOOP; ++i) {
		int early = 0;
		int done = 0;
		int cancelled = 0;
		int rc;
		pj_timestamp t1, t2, t_sched, t_cancel, t_poll;
		pj_time_val now, expire;

		pj_gettimeofday(&now);
		pj_srand(now.sec);
		t_sched.u32.lo = t_cancel.u32.lo = t_poll.u32.lo = 0;

		// Register timers
		for (j = 0; j < (int) count; ++j) {
			delay.sec = pj_rand() % DELAY;
			delay.msec = pj_rand() % 1000;

			// Schedule timer
			pj_get_timestamp(&t1);
			rc = pj_timer_heap_schedule(timer, &entry[j], &delay);
			if (rc != 0)
				return -40;
			pj_get_timestamp(&t2);

			t_sched.u32.lo += (t2.u32.lo - t1.u32.lo);

			// Poll timers.
			pj_get_timestamp(&t1);
			rc = pj_timer_heap_poll(timer, NULL);
			pj_get_timestamp(&t2);
			if (rc > 0) {
				t_poll.u32.lo += (t2.u32.lo - t1.u32.lo);
				early += rc;
			}
		}

		// Set the time where all timers should finish
		pj_gettimeofday(&expire);
		delay.sec = DELAY;
		delay.msec = 0;
		PJ_TIME_VAL_ADD(expire, delay);

		// Wait unfil all timers finish, cancel some of them.
		do {
			int index = pj_rand() % count;
			pj_get_timestamp(&t1);
			rc = pj_timer_heap_cancel(timer, &entry[index]);
			pj_get_timestamp(&t2);
			if (rc > 0) {
				cancelled += rc;
				t_cancel.u32.lo += (t2.u32.lo - t1.u32.lo);
			}

			pj_gettimeofday(&now);

			pj_get_timestamp(&t1);
#if defined(PJ_SYMBIAN) && PJ_SYMBIAN!=0
	    /* On Symbian, we must use OS poll (Active Scheduler poll) since
	     * timer is implemented using Active Object.
	     */
	    rc = 0;
	    while (pj_symbianos_poll(-1, 0))
		++rc;
#else
			rc = pj_timer_heap_poll(timer, NULL);
#endif
			pj_get_timestamp(&t2);
			if (rc > 0) {
				done += rc;
				t_poll.u32.lo += (t2.u32.lo - t1.u32.lo);
			}

		} while (PJ_TIME_VAL_LTE(now, expire) && pj_timer_heap_count(timer) > 0);

		if (pj_timer_heap_count(timer)) {
			PJ_LOG(3,
					(THIS_FILE, "ERROR: %d timers left", pj_timer_heap_count(timer)));
			++err;
		}
		t_sched.u32.lo /= count;
		t_cancel.u32.lo /= count;
		t_poll.u32.lo /= count;
		PJ_LOG(4,
				(THIS_FILE, "...ok (count:%d, early:%d, cancelled:%d, " "sched:%d, cancel:%d poll:%d)", count, early, cancelled, t_sched.u32.lo, t_cancel.u32.lo, t_poll.u32.lo));

		count = count * 2;
		if (count > MAX_COUNT)
			break;
	}

	pj_pool_release(pool);
	return err;
}

int mainlo(void) {
	puts("!!!Hello World!!!"); /* prints !!!Hello World!!! */

	pj_init();
	pj_caching_pool_init(&cp, &pj_pool_factory_default_policy, 102400);
	mem = &cp.factory;
	test_timer_heap();
	return EXIT_SUCCESS;
}

