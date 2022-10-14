/*
 * acore_ui.h
 *
 *  Created on: Aug 24, 2022
 *      Author: amneiht
 */

#ifndef MODULE_ACORE_UI_H_
#define MODULE_ACORE_UI_H_

#include <acore/list.h>
#include <pjlib.h>
#include <pjlib-util.h>

typedef void (*acore_ui_output_p)(void *ui_data, const pj_str_t *out);
typedef void (*acore_ui_handle_p)(void *user_data, pj_str_t *info,
		acore_ui_output_p out, void *ui_data);
typedef struct acore_ui_simple_opt {
	pj_str_t name;
	pj_str_t value;
} acore_ui_simple_opt;

typedef struct acore_ui_command_ele acore_ui_command_ele;
// simple prase out put //
/**
 * parse input from scanner to opt list
 * @param data input data
 * @param opt value list
 * @param opt_len legth of list
 * @return
 */
pj_bool_t acore_ui_simple_parse(pj_str_t *data, acore_ui_simple_opt *opt,
		int opt_len);

// ui command

void acore_user_input(const pj_str_t *in);
void acore_ui_input(void *data, acore_ui_output_p out, const pj_str_t *input);

pj_bool_t acore_ui_register_command(const char *module_name,
		acore_ui_command_ele **ele, int ele_count);

void acore_ui_unregister_command(acore_ui_command_ele **ele, int ele_count);

acore_ui_command_ele* acore_ui_create_command(pj_pool_t *pool,
		const pj_str_t *name, const pj_str_t *detail, acore_ui_handle_p handle,
		acore_callback_clear clear, void *user_data);
acore_ui_command_ele* acore_ui_create_command2(pj_pool_t *pool,
		const char *name, const char *detail, acore_ui_handle_p handle,
		acore_callback_clear clear, void *user_data);
#endif /* MODULE_ACORE_UI_H_ */
