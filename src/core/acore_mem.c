#include <acore/mem.h>
#include "local_core.h"
static pj_rbtree mem_tree;

#ifdef ACORE_MEM_LOCK

static pj_lock_t *mem_lock;
#define _mbegin pj_lock_acquire( mem_lock ); {
#define _mend  } pj_lock_release( mem_lock );

#else

#define _mbegin {
#define _mend  }

#endif

struct core_mem {
	pj_bool_t des;
	int ref;
	acore_callback_clear clear;
	void *data;
	pj_rbtree_node *par;
};
static int mem_cmp(const void *k1, const void *k2) {
	long aa = (long) k1;
	long bb = (long) k2;
	return aa - bb;
}

acore_mmap_t* acore_mmap_create(pj_pool_t *pool) {
	pj_rbtree *rb = PJ_POOL_ALLOC_T(pool, pj_rbtree);
	pj_rbtree_init(rb, mem_cmp);
	return (void*) rb;
}

// F(p1)=p2
pj_status_t acore_mmap_set(pj_pool_t *pool, acore_mmap_t *map, void *p1,
		void *p2) {
	pj_rbtree_node *node = pj_rbtree_find((pj_rbtree*) map, p1);
	if (node != NULL)
		return -1;

	node = PJ_POOL_ALLOC_T(pool, pj_rbtree_node);
	node->key = p1;
	node->user_data = p2;
	pj_rbtree_insert((pj_rbtree*) map, node);
	return PJ_TRUE;
}
// get p2 from p1
void* acore_mmap_get(acore_mmap_t *map, void *p1) {
	pj_rbtree_node *node = pj_rbtree_find((pj_rbtree*) map, p1);
	if (node == NULL)
		return NULL;
	return node->user_data;
}

void* acore_mmap_unset(acore_mmap_t *map, void *p1) {
	pj_rbtree_node *node = pj_rbtree_find((pj_rbtree*) map, p1);
	if (node == NULL)
		return NULL;
	void *res = node->user_data;
	pj_rbtree_erase((pj_rbtree*) map, node);
	return res;
}

ACORE_LOCAL void _acore_mem_init(pj_pool_t *pool) {
	pj_rbtree_init(&mem_tree, mem_cmp);
#ifdef ACORE_MEM_LOCK
	pj_lock_create_recursive_mutex(pool, "mem_lock", &mem_lock);
#endif
}
ACORE_LOCAL void _acore_mem_close() {
// todo : release all data in tree
#ifdef ACORE_MEM_LOCK
	pj_lock_destroy(mem_lock);
#endif

}

static struct core_mem* find_mem(void *data) {
	pj_rbtree_node *rb = pj_rbtree_find(&mem_tree, data);
	if (rb != NULL) {
		return (struct core_mem*) rb->user_data;
	} else
		return NULL;
}
pj_status_t acore_mem_bind(pj_pool_t *pool, void *data,
		acore_callback_clear clear) {
	if (clear == NULL) {
		return -1;
	}
	struct core_mem *cmd;
	cmd = find_mem(data);
	if (cmd) {
		return PJ_SUCCESS;
	}
	pj_rbtree_node *rb = pj_pool_alloc(pool, sizeof(pj_rbtree_node));
	rb->key = data;
	struct core_mem *cmem = pj_pool_alloc(pool, sizeof(struct core_mem));
	cmem->data = data;
	cmem->clear = clear;
	cmem->ref = 0;
	cmem->des = PJ_FALSE;
	cmem->par = rb;
	rb->user_data = cmem;
	pj_rbtree_insert(&mem_tree, rb);
	return PJ_SUCCESS;
}
/**
 * add one to reference counter
 * @param data
 * @return
 */
pj_status_t acore_mem_add_ref(void *data) {
	struct core_mem *cmd;
	pj_status_t st = -1;
	cmd = find_mem(data);
	_mbegin
		if (cmd) {
			cmd->ref = cmd->ref + 1;
			st = PJ_SUCCESS;
		}

	_mend
	return st;
}

pj_status_t acore_mem_dec_ref(void *data) {
	struct core_mem *cmd;
	pj_status_t st = -1;

	cmd = find_mem(data);
	_mbegin
		if (cmd) {
			cmd->ref = cmd->ref - 1;
			if (cmd->ref < 0) {
				pj_rbtree_erase(&mem_tree, cmd->par);
				cmd->clear(cmd->data);
			}
			st = PJ_SUCCESS;
		}
// end dec counter
	_mend
	return st;
}
pj_bool_t acore_mem_is_available(void *data) {
	struct core_mem *cmd;
	pj_bool_t res = PJ_FALSE;

	cmd = find_mem(data);
	if (cmd) {
		res = !cmd->des;
	}
	return res;
}
pj_status_t acore_mem_is_bind(void *data) {
	struct core_mem *cmd;
	cmd = find_mem(data);
	return cmd != NULL;
}
pj_status_t acore_mem_mask_destroy(void *data) {
	struct core_mem *cmd;
	pj_status_t st = -1;
	cmd = find_mem(data);
	_mbegin
		if (cmd && !cmd->des) {
			cmd->ref = cmd->ref - 1;
			cmd->des = PJ_TRUE;
			if (cmd->ref < 0) {
				pj_rbtree_erase(&mem_tree, cmd->par);
				cmd->clear(cmd->data);
			}
			st = PJ_SUCCESS;
		}

	_mend
	return st;
}
