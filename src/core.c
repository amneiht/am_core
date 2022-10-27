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

#include "../module/wav/wav_mod.h"
extern void wav_module_init();
extern void alsa_module_init();
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 4096, 2048);
	// register some help funtion
	acore_conf_t *conf = acore_conf_parse(
			"/home/amneiht/app/project/acore/core.conf");
	acore_media_init(conf);

	wav_module_init();
//	alsa_module_init();

	const struct wav_function *wav = acore_runtime_get_clone(wav_api_name);

	pjmedia_port *src = wav->create_playback(pool,
			"/home/amneiht/Music/goc.wav");
	pjmedia_port_info *inf = &src->info;
	pj_str_t dev_name = pj_str("sysdefault:CARD=PCH");
	int dexid = acore_media_find_dirver_index(&dev_name);
	pjmedia_snd_port *port = acore_media_snd_create(pool, dexid, 0,
			PJMEDIA_DIR_PLAYBACK, &inf->fmt.det.aud);

	pj_status_t status = pjmedia_snd_port_connect(port, src);
	if (status) {
		char buff[300];
		pj_str_t lerr = pjmedia_strerror(status, buff, 300);
		PJ_LOG(1, ("alsa","Port create : %.*s", (int)lerr.slen, lerr.ptr));
	}
	acore_loop();
//	// release data
//	acore_runtime_release_clone(wav);
//	acore_conf_release(conf);
//	acore_pool_release(pool);
	acore_close();
	return 0;
}
