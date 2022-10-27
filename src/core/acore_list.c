/*
 * acore_list.c
 *
 *  Created on: Aug 30, 2022
 *      Author: amneiht
 */

#include <acore/list.h>
#include <pjlib.h>
#include "local_core.h"
typedef struct core_list {
	ACORE_DECL_LIST_ELEMENT(struct core_list)
	;
} core_list;

acore_list_t* acore_list_create(pj_pool_t *pool, acore_list_element_cmp cmp) {
	acore_list_t *list = PJ_POOL_ZALLOC_T(pool, acore_list_t);
	list->cmp = cmp;
	pj_list_init(&list->array);
	return list;
}
//void acore_list_set_sort_rule(acore_list_t *list, acore_list_element_cmp cmp) {
//	list->cmp = cmp;
//}

int acore_list_size(acore_list_t *list) {
	return pj_list_size(&list->array);
}
pj_bool_t acore_list_is_empty(acore_list_t *list) {
	return pj_list_size(&list->array) == 0;
}
static int find_pos(void *data, const acore_list_element *list) {
	(void) list;
	int *pos = (int*) data;
	if (*pos < 1)
		return acore_ele_found;
	*pos = *pos - 1;
	return acore_ele_notfound;
}
acore_list_element* acore_list_element_at(acore_list_t *list, int pos) {
	if (pos < 0)
		return NULL;
	return acore_list_search(list, find_pos, &pos);
}
void acore_list_init_element(acore_list_element *list) {
	core_list *pl = (core_list*) list;
	pj_list_init(pl);
	pl->id = pj_rand();
	pl->clear = NULL;
	pl->user_data = NULL;
}
void* acore_list_ele_create(pj_pool_t *pool, ssize_t size) {
	core_list *pl = (core_list*) pj_pool_zalloc(pool, size);
	pj_list_init(pl);
	pl->id = pj_rand();
	pl->clear = NULL;
	pl->user_data = NULL;
	return pl;
}
void acore_list_add(acore_list_t *list, acore_list_element *node) {

	core_list *frist = (core_list*) list->array.next;
	core_list *last = (core_list*) &list->array;
	core_list *pos = (core_list*) node;
	if (frist == last || list->cmp == NULL) {
		pj_list_insert_after(frist, pos);
		return;
	}
	while (frist != last) {
		if (list->cmp(frist, pos) >= 0) {
			pj_list_insert_before(frist, pos);
			return;
		}
		frist = frist->next;
	}
// insert in last
	pj_list_insert_before(frist, pos);
}

pj_bool_t acore_list_remove(acore_list_t *list, acore_list_element *node) {
	if (node == NULL)
		return PJ_FALSE;
	core_list *pos = (core_list*) node;
	pj_list_erase(pos);
	if (pos->clear)
		pos->clear(pos->user_data);
	return PJ_TRUE;
}
static pj_bool_t compare_id(void *data, const acore_list_element *lst) {
	const core_list *t = (const core_list*) lst;
	pj_int32_t id = *((pj_int32_t*) data);
	if (t->id == id)
		return acore_ele_found;
	return acore_ele_notfound;
}
pj_bool_t acore_list_remove_id(acore_list_t *list, pj_int32_t id) {
	acore_list_element *pos = acore_list_search(list, compare_id, &id);
	return acore_list_remove(list, pos);
}

acore_list_element* acore_list_get_id(acore_list_t *list, pj_int32_t id) {
	return acore_list_search(list, compare_id, &id);
}
acore_list_element* acore_list_search(acore_list_t *list,
		int (*search)(void *data, const acore_list_element *list), void *data) {
	return pj_list_search(&list->array, data, search);
}

void acore_list_clear(acore_list_t *list) {
	core_list *alist = (core_list*) list->array.next;
	core_list *next, *last = (core_list*) &list->array;
	while (alist != last) {
		next = alist->next;
		pj_list_erase(alist);
		if (alist->clear)
			alist->clear(alist->user_data);
		alist = next;
	}
}
void acore_list_destroy(acore_list_t *list) {

	acore_list_clear(list);
}
