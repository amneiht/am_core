/*
 * acore_list.h
 *
 *  Created on: Aug 30, 2022
 *      Author: amneiht
 */

#ifndef ACORE_ACORE_LIST_H_
#define ACORE_ACORE_LIST_H_

#include <acore_conf.h>
#include <pjlib.h>

#define ACORE_DECL_LIST_ELEMENT(type) \
PJ_DECL_LIST_MEMBER(type); \
pj_uint32_t id; \
void(*clear)(void *user_data); \
void *user_data

typedef struct acore_list_t acore_list_t;
typedef void acore_list_element;

/**
 * @return	0 ele1 = ele2
 *			1 ele1 > ele1
 *			-1  ele1 < ele2
 */
typedef pj_int32_t acore_list_element_cmp(acore_list_element *ele1,
		acore_list_element *ele2);

acore_list_t* acore_list_create(pj_pool_t *pool, acore_list_element_cmp *cmp);

int acore_list_size(acore_list_t *list);
acore_list_element* acore_list_element_at(acore_list_t *list, int pos);

void acore_list_init_element(acore_list_element *ele);
void acore_list_add(acore_list_t *list, acore_list_element *node);

pj_bool_t acore_list_remove(acore_list_t *list, acore_list_element *node);
pj_bool_t acore_list_remove_id(acore_list_t *list, pj_int32_t id);


acore_list_element* acore_list_search(acore_list_t *list,
		pj_bool_t (*search)(acore_list_element *list, void *data), void *data);

void acore_list_clear(acore_list_t *list);
void acore_list_destroy(acore_list_t *list);
#endif /* ACORE_ACORE_LIST_H_ */
