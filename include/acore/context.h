/*
 * context.h
 *
 *  Created on: Oct 16, 2022
 *      Author: amneiht
 */

#ifndef ACORE_CONTEXT_H_
#define ACORE_CONTEXT_H_

#include <pjlib.h>

typedef struct acore_conf_t acore_conf_t;
typedef struct acore_context_t acore_context_t;

acore_conf_t* acore_conf_parse(const char *file);
acore_conf_t* acore_conf_prase2(char *buffer, long len);
void acore_conf_release(acore_conf_t*);

const char* acore_conf_get_path();
void acore_conf_set_path(const char *path);

void acore_conf_print(const acore_conf_t *conf);

acore_context_t* acore_conf_find_context(acore_conf_t *conf,
		const pj_str_t *name);
acore_context_t* acore_conf_find_context2(acore_conf_t *conf, const char *name);

// context funtion

acore_context_t* acore_context_next(const acore_context_t *context);
const pj_str_t* acore_context_name(const acore_context_t *context);
pj_bool_t acore_context_is_pattern(const acore_context_t *context);

const pj_str_t* acore_context_get_str2(const acore_context_t *context,
		const char *name);
const pj_str_t* acore_context_get_str(const acore_context_t *context,
		const pj_str_t *name);

int acore_context_get_int2(const acore_context_t *context, const char *name);
int acore_context_get_int(const acore_context_t *context, const pj_str_t *name);

double acore_context_get_double2(const acore_context_t *context,
		const char *name);
double acore_context_get_double(const acore_context_t *context,
		const pj_str_t *name);

void acore_context_list_value(const acore_context_t *context,
		const pj_str_t *name, void (*handle)(void *data, const pj_str_t *value),
		void *data);
void acore_context_list_value2(const acore_context_t *context, const char *name,
		void (*handle)(void *data, const pj_str_t *value), void *data);
void acore_context_print(const acore_context_t *context);

#endif /* ACORE_CONTEXT_H_ */
