/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include <acore/acore_conf.h>
static void print_data(void *data, const pj_str_t *dtx) {
	(void) data;
	puts(dtx->ptr);
}
int main(int argc, char **argv) {
	acore_init();
	// register some help funtion
	acore_conf_t *conf = acore_conf_parse(
			"/tftp/mip/mip10dv_installer_bcy/opt/vgisc/etc/megasip/config");
//	acore_conf_print(conf);
	double rate = acore_conf_get_double2(conf, "ausrc_srate");
	PJ_LOG(1, (ACORE_NAME , "rate is %f",rate));

	acore_conf_list2(conf, "module", print_data, NULL);
	acore_conf_release(conf);
	acore_close();
	return 0;
}

