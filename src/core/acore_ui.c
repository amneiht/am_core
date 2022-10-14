/*
 * acore_ui.c
 *
 *  Created on: Aug 29, 2022
 *      Author: amneiht
 */

#include <acore/list.h>
#include <acore/ui.h>
#include <acore.h>
// todo : convert to red back tree for big system
#define ui_base(type) \
ACORE_DECL_LIST_ELEMENT(struct type) ; \
pj_str_t name
struct acore_ui_command_ele {
	// begin interface //
	ui_base(acore_ui_command_ele)
	;
	// end interface //
	pj_str_t help_info;
	acore_ui_handle_p handle;
	void *parrent;
};
typedef struct ui_element {
	// begin interface //
	ui_base(ui_element)
	;
	// end interface //
	pj_pool_t *pool;
	acore_list_t *func_list;
	pj_lock_t *lock;
} ui_element;

static acore_list_t *core_ui;
static pj_lock_t *ui_lock;

static void ui_clear(void *user_data) {
	ui_element *lps = user_data;
	PJ_LOG(4, (ACORE_NAME,"clear ui:%.*s",(int)lps->name.slen , lps->name.ptr));
	pj_lock_acquire(lps->lock);
	acore_list_destroy(lps->func_list);
	pj_lock_release(lps->lock);
	acore_pool_release(lps->pool);
}
static ui_element* create_ui() {
	pj_pool_t *pool = acore_pool_create(NULL, 2048, 2048);
	ui_element *ret = PJ_POOL_ALLOC_T(pool, ui_element);
	acore_list_init_element(ret);
	ret->pool = pool;
	ret->name.slen = 0;
	ret->func_list = acore_list_create(pool, NULL);
	ret->user_data = ret;
	ret->clear = ui_clear;
	pj_lock_create_recursive_mutex(pool, "module_ui", &ret->lock);
	return ret;
}
static void output_p(void *ui_data, const pj_str_t *out) {
	PJ_UNUSED_ARG(ui_data);
//	PJ_LOG(1, ("","%.*s", (int) out->slen, out->ptr));
	fprintf(stdout, "%.*s\n", (int) out->slen, out->ptr);
}
struct ui_out {
	acore_ui_output_p out;
	void *ui_data;
};
static pj_bool_t print_ui(void *data, const acore_list_element *ele) {
	struct ui_out *out = data;
	const ui_element *ui = (const ui_element*) ele;
	int sl = acore_list_size(ui->func_list);
	char buff[500];
	pj_str_t wr;
	wr.ptr = buff;
	acore_ui_command_ele *frist = acore_list_element_at(ui->func_list, 0);
	acore_ui_command_ele *p = frist;
	if (ui->name.slen > 0)
		wr.slen = sprintf(buff, "module %.*s:", (int) ui->name.slen,
				ui->name.ptr);
	else {
		wr.slen = sprintf(buff, "base funtion:");
	}
	out->out(out->ui_data, &wr);
	while (sl > 0) {
		wr.slen = sprintf(buff, "\t%.*s:\t\t%.*s", (int) p->name.slen,
				p->name.ptr, (int) p->help_info.slen, p->help_info.ptr);
		out->out(out->ui_data, &wr);
		sl--;
		p = p->next;
	}
	return acore_ele_notfound;
}

static void print_help(void *user_data, pj_str_t *info, acore_ui_output_p out,
		void *ui_data) {
	PJ_UNUSED_ARG(user_data);
	PJ_UNUSED_ARG(info);
	struct ui_out ui_p = { out, ui_data };
	acore_list_search(core_ui, print_ui, &ui_p);
}
static void set_log(void *user_data, pj_str_t *info, acore_ui_output_p out,
		void *ui_data) {
	PJ_UNUSED_ARG(user_data);
	acore_ui_simple_opt spo;
	spo.name = pj_str("set");
	pj_bool_t sl = acore_ui_simple_parse(info, &spo, 1);
	if (sl) {
		char buff[spo.value.slen + 1];
		sprintf(buff, "%.*s", (int) spo.value.slen, spo.value.ptr);
		int s = atoi(buff);
		pj_log_set_level(s);
		PJ_LOG(3, ("log","set log_level to %d",s));
	}

}
static void ui_exit(void *user_data, pj_str_t *info, acore_ui_output_p out,
		void *ui_data) {
	acore_event_send(acore_main_event_id(), ACORE_CLOSE, NULL);
}
ACORE_LOCAL void _acore_ui_init(pj_pool_t *pool) {
	// list is no sort
	core_ui = acore_list_create(pool, NULL);
	pj_lock_create_recursive_mutex(pool, "ui_lock", &ui_lock);
	// register some help funtion
	acore_ui_command_ele *ele[3];
	ele[0] = acore_ui_create_command2(pool, "help", "show all command",
			print_help, NULL, NULL);
	ele[1] = acore_ui_create_command2(pool, "log", "log set <0-6>", set_log,
	NULL, NULL);
	ele[2] = acore_ui_create_command2(pool, "exit", "exit program", ui_exit,
	NULL, NULL);
	acore_ui_register_command("core", ele, 3);
}
ACORE_LOCAL void _acore_ui_close() {
	pj_lock_acquire(ui_lock);
	acore_list_destroy(core_ui);
	core_ui = NULL;
	pj_lock_release(ui_lock);
	pj_lock_destroy(ui_lock);
	ui_lock = NULL;
}
/// remove all module feature on ui command
static int find_ele(void *data, const acore_list_element *list) {
	const ui_element *ui = (ui_element*) list;
	pj_str_t *name = data;
	if (pj_strcmp(&ui->name, name) == 0)
		return acore_ele_found;
	return acore_ele_notfound;
}

