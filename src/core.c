/*
 * core.c
 *
 *  Created on: Aug 22, 2022
 *      Author: amneiht
 */

#include <acore.h>
#include <acore/context.h>
#include <acore_media/base.h>
#include <acore_media/sound.h>

int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 4096, 2048);
	acore_conf_t *conf;
// register some help funtion
	if (argc > 1)
		conf = acore_conf_parse(argv[1]);
	else
		conf = acore_conf_parse("/home/amneiht/app/project/acore/core.conf");
	acore_loop();
	acore_close();
	return 0;
}
