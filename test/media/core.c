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

static acore_media_sound_control *ctl;
static void* init_para(int playload, pjmedia_port_info *pia) {
	const pjmedia_codec_info *pika;
	pj_status_t st;
	pj_str_t dsound = pj_str("am_sound");
	st = pjmedia_codec_mgr_get_codec_info(
			pjmedia_endpt_get_codec_mgr(acore_media_endpt()), playload, &pika);
	if (st)
		return NULL;
	pjmedia_codec_param *prm = acore_alloca(pjmedia_codec_param);
	st = pjmedia_codec_mgr_get_default_param(
			pjmedia_endpt_get_codec_mgr(acore_media_endpt()), pika, prm);
	if (st)
		return NULL;
	int spf = PJMEDIA_SPF(prm->info.clock_rate, prm->info.frm_ptime * 1000,
			prm->info.channel_cnt);
	pjmedia_port_info_init(pia, &dsound, PJMEDIA_SIG_CLASS_PORT_AUD('s', 'i'),
			prm->info.clock_rate, prm->info.channel_cnt,
			prm->info.pcm_bits_per_sample, spf);
	pia->fmt.det.aud.frame_time_usec = prm->info.frm_ptime * 1000;
	return pia;
}
static pj_status_t end_cb(void *data) {
//	acore_media_sound_stop(ctl);
//	acore_media_sound_destroy(ctl);
//	ctl = NULL;
	return 0;
}
static pj_status_t wav_play_cb(void *user_data, pjmedia_frame *frame) {
	return pjmedia_port_get_frame((pjmedia_port*) user_data, frame);
}
int main(int argc, char **argv) {
	acore_init();
	pj_pool_t *pool = acore_pool_create("Pika", 4096, 2048);
	acore_conf_t *conf;
// register some help funtion
	if (argc > 1)
		conf = acore_conf_parse(argv[1]);
	else
		conf = acore_conf_parse("/home/amneiht/app/project/acore/core.conf");
	acore_media_init(conf);
	wav_module_init();
#if 0
	struct acore_loop_media *loop = acore_media_loop_back(pool,
			PJMEDIA_RTP_PT_PCMU);
	ctl = acore_media_sound_create_control(pool, loop->loop_port, conf, end_cb,
	NULL);

	pjmedia_stream_start(loop->stream);
	acore_media_sound_start(ctl);
#else
	pjmedia_aud_param param;
	pjmedia_aud_stream *strm = NULL;
	pj_status_t status;
	pj_str_t mod = pj_str("wav");
	pj_str_t dev_name = pj_str("sysdefault:CARD=PCH");
	pjmedia_port_info *pia = acore_alloca(pjmedia_port_info);
	init_para(PJMEDIA_RTP_PT_PCMA, pia);
	pjmedia_port *port_test = acore_media_audio_create(pool, &mod,
			ACORE_MEDIA_DIR_INPUT, &pia->fmt.det.aud, end_cb, NULL);

	int play_index = acore_media_find_dirver_index(&dev_name);
	status = pjmedia_aud_dev_default_param(play_index, &param);
	param.dir = ACORE_MEDIA_DIR_OUTPUT;
	param.clock_rate = PJMEDIA_PIA_SRATE(pia);
	param.samples_per_frame = PJMEDIA_PIA_SPF(pia);
	param.channel_count = PJMEDIA_PIA_CCNT(pia);
	param.bits_per_sample = PJMEDIA_PIA_BITS(pia);

	pjmedia_aud_stream_create(&param, NULL, &wav_play_cb, port_test, &strm);

	pjmedia_aud_stream_start(strm);
#endif
	acore_loop();
	acore_close();
	return 0;
}
