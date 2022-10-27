/*
 * event.h
 *
 *  Created on: Aug 17, 2022
 *      Author: amneiht
 */

#ifndef ACORE_EVENT_H_
#define ACORE_EVENT_H_

#include <acore/base.h>
#include <pjlib.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct acore_event_handle_t acore_event_handle_t;
typedef struct acore_event_t acore_event_t;

typedef enum acore_event {
	ACORE_MODULE_CLOSE, // data is const string
	ACORE_CLOSE, // data is NULL
	ACORE_MAX
} acore_event;

typedef void (*acore_handle_p)(void *user_data, int type, void *event_data);
/// create custom event publicser for user
/// if pool is NULL they will crate a new pool
acore_event_t* acore_event_create(pj_pool_t *pool, const char *event_name);
// call close funtion all event handle
pj_status_t acore_event_destroy(acore_event_t *evt);
pj_int32_t acore_event_get_id(acore_event_t *evt);

acore_event_handle_t* acore_event_resister_handle(pj_uint32_t event_id,
		pj_pool_t *pool, acore_handle_p handle, acore_callback_clear clear,
		void *data);

pj_status_t acore_event_unregister_handle(acore_event_handle_t *handle);
void acore_event_send(pj_uint32_t event_id, int type, void *data);

#ifdef __cplusplus
}
#endif

#endif /* ACORE_EVENT_H_ */
