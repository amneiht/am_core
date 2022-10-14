/*
 * acore_list.h
 *
 *  Created on: Aug 30, 2022
 *      Author: amneiht
 */

#ifndef ACORE_LIST_H_
#define ACORE_LIST_H_

#include <config.h>
#include <pjlib.h>
#include <acore/base.h>
#ifdef __cplusplus
extern "C" {
#endif

#define ACORE_DECL_LIST_ELEMENT(type) PJ_DECL_LIST_MEMBER(type); \
pj_uint32_t id; \
acore_callback_clear clear; \
void *user_data

typedef enum acore_search_status {
	acore_ele_found = 0, acore_ele_notfound = 1,
} acore_search_status;
typedef struct acore_list_t acore_list_t;
typedef void acore_list_element;

/**
 * @return	0 ele1 = ele2
 *			1 ele1 > ele1
 *			-1  ele1 < ele2
 */
typedef pj_int32_t acore_list_element_cmp(const acore_list_element *ele1,
		const acore_list_element *ele2);

acore_list_t* acore_list_create(pj_pool_t *pool, acore_list_element_cmp cmp);

int acore_list_size(acore_list_t *list);
pj_bool_t acore_list_is_empty(acore_list_t *list);
acore_list_element* acore_list_element_at(acore_list_t *list, int pos);

void acore_list_init_element(acore_list_element *ele);
void* acore_list_ele_create(pj_pool_t *pool, ssize_t size);

void acore_list_add(acore_list_t *list, acore_list_element *node);

pj_bool_t acore_list_remove(acore_list_t *list, acore_list_element *node);
pj_bool_t acore_list_remove_id(acore_list_t *list, pj_int32_t id);

acore_list_element* acore_list_get_id(acore_list_t *list, pj_int32_t id);
// same as pj_list_seach
acore_list_element* acore_list_search(acore_list_t *list,
		int (*search)(void *data, const acore_list_element *list), void *data);

void acore_list_clear(acore_list_t *list);
void acore_list_destroy(acore_list_t *list);

#ifdef __cplusplus
}
#endif

#endif /* ACORE_LIST_H_ */
