/*
 * asip_ua.c
 *
 *  Created on: Oct 2, 2022
 *      Author: amneiht
 */

#include <asip/ua.h>
#include <asip/cfg.h>
#include "local_sip.h"
struct ulist {
	PJ_DECL_LIST_MEMBER(asip_ua)
	;
};
static struct ulist ua_list;
static pj_lock_t *ua_lock;
ACORE_LOCAL void _asip_init_ua(pj_pool_t *pool) {
	pj_lock_create_recursive_mutex(pool, "ua", &ua_lock);
	pj_list_init(&ua_list);
}
ACORE_LOCAL void _asip_close_ua() {
	pj_lock_acquire(ua_lock);
	asip_ua *ua = ua_list.next;
	asip_ua *last = ua_list.prev;
	asip_ua *p;
	while (ua != last) {
		p = ua->next;
		asip_ua_destroy(ua);
		ua = p;
	}
	pj_lock_release(ua_lock);
	pj_lock_destroy(ua_lock);
}
//create ua
asip_ua* asip_ua_create2(pj_pool_t *pool, const char *user, const char *pass,
		const char *host, const int port) {

	const pj_str_t u_user = pj_str((char*) user);
	const pj_str_t u_pass = pj_str((char*) pass);
	const pj_str_t u_host = pj_str((char*) host);
	return asip_ua_create(pool, &u_user, &u_pass, &u_host, port);
}
asip_ua* asip_ua_create(pj_pool_t *pool, const pj_str_t *user,
		const pj_str_t *pass, const pj_str_t *host, const int port) {
	pj_bool_t use_pool = PJ_FALSE;
	if (pool == NULL) {
		pool = acore_pool_create(NULL, 2048, 2048);
		use_pool = PJ_TRUE;
	}
	asip_ua *uas = (asip_ua*) pj_pool_zalloc(pool, sizeof(asip_ua));
	if (use_pool)
		uas->pool = pool;
	// create cred
	pj_strdup_with_null(pool, &uas->login.cred.realm, host);
	pj_strdup2_with_null(pool, &uas->login.cred.scheme, "digest");
	pj_strdup_with_null(pool, &uas->login.cred.username, user);
	pj_strdup_with_null(pool, &uas->login.cred.data, pass);
	uas->login.cred.data_type = PJSIP_CRED_DATA_PLAIN_PASSWD;
	// contact buffer
	uas->contact.uid.ptr = pj_pool_alloc(pool, PJ_GUID_STRING_LENGTH + 1);
	uas->contact.uid.ptr[PJ_GUID_STRING_LENGTH] = 0;
	pj_generate_unique_string(&uas->contact.uid);
	uas->contact.last_contact.ptr = pj_pool_alloc(pool, 500);
	// set param for login
	uas->login.state = asip_state_null;
	uas->login.login_time = 60;
	uas->port = port;
	// for invite session
	pj_list_init(&uas->call);
	pj_list_insert_before(&ua_list, uas);
	return uas;
}
pj_bool_t asip_ua_destroy(asip_ua *ua) {
	pj_list_erase(ua);
	pj_bzero(ua, sizeof(asip_ua));
	if (ua->pool)
		acore_pool_release(ua->pool);
	return PJ_TRUE;
}

//find user_agent
static pj_bool_t find_by_uri(void *value, const pj_list_type *node) {
	const asip_ua *uas = node;
	pjsip_sip_uri *suri = value;
	return pj_strcmp(&suri->user, &uas->login.cred.username)
			|| pj_strcmp(&suri->host, &uas->login.cred.realm);
}
asip_ua* asip_ua_find_by_uri(pjsip_uri *uri) {
	if (PJSIP_URI_SCHEME_IS_SIP(uri) || PJSIP_URI_SCHEME_IS_SIPS(uri)) {
		pjsip_sip_uri *suri = (pjsip_sip_uri*) pjsip_uri_get_uri(uri);
		return (asip_ua*) pj_list_search(&ua_list, suri, find_by_uri);
	} else {
		return NULL;
	}
}
asip_ua* asip_ua_find_by_msg(pjsip_msg *msg) {
	pjsip_to_hdr *hdr = PJSIP_MSG_TO_HDR(msg);
	if (hdr == NULL)
		return NULL;
	pjsip_uri *uri = hdr->uri;
	return asip_ua_find_by_uri(uri);
}
static pj_bool_t find_by_name(void *value, const pj_list_type *node) {
	const asip_ua *uas = node;
	pj_str_t *str = value;
	return pj_strcmp(str, &uas->login.cred.username);
}
asip_ua* asip_ua_find_by_name(pj_str_t *name) {
	return (asip_ua*) pj_list_search(&ua_list, name, find_by_name);
}

