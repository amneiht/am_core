/*
 * runtime.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#include <acore.h>

static pj_rbtree mtree;
struct core_api {
	pj_rbtree_node *link;
};
static int api_comp(const void *key1, const void *key2) {
	const pj_str_t *k1 = key1;
	const pj_str_t *k2 = key2;
	return pj_strcmp(k1, k2);
}
ACORE_LOCAL void _acore_runtime_init() {
	pj_rbtree_init(&mtree, api_comp);
}
struct m_api {
	void *api_p;
	void (*clear)(void *api);
};

static void clear_api(void *data) {
	struct m_api *mapi = data;
	struct core_api *core = mapi->api_p;
	pj_rbtree_erase(&mtree, core->link);
	if (mapi->clear)
		mapi->clear(mapi->api_p);
}
void* acore_runtime_register_api(pj_pool_t *pool, const char *name, void *api,
		void (*clear)(void *api)) {
	pj_rbtree_node *node = PJ_POOL_ALLOC_T(pool, pj_rbtree_node);
	struct m_api *api_t = PJ_POOL_ALLOC_T(pool, struct m_api);
	api_t->clear = clear;
	api_t->api_p = api;
	pj_str_t *key = PJ_POOL_ALLOC_T(pool, pj_str_t);
	pj_strdup2_with_null(pool, key, name);
	node->key = key;
	node->user_data = api_t;
	pj_rbtree_insert(&mtree, node);
	acore_mem_bind(pool, api_t, clear_api);
	struct core_api *core = api;
	core->link = node;
	return api_t;
}
void acore_runtime_unregister_api(void *api) {
	acore_mem_mask_destroy(api);
}
const void* acore_runtime_get_clone(const char *name) {
	pj_str_t sname = pj_str((char*) name);
	pj_rbtree_node *node = pj_rbtree_find(&mtree, &sname);
	if (node == NULL)
		return NULL;
	struct m_api *l = node->user_data;
	if (acore_mem_is_available(l)) {
		acore_mem_add_ref(l);
		return l->api_p;
	}
	return NULL;
}
void acore_runtime_release_clone(const void *clone) {
	struct core_api *core = (void*) clone;
	acore_mem_dec_ref(core->link->user_data);
}
pj_bool_t acore_runtime_check(const void *clone) {
	struct core_api *core = (void*) clone;
	return acore_mem_is_available(core->link->user_data);
}
