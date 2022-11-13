/*
 * acore_context.c
 *
 *  Created on: Oct 16, 2022
 *      Author: amneiht
 */

#include <acore/context.h>
#include "local_core.h"
#include <pjlib-util.h>

struct acore_context_t {
	PJ_DECL_LIST_MEMBER(acore_context_t)
	;
	pj_rbtree_node *parent;
	pj_bool_t is_pattern;
	acore_context_t *pattern;
	acore_conf_t *conf; // refer to config
	struct {
		PJ_DECL_LIST_MEMBER(pj_json_elem)
		;
	} child_list;
};
struct acore_conf_t {
	pj_pool_t *pool;
	pj_rbtree *tree;
	pj_str_t path;
	struct {
		PJ_DECL_LIST_MEMBER(acore_context_t)
		;
	} list;

};
static pj_cis_t *cis_value;
static pj_cis_t *cis_cmt;
static pj_cis_t *cis_context;
static char *conf_path;

static void conf_clear(void *data);
const char* acore_conf_get_path() {
	return conf_path;
}
void acore_conf_set_path(const char *path) {
	sprintf(conf_path, "%s", path);
}

ACORE_LOCAL void _acore_context_init(pj_pool_t *pool) {
	conf_path = pj_pool_alloc(pool, 500);
#if defined ( __linux__)
	sprintf(conf_path, "%s", "/tmp/core");
#elif defined (__WIN32__)
	sprintf(conf_path, "%s", "C:\\ProgramData\\core");
#endif

	char *command_str = ";#\n";
	{
		pj_cis_buf_t *cs_cmt = pj_pool_alloc(pool, sizeof(pj_cis_buf_t));
		cis_cmt = pj_pool_alloc(pool, sizeof(pj_cis_t));
		pj_cis_buf_init(cs_cmt);
		pj_cis_init(cs_cmt, cis_cmt);
		pj_cis_invert(cis_cmt);
		pj_cis_del_str(cis_cmt, command_str);
	}
	// value str
	{
		pj_cis_buf_t *cs_cmt = pj_pool_alloc(pool, sizeof(pj_cis_buf_t));
		cis_value = pj_pool_alloc(pool, sizeof(pj_cis_t));
		pj_cis_buf_init(cs_cmt);
		pj_cis_init(cs_cmt, cis_value);
		pj_cis_invert(cis_value);
		pj_cis_del_range(cis_value, 0, ' ');
		pj_cis_del_str(cis_value, command_str);
		// context parten
	}
//cis_context
	{
		pj_cis_buf_t *cs_context = pj_pool_alloc(pool, sizeof(pj_cis_buf_t));
		cis_context = pj_pool_alloc(pool, sizeof(pj_cis_t));
		pj_cis_buf_init(cs_context);
		pj_cis_init(cs_context, cis_context);
		pj_cis_invert(cis_context);
		pj_cis_del_range(cis_context, 0, ' ' + 1);
		pj_cis_del_str(cis_context, command_str);
		pj_cis_del_str(cis_context, "[]()");
		pj_cis_del_str(cis_context, "=");
	}
}
static int rbtree_key_cmp(const void *key1, const void *key2) {
	const pj_str_t *str1 = key1;
	const pj_str_t *str2 = key2;
	return pj_strcmp(str1, str2);
}

