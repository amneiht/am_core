/*
 * asip_scf.c
 *
 *  Created on: Oct 3, 2022
 *      Author: amneiht
 */

#include <asip/cfg.h>
#include "local_sip.h"

static const pj_str_t sip_tp = { "tcp", 3 };

const pj_str_t* asip_cfg_get_transport() {
	asip_sys *sys = _asip_get_sys();
	if (sys->conf == NULL)
		return &sip_tp;
//	pj_str_t *transport = acore_conf_get_str2(sys->conf, "transport");
	pj_str_t *transport = NULL;
	if (transport == NULL)
		return &sip_tp;
	return transport;
}
