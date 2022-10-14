/*
 * runtime.c
 *
 *  Created on: Oct 1, 2022
 *      Author: amneiht
 */

#include <acore.h>

static pj_rbtree mtree;
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
void acore_runtime_register_api(pj_pool_t *pool, const char *name, void *api,
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
}
void acore_runtime_unregister_api(const char *name) {
	pj_str_t sname = pj_str((char*) name);
	pj_rbtree_node *node = pj_rbtree_find(&mtree, &sname);
	if (node == NULL)
		return;
	acore_event_send(acore_main_event_id(), ACORE_MODULE_CLOSE, (void*) name);
	pj_rbtree_erase(&mtree, node);
	struct m_api *l = node->user_data;
	if (l->clear)
		l->clear(l->api_p);
}

void* acore_runtime_get_api(const char *name) {
	pj_str_t sname = pj_str((char*) name);
	pj_rbtree_node *node = pj_rbtree_find(&mtree, &sname);
	if (node == NULL)
		return NULL;
	struct m_api *l = node->user_data;
	return l->api_p;
}
