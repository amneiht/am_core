/*
 * ua.h
 *
 *  Created on: Oct 2, 2022
 *      Author: amneiht
 */

#ifndef ASIP_UA_H_
#define ASIP_UA_H_

#include <pjsip.h>
#include <pjsip_ua.h>
#include <acore/base.h>
#include <acore/ui.h>

typedef enum asip_ua_state {
	asip_state_null, //
	asip_state_login, //
	asip_state_disconnect, //
	asip_state_wait, // trang thai cho khi login
	asip_state_error
} asip_ua_state;

typedef struct asip_ua asip_ua;

//create ua
asip_ua* asip_ua_create(pj_pool_t *pool, const pj_str_t *user,
		const pj_str_t *pass, const pj_str_t *host, const int port);
asip_ua* asip_ua_create2(pj_pool_t *pool, const char *user, const char *pass,
		const char *host, const int port);

pj_bool_t asip_ua_destroy(asip_ua *ua);
// config for ua;

void asip_ua_set_logintime(asip_ua *ua, int time);
//find user_agent
asip_ua* asip_ua_find_by_uri(pjsip_uri *uri);
asip_ua* asip_ua_find_by_name(pj_str_t *name);
asip_ua* asip_ua_find_by_msg(pjsip_msg *msg);

// uri formart <sip:"$user"@"$serverhost":"@port">
pj_bool_t asip_ua_print_uri(asip_ua *ua, pj_str_t *uri);

/**
 * print callee uri for invite
 * @param ua
 * @param callee who recieve a call : example "boo" or "boo@pika.com:2022"
 * @param buff
 * @param blen
 * @return
 */
pj_bool_t asip_ua_print_call_uri(asip_ua *ua, const pj_str_t *callee,
		pj_str_t *out);
pj_bool_t asip_ua_print_contact(asip_ua *ua, pj_str_t *contact);

// print avaiable ua
void asip_ua_print();
void asip_ua_print2(acore_ui_output_p out, void *ui_data);

// register
pj_bool_t asip_ua_register(asip_ua *ua);
pj_bool_t asip_ua_unregister(asip_ua *ua);

#endif /* ASIP_UA_H_ */
