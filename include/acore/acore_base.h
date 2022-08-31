/*
 * main.h
 *
 *  Created on: Aug 19, 2022
 *      Author: amneiht
 */

#ifndef ACORE_ACORE_BASE_H_
#define ACORE_ACORE_BASE_H_

#include <acore_conf.h>
#include <pjlib.h>

pj_status_t acore_init(void);
pj_status_t acore_close();

// data pool funtion
pj_pool_t* acore_pool_create(const char *name, int pool_size, int pool_inc);
void acore_pool_release(pj_pool_t *pool);




#endif /* ACORE_ACORE_BASE_H_ */
