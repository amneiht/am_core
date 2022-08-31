/*
 * event.h
 *
 *  Created on: Aug 17, 2022
 *      Author: amneiht
 */

#ifndef ACORE_ACORE_EVENT_H_
#define ACORE_ACORE_EVENT_H_

#include <acore_conf.h>
#include <pjlib.h>

#define acore_event_id 0

typedef enum acore_event_t {
	ACORE_CLOSE
} acore_event_t;

struct acore_event_listen {
	void (*handle)(void *user_data, int event_id, int type, void *event_data);
	void (*close)(void *user_data);
	void *user_data;
};
/// create custom event publicser for user
/// if pool is NULL they will crate a new pool
pj_int32_t acore_event_create(pj_pool_t *pool, const char *event_name);
// call close funtion all event handle
pj_status_t acore_event_destroy(pj_uint32_t event_id);

pj_int32_t acore_event_resister_handle(pj_uint32_t event_id, pj_pool_t *pool,
		const struct acore_event_listen *handle);

pj_status_t acore_event_unregister_handle(pj_uint32_t event_id,
		pj_uint32_t handle_id);

void acore_event_send(pj_uint32_t event_id, int type, void *data);

#endif /* ACORE_ACORE_EVENT_H_ */
