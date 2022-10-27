/*
 * runtime.h
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#ifndef ACORE_RUNTIME_H_
#define ACORE_RUNTIME_H_

#include <pjlib.h>

#define ACORE_RUNTIME_DECL void  * link

void* acore_runtime_register_api(pj_pool_t *pool, const char *name, void *api,
		void (*clear)(void *api));
void acore_runtime_unregister_api(void *api);
// get copy api struct
const void* acore_runtime_get_clone(const char *name);
void acore_runtime_release_clone(const void *clone);

// check api return PJ_FALSE if it will be remove in future
pj_bool_t acore_runtime_check(const void *clone_api);

#endif /* ACORE_RUNTIME_H_ */