pj_bool_t acore_ui_register_command(const char *module_name,
		acore_ui_command_ele **ele, int ele_count) {
	pj_str_t name;
	name.slen = 0;

	if (ele == NULL) {
		PJ_LOG(1, (ACORE_NAME,"ui element is NULL"));
	}
	if (module_name != NULL)
		name = pj_str((char*) module_name);

	ui_element *ui = acore_list_search(core_ui, find_ele, &name);
	if (ui == NULL) {
		ui = create_ui();
		pj_lock_acquire(ui_lock);
		pj_strdup(ui->pool, &ui->name, &name);
		acore_list_add(core_ui, ui);
		pj_lock_release(ui_lock);
	}
	if (ui == NULL)
		return PJ_FALSE;

	pj_lock_acquire(ui->lock);
	for (int i = 0; i < ele_count; i++) {
		ele[i]->parrent = ui;
		acore_list_add(ui->func_list, ele[i]);
	}
	pj_lock_release(ui->lock);
	return PJ_TRUE;
}

void acore_ui_unregister_command(acore_ui_command_ele **ele, int ele_count) {
	for (int i = 0; i < ele_count; i++) {
		ui_element *ui = ele[i]->parrent;
		pj_lock_acquire(ui->lock);
		ele[i]->parrent = NULL;
		acore_list_remove(ui->func_list, ele[i]);
		pj_lock_release(ui->lock);
		if (acore_list_is_empty(ui->func_list)) {
			pj_lock_acquire(ui_lock);
			acore_list_remove(core_ui, ui);
			pj_lock_release(ui_lock);
		}
	}
}
acore_ui_command_ele* acore_ui_create_command2(pj_pool_t *pool,
		const char *name, const char *detail, acore_ui_handle_p handle,
		acore_callback_clear clear, void *user_data) {
	pj_str_t name1 = pj_str((char*) name);
	pj_str_t dt2 = pj_str((char*) detail);
	return acore_ui_create_command(pool, &name1, &dt2, handle, clear, user_data);
}
acore_ui_command_ele* acore_ui_create_command(pj_pool_t *pool,
		const pj_str_t *name, const pj_str_t *detail, acore_ui_handle_p handle,
		acore_callback_clear clear, void *user_data) {
	acore_ui_command_ele *ele = PJ_POOL_ZALLOC_T(pool, acore_ui_command_ele);
	if (!ele) {
		PJ_LOG(4, (ACORE_FUNC ,"not enough memory"));
		return NULL;
	}
	acore_list_init_element(ele);
	ele->parrent = NULL;
	pj_strdup(pool, &ele->name, name);
	if (detail != NULL) {
		pj_strdup(pool, &ele->help_info, detail);
	}
	ele->handle = handle;
	ele->clear = clear;
	ele->user_data = user_data;
	return ele;
}

void acore_user_input(const pj_str_t *in) {
	acore_ui_input(NULL, NULL, in);
}
static void sc_callback(pj_scanner *scanner) {
	(void) scanner;
	pj_scan_get_char(scanner);
}
void acore_ui_input(void *data, acore_ui_output_p out, const pj_str_t *input) {
	pj_cis_buf_t cs_buff;
	pj_str_t mname, mfunc;
	pj_str_t cmd;
	pj_cis_t *cis = alloca(sizeof(pj_cis_t));
	pj_scanner *scanner = alloca(sizeof(pj_scanner));
	acore_ui_command_ele *ele = NULL;

	if (out == NULL)
		out = output_p;
	mname.slen = 0;
	mfunc.slen = 0;
	pj_cis_buf_init(&cs_buff);
	pj_cis_init(&cs_buff, cis);
	pj_cis_add_alpha(cis);
	pj_cis_add_num(cis);

	pj_scan_init(scanner, input->ptr, input->slen, PJ_SCAN_AUTOSKIP_WS,
			sc_callback);
	pj_scan_get(scanner, cis, &mname);
	if (mname.slen == 0) {
		PJ_LOG(1, ("ui","synctax error"));
	} else {
		ui_element *ui = acore_list_search(core_ui, find_ele, &mname);
		if (ui == NULL) {
			ui = acore_list_element_at(core_ui, 0);
			mfunc = mname;
		} else {
			// get funtion name
			pj_scan_get(scanner, cis, &mfunc);
		}
		if (mfunc.slen == 0) {
			PJ_LOG(3, ("ui","syntax error"));
		} else {
			pj_lock_acquire(ui->lock);
			ele = acore_list_search(ui->func_list, find_ele, &mfunc);
			pj_lock_release(ui->lock);
			if (ele && ele->handle) {
				cmd.ptr = scanner->curptr;
				cmd.slen = (int) (scanner->end - scanner->curptr);
				ele->handle(ele->user_data, &cmd, out, data);
			} else {
				PJ_LOG(3, ("ui","syntax error"));
			}
		}

	}
	pj_scan_fini(scanner);
}

#undef ui_base
