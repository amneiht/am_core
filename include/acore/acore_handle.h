/*
 * handle.h
 *
 *  Created on: Aug 17, 2022
 *      Author: amneiht
 */

#ifndef ACORE_ACORE_HANDLE_H_
#define ACORE_ACORE_HANDLE_H_


typedef struct acore_handle_data {
	int arg;
	pj_str_t *in; /// tro den 1 mang
} acore_handle_data;

pj_bool_t acore_handle_find_option(acore_handle_data *handle, pj_bool_t);




#endif /* ACORE_ACORE_HANDLE_H_ */