static void sc_callback(pj_scanner *scanner) {
	(void) scanner;
	if (!pj_cis_match(cis_cmt, scanner->curptr[0])) {
		// skip a line
		pj_scan_skip_line(scanner);
	}
}
static acore_context_t* create_context(acore_conf_t *conf,
		const pj_str_t *ctx_name) {
	pj_rbtree_node *node = pj_pool_alloc(conf->pool, sizeof(pj_rbtree_node));
	pj_str_t *name = pj_pool_alloc(conf->pool, sizeof(pj_str_t));
	pj_strdup(conf->pool, name, ctx_name);
	node->key = name;
	acore_context_t *ele = PJ_POOL_ALLOC_T(conf->pool, acore_context_t);
	pj_list_init(&ele->child_list);
	ele->parent = node;
	ele->is_pattern = PJ_FALSE;
	ele->pattern = NULL;
//insert node
	node->user_data = ele;
	pj_rbtree_insert(conf->tree, node);
//insert to list ;
	pj_list_insert_after(&conf->list, ele);
	ele->conf = conf;
	return ele;
}
static acore_context_t* add_context(acore_conf_t *conf, pj_scanner *sc) {
	pj_str_t *out = acore_alloca(pj_str_t);
	acore_context_t *ele;
	out->slen = 0;
	int line = sc->line;
	pj_scan_get_quotes(sc, "[", "]", 1, out);
	if (out->slen == 0)
		return NULL;
	out->ptr = out->ptr + 1;
	out->slen = out->slen - 2;
	out = pj_strtrim(out);
// check if node exits
	pj_rbtree_node *node = pj_rbtree_find(conf->tree, out);
	if (node == NULL) {
		// create new context
		ele = create_context(conf, out);
		if (sc->curptr[0] == '(') {
			pj_scan_get_quotes(sc, "(", ")", 1, out);
			out->ptr = out->ptr + 1;
			out->slen = out->slen - 2;
			out = pj_strtrim(out);
			if (out->slen > 0) {
				if (pj_strcmp2(out, "!") == 0) {
					ele->is_pattern = PJ_TRUE;
				} else {
					node = pj_rbtree_find(conf->tree, out);
					if (node == NULL) {
						PJ_LOG(1,
								("Config","Pattern %.*s must declare first",(int)out->slen , out->ptr));
						goto AOUT;
					}
					acore_context_t *ele2 = node->user_data;
					if (!ele2->is_pattern) {
						PJ_LOG(1,
								("Config","Pattern %.*s if not declare with (!)",(int)out->slen , out->ptr));
					} else {
						ele->pattern = ele2;
					}
				}

			}
		}

	} else {
		ele = node->user_data;
	}

	AOUT: if (sc->line == line)
		pj_scan_skip_line(sc);
	return ele;
}
static int find_cmp(void *value, const pj_list_type *node) {
	const pj_str_t *name = value;
	const pj_json_elem *ele = node;
	return pj_strcmp(name, &ele->name);
}
static pj_json_elem* find_ele(const pj_str_t *name, void *list) {

	return (pj_json_elem*) pj_list_search(list, (pj_str_t*) name, find_cmp);
}
static void add_ele(pj_pool_t *pool, acore_context_t *ctx, pj_str_t *name,
		pj_str_t *value) {
	pj_str_t v;
	pj_json_elem *find = find_ele(name, &ctx->child_list);
	pj_strdup_with_null(pool, &v, value);
	if (find == NULL) {
		find = PJ_POOL_ZALLOC_T(pool, pj_json_elem);
		pj_str_t sl;
		pj_strdup_with_null(pool, &sl, name);
		pj_json_elem_string(find, &sl, &v);
		pj_list_insert_before(ctx->child_list.next, find);
	} else {
		if (find->type != PJ_JSON_VAL_ARRAY) {
			pj_json_elem *f2 = PJ_POOL_ZALLOC_T(pool, pj_json_elem);
			pj_json_elem_string(f2, &find->name, &find->value.str);
			pj_json_elem_array(find, &find->name);
			pj_json_elem_add(find, f2);
		}
		pj_json_elem *ele = PJ_POOL_ZALLOC_T(pool, pj_json_elem);
		pj_json_elem_string(ele, &find->name, &v);
		pj_json_elem_add(find, ele);
	}
}
static pj_status_t parrent_dir(const char *file, pj_str_t *path) {
	if (file == NULL)
		return -1;
	char sp = ACORE_PATH_SP;
	int i;
	int size = strlen(file);
	for (i = size - 1; i > -1; i--) {
		if (file[i] == sp)
			break;
	}
	if (i < 0) {
		path->slen = sprintf(path->ptr, ".");
		return 0;
	}
	i--;
	path->slen = sprintf(path->ptr, "%.*s", i, file);
	return 0;
}
acore_conf_t* acore_conf_parse(const char *file) {
	FILE *fp = fopen(file, "r");
	if (!fp) {
		PJ_LOG(3, (ACORE_NAME,"no config file"));
	}
	fseek(fp, 0, SEEK_END);
	long sl = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char buff[sl + 1];
	buff[sl] = 0;
	fread(buff, sizeof(char), sl, fp);

	acore_conf_t *conf = acore_conf_prase2(buff, sl);
	conf->path.ptr = pj_pool_alloc(conf->pool, 500);
	parrent_dir(file, &conf->path);
	fclose(fp);
	return conf;
}
acore_conf_t* acore_conf_prase2(char *buffer, long len) {

	pj_pool_t *pool = acore_pool_create("conf pool", 4096, 4096);
	acore_conf_t *conf = pj_pool_alloc(pool, sizeof(acore_conf_t));

	conf->pool = pool;
	conf->tree = PJ_POOL_ZALLOC_T(conf->pool, pj_rbtree);
	pj_rbtree_init(conf->tree, rbtree_key_cmp);
	pj_list_init(&conf->list);

	pj_scan_state *state = acore_alloca(pj_scan_state);
	pj_scanner *scanner = alloca(sizeof(pj_scanner));
	pj_scan_init(scanner, buffer, len,
			PJ_SCAN_AUTOSKIP_WS | PJ_SCAN_AUTOSKIP_WS_HEADER
					| PJ_SCAN_AUTOSKIP_NEWLINE, sc_callback);

	pj_str_t *name = alloca(sizeof(pj_str_t));
	pj_str_t *value = alloca(sizeof(pj_str_t));

	pj_str_t ctx_name = pj_str("core");
	acore_context_t *current = create_context(conf, &ctx_name);
	while (!pj_scan_is_eof(scanner)) {
		name->slen = 0;
		value->slen = 0;
		pj_scan_get(scanner, cis_context, name);
		if (name->slen == 0) {
			pj_scan_skip_whitespace(scanner);
			if (scanner->curptr[0] != '[') {
				continue;
			}
			current = add_context(conf, scanner);
			continue;
		}
		name = pj_strtrim(name);
		pj_scan_save_state(scanner, state);
		pj_scan_get(scanner, cis_value, value);
		if (value->slen == 0)
			continue;
		// remove "="
		if (value->ptr[0] == '=') {
			// reset str
			pj_scan_restore_state(scanner, state);
			scanner->curptr = value->ptr + 1;
			pj_scan_skip_whitespace(scanner);
			value->slen = 0;
			pj_scan_get(scanner, cis_value, value);
			if (value->slen == 0)
				continue;
		}
		value = pj_strtrim(value);
		add_ele(conf->pool, current, name, value);
	}
	pj_scan_fini(scanner);
	acore_mem_bind(pool, conf, conf_clear);
	return conf;
}
static void conf_clear(void *data) {
	acore_conf_t *conf = data;
	acore_pool_release(conf->pool);
}
void acore_conf_release(acore_conf_t *conf) {
	acore_mem_mask_destroy(conf);
}
void node_print(void *node, void *user_data) {
	(void) user_data;
	pj_rbtree_node *rbnode = node;
	pj_json_elem *child;
	pj_str_t *val;
	acore_context_t *con = rbnode->user_data;
	const pj_str_t *name = rbnode->key;
	if (con->is_pattern) {
		PJ_LOG(1, ("","[%.*s](!)",(int)name->slen , name->ptr));

	} else if (con->pattern != NULL) {
		const pj_str_t *pat = con->pattern->parent->key;
		PJ_LOG(1,
				("","[%.*s](%.*s)",(int)name->slen , name->ptr,(int)pat->slen , pat->ptr));
	} else {
		PJ_LOG(1, ("","[%.*s]",(int)name->slen , name->ptr));
	}
// log child
	pj_json_elem *ele = con->child_list.next;
	while (ele != (pj_json_elem*) &con->child_list) {
		name = &ele->name;
		if (ele->type == PJ_JSON_VAL_ARRAY) {
			child = ele->value.children.next;
			while (child != (pj_json_elem*) &ele->value.children) {
				val = &child->value.str;
				PJ_LOG(1,
						("","%-32.*s=\t%.*s",(int)name->slen , name->ptr,(int)val->slen , val->ptr));
				child = child->next;
			}
		} else {
			val = &ele->value.str;
			PJ_LOG(1,
					("","%-32.*s=\t%.*s",(int)name->slen , name->ptr,(int)val->slen , val->ptr));
		}
		ele = ele->next;
	}
	PJ_LOG(1, ("","-----------"));
}
void acore_conf_print(const acore_conf_t *conf) {
	_acore_tree_view(conf->tree, node_print, NULL);
}
acore_context_t* acore_conf_find_context(acore_conf_t *conf,
		const pj_str_t *name) {

	pj_rbtree_node *node = pj_rbtree_find(conf->tree, (void*) name);
	if (node == NULL)
		return NULL;
	return (acore_context_t*) node->user_data;
}
acore_context_t* acore_conf_find_context2(acore_conf_t *conf, const char *name) {
	convert_str(ctx_name, name);
	return acore_conf_find_context(conf, ctx_name);
}
acore_context_t* acore_context_next(const acore_context_t *context) {
	const acore_context_t *last = context->conf->list.prev;
	if (context->next == last)
		return NULL;
	return context->next;
}
const pj_str_t* acore_context_name(const acore_context_t *context) {
	return (const pj_str_t*) context->parent->key;
}
pj_bool_t acore_context_is_pattern(const acore_context_t *context) {
	return context->is_pattern;
}

