/*
 * base.h
 *
 *  Created on: Oct 21, 2022
 *      Author: amneiht
 */

#ifndef ACORE_MEDIA_BASE_H_
#define ACORE_MEDIA_BASE_H_
#include <acore/base.h>
#include <acore/context.h>
#include <pjmedia.h>
// suport for audio . video will support in far future

enum acore_media_dir {
	ACORE_MEDIA_DIR_INPUT = PJMEDIA_DIR_CAPTURE,
	ACORE_MEDIA_DIR_OUTPUT = PJMEDIA_DIR_PLAYBACK,
	ACORE_MEDIA_DIR_FULL = PJMEDIA_DIR_CAPTURE_PLAYBACK
};
/**
 * callback when audio has problem
 * @param user_data
 */
typedef int (*acore_media_fail_callback)(void *fail_callback_data);

/**
 *
 * @param pool
 * @param dir
 * @param aud
 * @param sys_conf
 * @param fail
 * @param fail_callback_data
 * @param factory_data
 * @return
 */
typedef pjmedia_port* (*acore_audio_create_callback)(pj_pool_t *pool,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud,
		acore_conf_t *sys_conf, acore_media_fail_callback fail,
		void *fail_callback_data, void *factory_data);

typedef void (*acore_audio_destroy_callback)(pjmedia_port *port,
		void *user_data);

typedef struct acore_audio_factory_t acore_audio_factory_t;

pj_status_t acore_media_init(acore_conf_t *conf);

void acore_media_close();

pjmedia_endpt* acore_media_endpt();

acore_audio_factory_t* acore_media_audio_register_factory(pj_pool_t *pool,
		const char *name, acore_audio_create_callback create,
		acore_audio_destroy_callback des, acore_callback_clear clear,
		void *user_data);
void
acore_media_audio_unregister_factory(acore_audio_factory_t *fac);

pjmedia_port* acore_media_audio_create(pj_pool_t *pool, const pj_str_t *name,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud,
		acore_media_fail_callback fail, void *callback_data);
pj_bool_t acore_media_port_destroy(pjmedia_port *port);

acore_conf_t* acore_media_default_conf();
#endif /* ACORE_MEDIA_BASE_H_ */
