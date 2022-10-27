/*
 * acore_conf.h
 *
 *  Created on: Sep 25, 2022
 *      Author: amneiht
 */

#ifndef ACORE_CONF_H_
#define ACORE_CONF_H_

#if 0

#include <pjlib.h>
#include <pjlib-util.h>

typedef struct acore_conf_t acore_conf_t;

acore_conf_t* acore_conf_parse(const char *file);
acore_conf_t* acore_conf_prase2(char *buffer, long len);
void acore_conf_release(acore_conf_t*);

pj_str_t* acore_conf_get_str2(acore_conf_t *conf, const char *name);
pj_str_t* acore_conf_get_str(acore_conf_t *conf, const pj_str_t *name);

int acore_conf_get_int2(acore_conf_t *conf, const char *name);
int acore_conf_get_int(acore_conf_t *conf, const pj_str_t *name);

double acore_conf_get_double2(acore_conf_t *conf, const char *name);
double acore_conf_get_double(acore_conf_t *conf, const pj_str_t *name);

void acore_conf_list(acore_conf_t *conf, const pj_str_t *name,
		void (*handle)(void *data, const pj_str_t *value), void *data);
void acore_conf_list2(acore_conf_t *conf, const char *name,
		void (*handle)(void *data, const pj_str_t *value), void *data);

void acore_conf_print(acore_conf_t *conf);

#endif
#endif /* ACORE_CONF_H_ */
