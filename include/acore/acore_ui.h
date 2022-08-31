/*
 * acore_ui.h
 *
 *  Created on: Aug 24, 2022
 *      Author: amneiht
 */

#ifndef MODULE_ACORE_UI_H_
#define MODULE_ACORE_UI_H_


#include <pjlib.h>

enum acore_ui_result {
	acore_ui_failse_syntax = -1, // failse syntax
	acore_ui_ok = 0, // handle successfull
	acore_ui_notmatch = 0, // not match this module
};
typedef struct acore_handle_t {
	int arg; // muber of arg
	pj_str_t *argment_list;
} acore_handle_t;

struct acore_ui_funtion {
	char *name;
	void (*ui_handle)(void *user_data, acore_handle_t *option);
	void (*ui_clear)(void *user_data); // clear data when unregister optinal
	char *detail; // module info for help comand
};
/// declare a new module command for ui handle , return false if has a same module name in the pass
pj_bool_t acore_ui_declare(pj_pool_t *pool, const char *module_name);
/// remove all module feature on ui command
pj_bool_t acore_ui_undeclare(const char *module_name);

/// example when user input : sip test -d sip.vgisc.com -info "name is fake"
/// this parse data and call funtion "test" in sip module name
pj_bool_t acore_ui_register_funtion(pj_pool_t *pool,
		const char *module_name, struct acore_ui_funtion *funtion_list,
		int list_count);
pj_bool_t acore_ui_unregister_funtion(const char *module_name,
		struct acore_ui_funtion *funtion_list, int list_count);

pj_int32_t acore_ui_register_output(pj_pool_t *pool, const char *name,
		void (*output)(const pj_str_t *out));

pj_int32_t acore_ui_get_output_by_name(pj_str_t *name);
pj_int32_t acore_ui_get_output_by_name2(pj_str_t *name);

pj_int32_t acore_ui_unregister_output(pj_int32_t id);
pj_bool_t acore_ui_input_str(pj_int32_t uid, const char *str);
pj_bool_t acore_ui_output_str(pj_int32_t uid, const char *str);


// get option give by string , pos is use to search faste if has mult opotion
//example: set -b log=8 -f /var/tmp/test -b ec=8
/**
 * get get option given by string
 * @param data user input data
 * @param option "option to get"
 * @param pos start postion to find and return next postion if we has find this
 * @return option or null
 */
pj_str_t* acore_ui_get_option(acore_handle_t *data, char *option, int *pos);
#endif /* MODULE_ACORE_UI_H_ */
