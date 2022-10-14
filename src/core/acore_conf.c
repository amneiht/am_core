#include <acore/conf.h>
#include <acore.h>
#include <stdio.h>

struct acore_conf_t {
	pj_pool_t *pool;
	pj_json_elem *json;
};

static void sc_callback(pj_scanner *scanner) {
	(void) scanner;
	if (scanner->curptr[0] == '#') {
		// skip a line
		pj_scan_skip_line(scanner);
	} else {
		pj_scan_get_char(scanner);
	}
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
	fclose(fp);

	acore_conf_t *conf = acore_conf_prase2(buff, sl);
	pj_bzero(buff, sl);
	return conf;
}
static int find_cmp(void *value, const pj_list_type *node) {
	const pj_str_t *name = value;
	const pj_json_elem *ele = node;
	return pj_strcmp(name, &ele->name);
}
static pj_json_elem* find_ele(const pj_str_t *name, pj_json_elem *ele) {

	pj_json_elem *st = (pj_json_elem*) &ele->value.children;
	return (pj_json_elem*) pj_list_search(st, (pj_str_t*) name, find_cmp);
}

static void add_ele(acore_conf_t *conf, pj_str_t *name, pj_str_t *value) {
	pj_str_t v;

	pj_strdup_with_null(conf->pool, &v, value);
	pj_json_elem *find = find_ele(name, conf->json);
	if (find == NULL) {
		find = PJ_POOL_ZALLOC_T(conf->pool, pj_json_elem);
		pj_str_t sl;
		pj_strdup_with_null(conf->pool, &sl, name);
		pj_json_elem_string(find, &sl, &v);
		pj_json_elem_add(conf->json, find);
	} else {
		if (find->type != PJ_JSON_VAL_ARRAY) {
			pj_json_elem *f2 = PJ_POOL_ZALLOC_T(conf->pool, pj_json_elem);
			pj_json_elem_string(f2, &find->name, &find->value.str);
			pj_json_elem_array(find, &find->name);
			pj_json_elem_add(find, f2);
		}
		pj_json_elem *ele = PJ_POOL_ZALLOC_T(conf->pool, pj_json_elem);
		pj_json_elem_string(ele, &find->name, &v);
		pj_json_elem_add(find, ele);
	}
}
void acore_conf_print(acore_conf_t *conf) {
	pj_json_elem *st = conf->json;
	pj_json_elem *p, *last;
	pj_json_elem *p2, *last2;
	last = (pj_json_elem*) &st->value.children;
	p = st->value.children.next;
	while (p != last) {
		if (p->type == PJ_JSON_VAL_ARRAY || p->type == PJ_JSON_VAL_OBJ) {
			last2 = (pj_json_elem*) &p->value.children;
			p2 = p->value.children.next;
			while (p2 != last2) {
				PJ_LOG(1,
						(ACORE_NAME,"%-30.*s %.*s",(int)p->name.slen,p->name.ptr,(int)p2->value.str.slen,p2->value.str.ptr));
				p2 = p2->next;
			}
		} else {
			PJ_LOG(1,
					(ACORE_NAME,"%-30.*s %.*s",(int)p->name.slen,p->name.ptr,(int)p->value.str.slen,p->value.str.ptr));
		}
		p = p->next;
	}
}
acore_conf_t* acore_conf_prase2(char *buffer, long len) {

	pj_pool_t *pool = acore_pool_create("conf pool", 4096, 4096);
	acore_conf_t *conf = pj_pool_alloc(pool, sizeof(acore_conf_t));

	conf->pool = pool;
	conf->json = PJ_POOL_ZALLOC_T(conf->pool, pj_json_elem);
	pj_json_elem_array(conf->json, NULL);
	//get size of file
	pj_cis_buf_t cs_buff, cs_cmt;
	pj_cis_t *cis = alloca(sizeof(pj_cis_t));
	pj_cis_t *cmt = alloca(sizeof(pj_cis_t));
	pj_cis_buf_init(&cs_buff);
	pj_cis_init(&cs_buff, cis);
	pj_cis_invert(cis);
	pj_cis_del_range(cis, 0, ' ');
	pj_cis_del_str(cis, "# ");

	pj_cis_buf_init(&cs_cmt);
	pj_cis_init(&cs_cmt, cmt);
	pj_cis_add_str(cmt, "#\n");
	pj_cis_invert(cmt);
	pj_scanner *scanner = alloca(sizeof(pj_scanner));
	pj_scan_init(scanner, buffer, len,
			PJ_SCAN_AUTOSKIP_WS | PJ_SCAN_AUTOSKIP_NEWLINE, sc_callback);

	pj_str_t *name = alloca(sizeof(pj_str_t));
	pj_str_t *value = alloca(sizeof(pj_str_t));

	while (!pj_scan_is_eof(scanner)) {
		name->slen = 0;
		value->slen = 0;
		pj_scan_get(scanner, cis, name);
		name = pj_strtrim(name);
		if (name->slen == 0)
			continue;
		pj_scan_get(scanner, cmt, value);
		value = pj_strtrim(value);
		add_ele(conf, name, value);
//		pj_scan_skip_line(scanner);
	}
	pj_scan_fini(scanner);
	return conf;
}
void acore_conf_release(acore_conf_t *conf) {
	conf->json = NULL;
	acore_pool_release(conf->pool);
	conf = NULL;
}
pj_str_t* acore_conf_get_str2(acore_conf_t *conf, const char *name) {
	pj_str_t sname = pj_str((char*) name);
	return acore_conf_get_str(conf, &sname);
}
pj_str_t* acore_conf_get_str(acore_conf_t *conf, const pj_str_t *name) {
	pj_json_elem *ele = find_ele(name, conf->json);
	if (ele == NULL)
		return NULL;
	if (ele->type == PJ_JSON_VAL_STRING)
		return &(ele->value.str);
	else
		return &(ele->value.children.next->value.str);
}
int acore_conf_get_int2(acore_conf_t *conf, const char *name) {
	pj_str_t sname = pj_str((char*) name);
	return acore_conf_get_int(conf, &sname);
}
int acore_conf_get_int(acore_conf_t *conf, const pj_str_t *name) {
	pj_str_t *sname = acore_conf_get_str(conf, name);
	if (sname == NULL)
		return -1;
	return atoi(sname->ptr);
}
double acore_conf_get_double2(acore_conf_t *conf, const char *name) {
	pj_str_t sname = pj_str((char*) name);
	return acore_conf_get_double(conf, &sname);
}
double acore_conf_get_double(acore_conf_t *conf, const pj_str_t *name) {
	pj_str_t *sname = acore_conf_get_str(conf, name);
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
void acore_conf_list2(acore_conf_t *conf, const char *name,
		void (*handle)(void *data, const pj_str_t *value), void *data) {
	pj_str_t sname = pj_str((char*) name);
	acore_conf_list(conf, &sname, handle, data);
}
void acore_conf_list(acore_conf_t *conf, const pj_str_t *name,
		void (*handle)(void *data, const pj_str_t *value), void *data) {
	pj_json_elem *ele = find_ele(name, conf->json);
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
