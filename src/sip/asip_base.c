/*
 * asip_base.c
 *
 *  Created on: Oct 4, 2022
 *      Author: amneiht
 */

#include <asip.h>
#include "local_sip.h"

int asip_get_transport_port(const pj_str_t *tran) {
	asip_sys *sip = _asip_get_sys();
	int port = 0;
	if (pj_strcmp2(tran, "tcp") == 0 && sip->transport.tcp != 0) {
		port = sip->transport.tcp->addr_name.port;
	} else if (pj_strcmp2(tran, "udp") == 0 && sip->transport.udp != 0) {
		port = sip->transport.udp->local_name.port;
	}
	return port;
}
void asip_start() {
	asip_sys *sip = _asip_get_sys();
	pj_lock_acquire(sip->lock);
}

void asip_end() {
	asip_sys *sip = _asip_get_sys();
	pj_lock_release(sip->lock);
}
