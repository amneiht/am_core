/*
 * acore_mem.h
 *
 *  Created on: Oct 19, 2022
 *      Author: amneiht
 */

#ifndef ACORE_MEM_H_
#define ACORE_MEM_H_

#include <pjlib.h>
#include <acore/base.h>
// memmory control for object
// use it to avoid error when delete data is using
/**
 * bind pointer for memory control
 * @param pool pool to alloc data if
 * @param data pointer to data use wand to bind
 * @param clear cllear call back
 * @return
 */
pj_status_t acore_mem_bind(pj_pool_t *pool, void *data,
		acore_callback_clear clear);
/**
 * check data is areadly bind to memory control
 * @param data
 * @return
 */
pj_bool_t acore_mem_is_bind(void *data);
/**
 * add one to reference counter
 * @param data
 * @return
 */
pj_status_t acore_mem_add_ref(void *data);
/**
 * decrease reffer to one
 * clear call_back will call when reference couter is -1
 * @param data
 * @return
 */
pj_status_t acore_mem_dec_ref(void *data);

/**
 * check pointer if available to use
 * use must check it before add ref counter
 * @param data
 * @return
 */
pj_bool_t acore_mem_is_available(void *data);

/**
 * Mark the memory to be destroyed, the  acore_mem_is_available function
 * always returns false value when it call
 * @param data
 * @return
 */
pj_status_t acore_mem_mask_destroy(void *data);

/**
 * bind pointer for memory control
 * @param pool pool to alloc data if
 * @param data pointer to data use wand to bind
 * @param clear cllear call back
 * @return
 */
pj_status_t acore_mem_isbind(void *data);

// memory map funtion

typedef void acore_mmap_t;
acore_mmap_t* acore_mmap_create(pj_pool_t *pool);

// F(p1)=p2
pj_status_t acore_mmap_set(pj_pool_t *pool, acore_mmap_t *map, void *p1,
		void *p2);
// get p2 from p1
void* acore_mmap_get(acore_mmap_t *map, void *p1);

// get p2 from p1 abd remove to map
void* acore_mmap_unset(acore_mmap_t *map, void *p1);

#endif /* ACORE_MEM_H_ */
