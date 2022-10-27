/*
 * core_sound.c
 *
 *  Created on: Oct 26, 2022
 *      Author: amneiht
 */

#include <acore_media/sound.h>
#include <acore/base.h>
pjmedia_aud_dev_index acore_media_find_dirver_index(pj_str_t *dev_name) {
	if (pj_strcmp2(dev_name, "default") == 0)
		return -1;
	pj_status_t status;
	int dev_count = pjmedia_aud_dev_count();
	pjmedia_aud_dev_info info;
	pj_int32_t devid = -1;
	if (dev_count == 0) {
		PJ_LOG(3, (ACORE_NAME, "No devices found"));
		return -1;
	}
	PJ_LOG(3, (ACORE_NAME, "Found %d devices:", dev_count));
	for (int i = 0; i < dev_count; ++i) {
		status = pjmedia_aud_dev_get_info(i, &info);
		if (status != PJ_SUCCESS)
			continue;
		if (pj_strcmp2(dev_name, info.name) == 0) {
			devid = i;
			break;
		}
	}
	return devid;
}
pjmedia_snd_port* acore_media_snd_create(pj_pool_t *pool,
		pjmedia_aud_dev_index playback, pjmedia_aud_dev_index capture,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud) {
	pj_status_t status;
	//	custom data
	pjmedia_snd_port_param *prm = acore_alloca(pjmedia_snd_port_param);
	pjmedia_snd_port_param_default(prm);
	prm->base.dir = dir;
	prm->base.rec_id = capture;
	prm->base.play_id = playback;
	prm->base.clock_rate = aud->clock_rate;
	prm->base.channel_count = aud->channel_count;
	prm->base.bits_per_sample = aud->bits_per_sample;
	prm->base.samples_per_frame = PJMEDIA_AFD_SPF(aud);
	prm->options = 0;
	prm->ec_options = 0;

	pjmedia_snd_port *port = NULL;
	status = pjmedia_snd_port_create2(pool, prm, &port);

	if (status) {
		char buff[300];
		pj_str_t lerr = pjmedia_strerror(status, buff, 300);
		PJ_LOG(1, ("alsa","Port create : %.*s", (int)lerr.slen, lerr.ptr));
		return NULL;
	}
	return port;
}