// uri help
pj_bool_t asip_ua_print_uri(asip_ua *ua, pj_str_t *uri) {

	if (ua->pool == 0)
		uri->slen = sprintf(uri->ptr, "<sip:%.*s@%.*s>",
				(int) ua->login.cred.username.slen, ua->login.cred.username.ptr,
				(int) ua->login.cred.realm.slen, ua->login.cred.realm.ptr);
	else
		uri->slen = sprintf(uri->ptr, "<sip:%.*s@%.*s:%d>",
				(int) ua->login.cred.username.slen, ua->login.cred.username.ptr,
				(int) ua->login.cred.realm.slen, ua->login.cred.realm.ptr,
				ua->port);

	return PJ_TRUE;
}
static pj_bool_t checkuri(const pj_str_t *name) {
	int str = name->slen;
	for (int i = 0; i < str; i++) {
		if (name->ptr[i] == '@')
			return PJ_TRUE;
	}
	return PJ_FALSE;
}
pj_bool_t asip_ua_print_call_uri(asip_ua *ua, const pj_str_t *callee,
		pj_str_t *out) {
	out->slen = 0;
	if (checkuri(callee)) {
		out->slen = sprintf(out->ptr, "<sip:%.*s>", (int) callee->slen,
				callee->ptr);
	} else {
		if (ua->port == 0)
			out->slen = sprintf(out->ptr, "<sip:%.*s@%.*s>", (int) callee->slen,
					callee->ptr, (int) ua->login.cred.realm.slen,
					ua->login.cred.realm.ptr);
		else
			out->slen = sprintf(out->ptr, "<sip:%.*s@%.*s:%d>",
					(int) callee->slen, callee->ptr,
					(int) ua->login.cred.realm.slen, ua->login.cred.realm.ptr,
					ua->port);
	}

	return PJ_TRUE;
}
pj_bool_t asip_ua_print_contact(asip_ua *ua, pj_str_t *conf) {
	pj_sockaddr addr;
	pj_gethostip(pj_AF_INET(), &addr);
	char *hostip = pj_inet_ntoa(addr.ipv4.sin_addr);
	const pj_str_t *tran = asip_cfg_get_transport();
	int port = asip_get_transport_port(tran);
	conf->slen = sprintf(conf->ptr,
			"<sip:%.*s@%s:%d;transport=%.*s>;+sip.instance=\"<urn:uuid:%s>\"",
			(int) ua->login.cred.username.slen, ua->login.cred.username.ptr,
			hostip, port, (int) tran->slen, tran->ptr, ua->contact.uid.ptr);
	return PJ_TRUE;
}

// print availe ua
void asip_ua_print() {
	asip_ua *p = ua_list.next;
	asip_ua *last = ua_list.prev;
	pj_str_t uri;
	uri.ptr = alloca(300);
	while (p != last) {
		asip_ua_print_uri(p, &uri);
		PJ_LOG(1, (ACORE_NAME,"ua :\"%s\"",uri.ptr));
		p = p->next;
	}
}
void asip_ua_print2(acore_ui_output_p out, void *ui_data) {
	asip_ua *p = ua_list.next;
	asip_ua *last = ua_list.prev;
	char buff2[300];
	pj_str_t pua;
	pua.ptr = buff2;
	pj_str_t uri;
	uri.ptr = alloca(300);
	while (p != last) {
		asip_ua_print_uri(p, &uri);
		pua.slen = sprintf(buff2, "ua :\"%s\"", uri.ptr);
		out(ui_data, &pua);
		p = p->next;
	}
}

