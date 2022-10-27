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
pjmedia_aud_dev_index acore_media_find_dirver_index(pj_str_t *name);

pjmedia_snd_port* acore_media_snd_create(pj_pool_t *pool,
		pjmedia_aud_dev_index playback, pjmedia_aud_dev_index capture,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud);

pjmedia_port* acore_media_sound_default(pj_pool_t *pool,
		const pjmedia_audio_format_detail *aud, acore_conf_t *config);

#endif /* ACORE_MEDIA_SOUND_H_ */
