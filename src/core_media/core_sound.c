/*
 * core_sound.c
 *
 *  Created on: Oct 26, 2022
 *      Author: amneiht
 */

#include <acore_media/sound.h>
#include <acore_media/base.h>
#include <acore/base.h>
#include "media_local.h"

// reduce threa create
static pj_bool_t thread_multi = PJ_FALSE;
static void put_frame(pj_bool_t mute, pjmedia_port *stream, pjmedia_port *mport);
static void get_frame(pjmedia_port *stream, pjmedia_port *mport);

pjmedia_aud_dev_index acore_media_find_dirver_index(const pj_str_t *dev_name) {
	if (!dev_name || pj_strcmp2(dev_name, "default") == 0)
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
static void acore_clock_cb(const pj_timestamp *ts, void *user_data) {
	(void) ts;
	acore_media_sound_control *sct = user_data;
	switch (sct->mode) {
	case acore_port_mode_mod_full:
		// get capture
		put_frame(sct->mute, sct->stream_port, sct->port.one_p.media);
		// play back
		get_frame(sct->stream_port, sct->port.one_p.media);
		break;
	case acore_port_mode_mod_mix:
		// get capture from input and send to stream
		put_frame(sct->mute, sct->stream_port, sct->port.two_p.input);
		// play back frame from stream
		get_frame(sct->stream_port, sct->port.two_p.output);
		break;
	case acore_port_mode_mix_capture:
		// get capture from input and send to stream
		put_frame(sct->mute, sct->stream_port, sct->port.combine.lport);
		// play back auto call with aud stream
		break;
	case acore_port_mode_mix_playback:
		// play back
		get_frame(sct->stream_port, sct->port.combine.lport);
		break;
	default:
		break;
	}
}
static void setting_prm(pjmedia_port *stream_port, pjmedia_aud_param *prm,
		int dev_c, int dev_p) {
	pjmedia_aud_dev_default_param(dev_c, prm);
	prm->bits_per_sample = PJMEDIA_PIA_BITS(&stream_port->info);
	prm->channel_count = PJMEDIA_PIA_CCNT(&stream_port->info);
	prm->samples_per_frame = PJMEDIA_PIA_SPF(&stream_port->info);
	prm->clock_rate = PJMEDIA_PIA_SRATE(&stream_port->info);
	prm->rec_id = dev_c;
	prm->play_id = dev_p;
}

// sound callback
// put frame from media port to stream port
static void put_frame(pj_bool_t mute, pjmedia_port *stream, pjmedia_port *mport) {
	pjmedia_frame *fm = acore_alloca(pjmedia_frame);
	pj_bzero(fm, sizeof(pjmedia_frame));
	fm->size = PJMEDIA_PIA_SPF(&mport->info)
			* ((PJMEDIA_PIA_BITS(&mport->info) + 7) / 8);
	fm->buf = alloca(fm->size);
	pj_status_t st;
	st = pjmedia_port_get_frame(mport, fm);
	if (mute || st != PJ_SUCCESS)
		fm->type = PJMEDIA_TYPE_NONE;
	pjmedia_port_put_frame(stream, fm);
}
// get fream from stream port to media port
static void get_frame(pjmedia_port *stream, pjmedia_port *mport) {
	pjmedia_frame *fm = acore_alloca(pjmedia_frame);
	fm->size = PJMEDIA_PIA_SPF(&mport->info)
			* ((PJMEDIA_PIA_BITS(&mport->info) + 7) / 8);
	fm->buf = alloca(fm->size);
	pj_status_t st;
	st = pjmedia_port_get_frame(stream, fm);
	if (st != PJ_SUCCESS)
		fm->type = PJMEDIA_TYPE_NONE;
	pjmedia_port_put_frame(mport, fm);
}

static pj_status_t sound_play_cb(void *user_data, pjmedia_frame *frame) {
	acore_media_sound_control *sct = user_data;
	pj_status_t st;
	frame->type = PJMEDIA_TYPE_NONE;

	if (!thread_multi)
		acore_clock_cb(NULL, user_data);
	st = pjmedia_port_get_frame(sct->stream_port, frame);
	if (st) {
		char buff[300];
		pjmedia_strerror(st, buff, 300);
		puts(buff);
		frame->type = PJMEDIA_TYPE_NONE;
	}

	return PJ_SUCCESS;
}
static pj_status_t sound_rec_cb(void *user_data, pjmedia_frame *frame) {
	acore_media_sound_control *sct = user_data;
	if (sct->mute)
		frame->type = PJMEDIA_TYPE_NONE;
	pjmedia_port_put_frame(sct->stream_port, frame);
	if (!thread_multi)
		acore_clock_cb(NULL, user_data);
	return PJ_SUCCESS;
}

acore_media_sound_control* acore_media_sound_create_control(pj_pool_t *pool,
		pjmedia_port *stream_port, acore_conf_t *config,
		acore_media_fail_callback fail, void *fail_cb_data) {

	if (stream_port->info.fmt.type != PJMEDIA_TYPE_AUDIO) {
		PJ_LOG(4, ("media", "stream is not audio stream"));
		return NULL;
	}
	pj_str_t def = pj_str("system_sound");
	pj_status_t st;
	if (config == NULL)
		config = acore_media_default_conf();
	acore_context_t *aud_ctx = acore_conf_find_context2(config, "audio");
	if (aud_ctx == NULL) {
		PJ_LOG(4, ("media", "config is not include [audio] context"));
		return NULL;
	}
	const pj_str_t *source, *player;
	player = acore_context_get_str2(aud_ctx, "audio_player");
	source = acore_context_get_str2(aud_ctx, "audio_source");
	if (player == NULL) {
		player = &def;
	}
	if (source == NULL) {
		source = &def;
	}
	acore_media_sound_control *port_dtl = pj_pool_zalloc(pool,
			sizeof(acore_media_sound_control));
	port_dtl->stream_port = stream_port;
	pjmedia_port_info *pia = &stream_port->info;
	pjmedia_audio_format_detail *aud = &pia->fmt.det.aud;
	if (pj_strcmp(source, player) == 0) {
		if (pj_strcmp(source, &def) == 0) {
			port_dtl->mode = acore_port_mode_snd_full;
			acore_context_t *sys_ctx = acore_conf_find_context2(config,
					"system_audio");
			int dev_c = acore_media_find_dirver_index(
					acore_context_get_str2(sys_ctx, "capture"));
			int dev_p = acore_media_find_dirver_index(
					acore_context_get_str2(sys_ctx, "playback"));
			pjmedia_aud_param prm;
			setting_prm(stream_port, &prm, dev_c, dev_p);
			prm.dir = PJMEDIA_DIR_CAPTURE_PLAYBACK;
			st = pjmedia_aud_stream_create(&prm, sound_rec_cb, sound_play_cb,
					port_dtl, &port_dtl->port.snd.aud);
			if (st != PJ_SUCCESS) {
				PJ_LOG(4, ("media", "can not create pjmedia_aud_stream"));
				return NULL;
			}
		} else {
			port_dtl->mode = acore_port_mode_mod_full;
			port_dtl->port.one_p.media = acore_media_audio_create(pool, player,
					PJMEDIA_DIR_CAPTURE_PLAYBACK,
					&stream_port->info.fmt.det.aud, fail, fail_cb_data);
			if (port_dtl->port.one_p.media == NULL) {
				PJ_LOG(4,
						("media", "can not create port with factory name : %.*s",(int)player->slen , player->ptr));
				return NULL;
			}
			// create clock callback
			st = pjmedia_clock_create(pool, PJMEDIA_PIA_SRATE(pia),
					PJMEDIA_PIA_CCNT(pia), PJMEDIA_PIA_SPF(pia), 0,
					acore_clock_cb, port_dtl, &port_dtl->clock);
			if (st != PJ_SUCCESS) {
				PJ_LOG(4, ("media", "can not create pjmedia_clock"));
				acore_media_port_destroy(port_dtl->port.one_p.media);
				return NULL;
			}
		}
	} else {
		if (pj_strcmp(source, &def) != 0 && pj_strcmp(player, &def) != 0) {
			port_dtl->mode = acore_port_mode_mod_mix;
			// get audio input
			port_dtl->port.two_p.input = acore_media_audio_create(pool, source,
					ACORE_MEDIA_DIR_INPUT, aud, fail, fail_cb_data);
			// get audio output
			port_dtl->port.two_p.output = acore_media_audio_create(pool, player,
					ACORE_MEDIA_DIR_OUTPUT, aud, fail, fail_cb_data);
			if (port_dtl->port.two_p.input == NULL
					|| port_dtl->port.two_p.output == NULL) {
				if (port_dtl->port.two_p.input == NULL) {
					PJ_LOG(4,
							("media", "can not create audio input with factory name : %.*s",(int)player->slen , player->ptr));
				} else {
					acore_media_port_destroy(port_dtl->port.two_p.input);
				}
				if (port_dtl->port.two_p.output == NULL) {
					PJ_LOG(4,
							("media", "can not create audio output with factory name : %.*s",(int)source->slen ,source->ptr));
				} else {
					acore_media_port_destroy(port_dtl->port.two_p.output);
				}

				return NULL;
			}
			st = pjmedia_clock_create(pool, PJMEDIA_PIA_SRATE(pia),
					PJMEDIA_PIA_CCNT(pia), PJMEDIA_PIA_SPF(pia),
					PJMEDIA_CLOCK_NO_HIGHEST_PRIO, acore_clock_cb, port_dtl,
					&port_dtl->clock);
			if (st != PJ_SUCCESS) {
				PJ_LOG(4, ("media", "can not create pjmedia_clock"));
				acore_media_port_destroy(port_dtl->port.two_p.input);
				acore_media_port_destroy(port_dtl->port.two_p.output);
				return NULL;
			}
		} else {
			// port combine
			acore_context_t *sys_ctx = acore_conf_find_context2(config,
					"system_audio");
			int dev_c = acore_media_find_dirver_index(
					acore_context_get_str2(sys_ctx, "capture"));
			int dev_p = acore_media_find_dirver_index(
					acore_context_get_str2(sys_ctx, "playback"));
			pjmedia_aud_param prm;

			if (pj_strcmp(source, &def) == 0) {
				port_dtl->mode = acore_port_mode_mix_playback;
				// get output port
				port_dtl->port.combine.lport = acore_media_audio_create(pool,
						player, ACORE_MEDIA_DIR_OUTPUT, aud, fail,
						fail_cb_data);
				if (port_dtl->port.combine.lport == NULL) {
					PJ_LOG(4,
							("media_mix", "can not create playback port with factory name : %.*s",(int)source->slen ,source->ptr));
					return NULL;
				}
				setting_prm(stream_port, &prm, dev_p, dev_p);
				prm.dir = ACORE_MEDIA_DIR_INPUT;
				st = pjmedia_aud_stream_create(&prm, sound_rec_cb,
						sound_play_cb, port_dtl, &port_dtl->port.combine.strm);

			} else {
				port_dtl->mode = acore_port_mode_mix_capture;
				// intput data port
				port_dtl->port.combine.lport = acore_media_audio_create(pool,
						source, ACORE_MEDIA_DIR_INPUT, aud, fail, fail_cb_data);
				if (port_dtl->port.combine.lport == NULL) {
					PJ_LOG(4,
							("media_mix", "can not create capturer port with factory name : %.*s",(int)source->slen ,source->ptr));
					return NULL;
				}
				setting_prm(stream_port, &prm, dev_c, dev_c);
				prm.dir = ACORE_MEDIA_DIR_OUTPUT;
				st = pjmedia_aud_stream_create(&prm, sound_rec_cb,
						sound_play_cb, port_dtl, &port_dtl->port.combine.strm);
			}
			if (st != PJ_SUCCESS) {
				PJ_LOG(4, ("media_mix", "can not create pjmedia_aud_stream"));
				acore_media_port_destroy(port_dtl->port.combine.lport);
				return NULL;
			}
			if (thread_multi) {
				st = pjmedia_clock_create(pool, PJMEDIA_PIA_SRATE(pia),
						PJMEDIA_PIA_CCNT(pia), PJMEDIA_PIA_SPF(pia), 0,
						acore_clock_cb, port_dtl, &port_dtl->clock);
				if (st != PJ_SUCCESS) {
					PJ_LOG(4, ("media_mix", "can not create pjmedia_clock"));
					acore_media_port_destroy(port_dtl->port.combine.lport);
					pjmedia_aud_stream_destroy(port_dtl->port.combine.strm);
					return NULL;
				}
			}
		}
	}
	return port_dtl;
}

void acore_media_sound_start(acore_media_sound_control *snd) {
	switch (snd->mode) {
	case acore_port_mode_mod_full:
	case acore_port_mode_mod_mix:
		pjmedia_clock_start(snd->clock);
		break;
	case acore_port_mode_mix_capture:
	case acore_port_mode_mix_playback:
		if (thread_multi)
			pjmedia_clock_start(snd->clock);
		pjmedia_aud_stream_start(snd->port.combine.strm);
		break;
	case acore_port_mode_snd_full:
		pjmedia_aud_stream_start(snd->port.snd.aud);
		break;
	default:
		break;
	}
}
void acore_media_sound_stop(acore_media_sound_control *snd) {
	switch (snd->mode) {
	case acore_port_mode_mod_full:
	case acore_port_mode_mod_mix:
		pjmedia_clock_stop(snd->clock);
		break;
	case acore_port_mode_mix_capture:
	case acore_port_mode_mix_playback:
		if (thread_multi)
			pjmedia_clock_stop(snd->clock);
		pjmedia_aud_stream_stop(snd->port.combine.strm);
		break;
	case acore_port_mode_snd_full:
		pjmedia_aud_stream_stop(snd->port.snd.aud);
		break;
	default:
		break;
	}
}
void acore_media_sound_destroy(acore_media_sound_control *snd) {
	switch (snd->mode) {
	case acore_port_mode_mod_full:
		pjmedia_clock_stop(snd->clock);
		pjmedia_clock_destroy(snd->clock);
		acore_media_port_destroy(snd->port.one_p.media);
		break;
	case acore_port_mode_mod_mix:
		pjmedia_clock_stop(snd->clock);
		pjmedia_clock_destroy(snd->clock);
		acore_media_port_destroy(snd->port.two_p.input);
		acore_media_port_destroy(snd->port.two_p.output);
		break;
	case acore_port_mode_mix_capture:
	case acore_port_mode_mix_playback:
		if (thread_multi) {
			pjmedia_clock_stop(snd->clock);
			pjmedia_clock_destroy(snd->clock);
		}
		pjmedia_aud_stream_stop(snd->port.combine.strm);
		pjmedia_aud_stream_destroy(snd->port.combine.strm);
		acore_media_port_destroy(snd->port.combine.lport);
		break;
	case acore_port_mode_snd_full:
		pjmedia_aud_stream_stop(snd->port.snd.aud);
		pjmedia_aud_stream_destroy(snd->port.snd.aud);
		break;
	default:
		break;
	}
}

void acore_media_sound_mute(acore_media_sound_control *snd) {
	snd->mute = PJ_TRUE;
}
void acore_media_sound_unmute(acore_media_sound_control *snd) {
	snd->mute = PJ_FALSE;
}

/// get port connect with sound control aka stream port
pjmedia_port* acore_media_sound_get_port(acore_media_sound_control *snd) {
	return snd->stream_port;
}

