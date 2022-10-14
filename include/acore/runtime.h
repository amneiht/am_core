/*
 * runtime.h
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#ifndef ACORE_RUNTIME_H_
#define ACORE_RUNTIME_H_

#include <pjlib.h>
void acore_runtime_register_api(pj_pool_t *pool, const char *name, void *api,
		void (*clear)(void *api));
void acore_runtime_unregister_api(const char *name);
void* acore_runtime_get_api(const char *name);

#endif /* ACORE_RUNTIME_H_ */
