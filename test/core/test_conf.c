/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include <acore/context.h>

int main(int argc, char **argv) {
	acore_init();
	// register some help funtion
	acore_conf_t *conf = acore_conf_parse(
			"/home/amneiht/app/project/acore/core.conf");
	const acore_context_t *con = acore_conf_find_context2(conf, "audio");

//	const pj_str_t *str = acore_context_get_str2(con, "audio_port");
	acore_conf_print(conf);
	acore_conf_release(conf);
	acore_close();
	return 0;
}