const pj_str_t* acore_context_get_str2(const acore_context_t *context,
		const char *name) {
	convert_str(sname, name);
	return acore_context_get_str(context, sname);
}
const pj_str_t* acore_context_get_str(const acore_context_t *context,
		const pj_str_t *name) {
	pj_json_elem *ele = find_ele(name, (void*) &context->child_list);
	if (ele == NULL) {
		if (context->pattern) {
			return acore_context_get_str(context->pattern, name);
		} else
			return NULL;
	}
	if (ele->type == PJ_JSON_VAL_STRING)
		return &(ele->value.str);
	else
		return &(ele->value.children.next->value.str);
}

int acore_context_get_int2(const acore_context_t *context, const char *name) {
	convert_str(sname, name);
	return acore_context_get_int(context, sname);
}
int acore_context_get_int(const acore_context_t *context, const pj_str_t *name) {
	const pj_str_t *sname = acore_context_get_str(context, name);
	if (sname == NULL)
		return -1;
	return atoi(sname->ptr);
}

double acore_context_get_double2(const acore_context_t *context,
		const char *name) {
	convert_str(sname, name);
	return acore_context_get_double(context, sname);
}
double acore_context_get_double(const acore_context_t *context,
		const pj_str_t *name) {
	const pj_str_t *sname = acore_context_get_str(context, name);
	if (sname == NULL)
		return -1;
	return strtod(sname->ptr, NULL);
}
struct dlist {
	void (*handle)(void *data, const pj_str_t *value);
	void *data;
};
static int list_cmp(void *value, const pj_list_type *node) {
	struct dlist *dl = (struct dlist*) value;
	const pj_json_elem *ele = node;
	dl->handle(dl->data, &ele->value.str);
	return 1;
}

void acore_context_list_value(const acore_context_t *context,
		const pj_str_t *name, void (*handle)(void *data, const pj_str_t *value),
		void *data) {
	pj_json_elem *ele = find_ele(name, (void*) &context->child_list);
	if (ele == NULL)
		return;
	if (ele->type == PJ_JSON_VAL_STRING) {
		handle(data, &ele->value.str);
	} else {
		struct dlist *dl = alloca(sizeof(struct dlist));
		dl->data = data;
		dl->handle = handle;
		pj_list_search((void*) &ele->value.children, dl, list_cmp);
	}
}
void acore_context_list_value2(const acore_context_t *context, const char *name,
		void (*handle)(void *data, const pj_str_t *value), void *data) {
	convert_str(sname, name);
	acore_context_list_value(context, sname, handle, data);
}
void acore_context_print(const acore_context_t *context) {
	node_print(context->parent, NULL);
}

