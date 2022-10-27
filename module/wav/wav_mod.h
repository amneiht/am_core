/*
 * wav_mod.h
 *
 *  Created on: Oct 20, 2022
 *      Author: amneiht
 */

#ifndef WAV_WAV_MOD_H_
#define WAV_WAV_MOD_H_

#include <acore_media/base.h>
#include <acore/runtime.h>

#define wav_api_name "wav"
typedef struct wav_function {
	ACORE_RUNTIME_DECL;
	pjmedia_port* (*create_playback)(pj_pool_t *pool, const char *name);
	pjmedia_port* (*create_record)(pj_pool_t *pool,
			const pjmedia_audio_format_detail *inf, const char *name,
			enum pjmedia_file_writer_option opt);
	void (*clear_port)(pjmedia_port*);
} wav_function;

#endif /* WAV_WAV_MOD_H_ */
