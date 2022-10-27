/*
 * wav_mod.c
 *
 *  Created on: Oct 20, 2022
 *      Author: amneiht
 */

#include "wav_mod.h"
#include <acore/runtime.h>
#include <acore/mem.h>
typedef struct wav_mod {
	pj_pool_t *pool;
	wav_function *fun;
	void *api;
} wav_mod;

static wav_mod *wav;

static pjmedia_port* wav_create_playback(pj_pool_t *pool, const char *name) {
	if (!acore_mem_is_available(wav))
		return NULL;
	pjmedia_port *player = NULL;
	pj_status_t status = pjmedia_wav_player_port_create(pool, name, 20, 0, 0,
			&player);

	if (status != PJ_SUCCESS)
		acore_mem_add_ref(wav);
	return player;
}
static pjmedia_port* wav_create_record(pj_pool_t *pool,
		const pjmedia_audio_format_detail *inf, const char *filename,
		enum pjmedia_file_writer_option opt) {
	if (!acore_mem_is_available(wav))
		return NULL;
	pjmedia_port *record = NULL;
	pj_status_t status = pjmedia_wav_writer_port_create(pool, filename,
			inf->clock_rate, inf->channel_count, PJMEDIA_AFD_SPF(inf),
			inf->bits_per_sample, opt, 0, &record);
	if (status != PJ_SUCCESS)
		acore_mem_add_ref(wav);
	return record;
}
static void wav_clear_port(pjmedia_port *p) {
	pjmedia_port_destroy(p);
	acore_mem_dec_ref(wav);
}

static void wav_destroy(void *data) {
	wav_mod *wav = data;
	acore_pool_release(wav->pool);
}
static void api_clear(void *data) {
	PJ_UNUSED_ARG(data);
	acore_mem_dec_ref(wav);
}
void wav_module_init() {
	pj_pool_t *pool = acore_pool_create("pool", ACORE_POOL_SIZE,
	ACORE_POOL_SIZE);
	wav = pj_pool_alloc(pool, sizeof(wav_mod));
	wav->pool = pool;
	wav->fun = pj_pool_alloc(pool, sizeof(wav_function));
	wav->fun->create_playback = wav_create_playback;
	wav->fun->create_record = wav_create_record;
	wav->fun->clear_port = wav_clear_port;
	acore_mem_bind(pool, wav, wav_destroy);
	acore_mem_add_ref(wav);
	acore_runtime_register_api(pool, wav_api_name, wav->fun, api_clear);
}
void wav_module_close() {
	acore_runtime_unregister_api(wav->fun);
	acore_mem_mask_destroy(wav);
}
