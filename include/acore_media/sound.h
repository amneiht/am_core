/*
 * sound.h
 *
 *  Created on: Oct 26, 2022
 *      Author: amneiht
 */

#ifndef ACORE_MEDIA_SOUND_H_
#define ACORE_MEDIA_SOUND_H_
#include <pjmedia.h>
#include <acore/context.h>
#include <acore_media/base.h>
typedef struct acore_media_sound_control acore_media_sound_control;
pjmedia_aud_dev_index acore_media_find_dirver_index(const pj_str_t *name);

pjmedia_snd_port* acore_media_snd_create(pj_pool_t *pool,
		pjmedia_aud_dev_index playback, pjmedia_aud_dev_index capture,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud);

/**
 * create a sound port with config file
 * @param pool data alloc pool
 * @param stream_port uptream port to connect
 * @param config config struct , Null to user default config data
 * @param fail callback when sound port has problem
 * @param fail_cb_data user data
 * @return
 */
acore_media_sound_control* acore_media_sound_create_control(pj_pool_t *pool,
		pjmedia_port *stream_port, acore_conf_t *config,
		acore_media_fail_callback fail, void *fail_cb_data);

void acore_media_sound_start(acore_media_sound_control *snd);
void acore_media_sound_stop(acore_media_sound_control *snd);
void acore_media_sound_destroy(acore_media_sound_control *snd);

void acore_media_sound_mute(acore_media_sound_control *snd);
void acore_media_sound_unmute(acore_media_sound_control *snd);

/// get port connect with sound control aka stream port
pjmedia_port* acore_media_sound_get_port(acore_media_sound_control *snd);

// for test
struct acore_loop_media {
	pjmedia_stream *stream;
	pjmedia_port *loop_port;
	pjmedia_transport *tp;
};
struct acore_loop_media* acore_media_loop_back(pj_pool_t *pool, int playload);
#endif /* ACORE_MEDIA_SOUND_H_ */
