/*
 * acore_ui.c
 *
 *  Created on: Aug 29, 2022
 *      Author: amneiht
 */

#include <acore/acore_ui.h>
#include <acore.h>

typedef struct acore_funtion_list {
	PJ_DECL_LIST_MEMBER(struct acore_funtion_list)
	;
	pj_str_t name;
	void (*ui_handle)(void *user_data, acore_handle_t *option);
	void (*ui_clear)(void *user_data); // clear data when unregister optinal
	pj_str_t detail; // module info for help comand
} funtion_list;

typedef struct acore_module_list {
	PJ_DECL_LIST_MEMBER(struct acore_module_list)
	;
	pj_str_t name;
	funtion_list *flist;
} module_list;


static pj_lock_t *ui_lock;
static module_list *ui_list;

static void clear_list(module_list *p) {
	if (p->flist == NULL)
		return;
	funtion_list *tmp = p->flist;
	do {
	} while (tmp != p->flist);
}

pj_bool_t acore_ui_declare(pj_pool_t *pool, const char *module_name) {
	if (module_name == NULL)
		return PJ_FALSE;
	pj_bool_t res = PJ_TRUE;
	pj_lock_acquire(ui_lock);
	module_list *ml = ui_list->next;
	while (ml != ui_list && res) {
		if (pj_strcmp2(&ml->name, module_name) == 0) {
			res = PJ_FALSE;
		}
		ml = ml->next;
	}
	if (res) {
		module_list *m2 = PJ_POOL_ALLOC_T(pool, module_list);
		pj_list_init(m2);
		pj_list_insert_before(ui_list, m2);
		pj_strdup2(pool, &m2->name, module_name);
		m2->flist = NULL;
	}
	pj_lock_release(ui_lock);
	return res;
}

pj_bool_t acore_ui_undeclare(const char *module_name) {
	if (module_name == NULL)
		return PJ_FALSE;
	pj_bool_t res = PJ_TRUE;
	pj_lock_acquire(ui_lock);
	module_list *ml = ui_list->next;
	while (ml != ui_list && res) {
		if (pj_strcmp2(&ml->name, module_name) == 0) {
			res = PJ_FALSE;
		}
		ml = ml->next;
	}
	pj_lock_release(ui_lock);
	return res;
}

void __acore_ui_init(pj_pool_t *pool) {
	ui_list = PJ_POOL_ALLOC_T(pool, module_list);
	pj_list_init(ui_list);
	ui_list->name.ptr = NULL;
	ui_list->name.slen = 0;
	ui_list->flist = NULL;
	pj_lock_create_simple_mutex(pool, "ui_lock", &ui_lock);
}

void __acore_ui_destroy(pj_pool_t *pool) {
	pj_lock_acquire(ui_lock);

	pj_lock_release(ui_lock);
}

pj_bool_t acore_ui_register_funtion(pj_pool_t *pool, const char *module_name,
		struct acore_ui_funtion *funtion_list, int list_count);
pj_bool_t acore_ui_unregister_funtion(const char *module_name,
		struct acore_ui_funtion *funtion_list, int list_count);

pj_int32_t acore_ui_register_output(pj_pool_t *pool, const char *name,
		void (*output)(const pj_str_t *out));

pj_int32_t acore_ui_get_output_by_name(pj_str_t *name);
pj_int32_t acore_ui_get_output_by_name2(pj_str_t *name);

pj_int32_t acore_ui_unregister_output(pj_int32_t id);
pj_bool_t acore_ui_input_str(pj_int32_t uid, const char *str);
pj_bool_t acore_ui_output_str(pj_int32_t uid, const char *str);
