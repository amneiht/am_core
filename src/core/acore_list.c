/*
 * acore_list.c
 *
 *  Created on: Aug 30, 2022
 *      Author: amneiht
 */

#include <acore/acore_list.h>
#include <pjlib.h>
typedef struct core_list {
	ACORE_DECL_LIST_ELEMENT(struct core_list)
	;
} core_list;
struct acore_list_t {
	core_list *ele;
	acore_list_element_cmp *cmp; // default null
};

acore_list_t* acore_list_create(pj_pool_t *pool, acore_list_element_cmp *cmp) {
	acore_list_t *list = PJ_POOL_ZALLOC_T(pool, acore_list_t);
	list->cmp = cmp;
	return list;
}
void acore_list_set_sort_rule(acore_list_t *list, acore_list_element_cmp *cmp) {
	list->cmp = cmp;
}

int acore_list_size(acore_list_t *list) {
	if (list->ele == NULL)
		return 0;
	return pj_list_size(list->ele);
}
static pj_bool_t find_id(void *lst, void *arg) {
	(void) lst;
	int *pos = (int*) arg;
	if (*pos < 1)
		return PJ_TRUE;
	*pos = *pos - 1;
	return PJ_FALSE;
}
acore_list_element* acore_list_element_at(acore_list_t *list, int pos) {
	if (pos < 0)
		return NULL;
	return acore_list_search(list, find_id, &pos);
}
void acore_list_init_element(acore_list_element *list) {
	core_list *pl = (core_list*) list;
	pj_list_init(pl);
	pl->id = pj_rand();
	pl->clear = NULL;
}
void acore_list_add(acore_list_t *list, acore_list_element *node) {
	if (node == list->ele || node == NULL)
		return;
	core_list *frist = list->ele;
	core_list *pos = (core_list*) node;
	core_list *tmp;
	pj_bool_t inset = PJ_FALSE;
	if (list->ele == NULL) {
		pj_list_init(node);
		list->ele = pos;
	} else if (list->cmp == NULL) {
		pj_list_insert_before(frist, pos);
	} else {
		if (list->cmp(frist, pos) > 0) {
			pj_list_insert_before(frist, pos);
			list->ele = pos;
			return;
		}
		tmp = frist->next;
		while (tmp != frist) {
			//tmp > pos
			if (list->cmp(tmp, pos) > 0) {
				pj_list_insert_before(tmp, pos);
				inset = PJ_TRUE;
				break;
			}
			tmp = tmp->next;
		}
		if (!inset) {
			pj_list_insert_before(frist, pos);
		}

	}
}

pj_bool_t acore_list_remove(acore_list_t *list, acore_list_element *node) {
	if (node == NULL || list->ele == NULL) {
		return PJ_FALSE;
	}
	core_list *frist = list->ele;
	core_list *pos = (core_list*) node;
	if (frist == pos) {
		if (frist->next == frist) {
			if (frist->clear) {
				frist->clear(frist->user_data);
			}
			list->ele = NULL;
		} else {
			list->ele = frist->next;
		}
	}
	pj_list_erase(pos);
	if (pos->clear)
		pos->clear(pos->user_data);
	return PJ_TRUE;
}
static pj_bool_t compare_id(acore_list_element *lst, void *arg) {
	core_list *t = (core_list*) lst;
	pj_int32_t id = *((pj_int32_t*) arg);
	return t->id == id;
}
pj_bool_t acore_list_remove_id(acore_list_t *list, pj_int32_t id) {
	acore_list_element *pos = acore_list_search(list, compare_id, &id);
	return acore_list_remove(list, pos);
}
acore_list_element* acore_list_search(acore_list_t *list,
		pj_bool_t (*search)(acore_list_element *list, void *data), void *data) {
	if (list->ele == NULL)
		return NULL;
	core_list *alist = list->ele;
	core_list *next;
	do {
		next = alist->next;
		if (search((void*) alist, data)) {
			return alist;
		}
		alist = next;
	} while (alist != list->ele);
	return NULL;
}

static pj_bool_t clear_list(void *lst, void *arg) {
	core_list *t = (core_list*) lst;
	(void) arg;
	pj_list_init(lst);
	if (t->clear)
		t->clear(t->user_data);

	return PJ_FALSE;
}
void acore_list_clear(acore_list_t *list) {
	acore_list_search(list, clear_list, NULL);
	list->ele = NULL;
}
void acore_list_destroy(acore_list_t *list) {
	acore_list_clear(list);
}
