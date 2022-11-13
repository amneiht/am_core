/*
 * wav_mod.c
 *
 *  Created on: Oct 20, 2022
 *      Author: amneiht
 */

#include "wav_mod.h"
#include <acore/runtime.h>
#include <acore/mem.h>
#include <acore_media/base.h>
struct m_list {
	pjmedia_port *mport[2];
};
typedef struct wav_mod {
	pj_pool_t *pool;
	wav_function *fun;
	acore_mmap_t *map;
	acore_audio_factory_t *fac;
	void *api;
} wav_mod;

static wav_mod *wav;

static pjmedia_port* wav_api_create_input(pj_pool_t *pool, const char *name) {
	if (!acore_mem_is_available(wav))
		return NULL;
	pjmedia_port *player = NULL;
	pj_status_t status = pjmedia_wav_player_port_create(pool, name, 20, 0, 0,
			&player);

	if (status != PJ_SUCCESS)
		acore_mem_add_ref(wav);
	return player;
}
static pjmedia_port* wav_api_create_output(pj_pool_t *pool,
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

static void port_destroy(pjmedia_port *rport) {
	pjmedia_port_destroy(rport);
	acore_mem_dec_ref(wav);
}

// call back when wav module is destroy
static void wav_module_destroy(void *data) {
	wav_mod *wav = data;
	acore_pool_release(wav->pool);
}
static void wav_clear(void *data) {
	PJ_UNUSED_ARG(data);
	acore_mem_dec_ref(wav);
}
// media module mod
struct eof_cb {
	acore_media_fail_callback fail;
	void *data;
};
static void eof_call_back(pjmedia_port *prt, void *data) {
	struct eof_cb *ecb = data;
	ecb->fail(ecb->data);
}
static void wav_media_bi_port_destroy(void *data) {
	pjmedia_port *rport = data;
	pjmedia_port_destroy(rport);
	struct m_list *mlist = acore_mmap_unset(wav->map, data);
	acore_mem_dec_ref(mlist->mport[0]);
	acore_mem_dec_ref(mlist->mport[1]);
	acore_mem_dec_ref(wav);
}
static int check_sample(const pjmedia_audio_format_detail *p1,
		const pjmedia_audio_format_detail *p2) {
	if (p1->channel_count != p2->channel_count)
		return 0;
	if (p1->bits_per_sample != p2->bits_per_sample)
		return 0;
	if (p1->clock_rate != p2->clock_rate)
		return 1;
	return 2;
}
static pjmedia_port* media_create_playback(pj_pool_t *pool,
		const pj_str_t *playback, const pjmedia_audio_format_detail *aud) {
	pjmedia_port *rport = NULL;
	pjmedia_port *p_port;
	pj_status_t status = pjmedia_wav_player_port_create(pool, playback->ptr,
			aud->frame_time_usec / 1000, 0, 0, &p_port);
	if (status != PJ_SUCCESS)
		return NULL;
	switch (check_sample(&p_port->info.fmt.det.aud, aud)) {
	case 0:
		pjmedia_port_destroy(p_port);
		PJ_LOG(2,
				(wav_api_name,"canot create playback becaus not match audio formart"));

		break;
	case 1:
		//resample port create
		PJ_LOG(3, (ACORE_NAME , "create play back with resample"));
		pjmedia_resample_port_create(pool, p_port, aud->clock_rate,
				PJMEDIA_RESAMPLE_USE_LINEAR, &rport);
		acore_mem_bind(pool, rport, (acore_callback_clear) port_destroy);
		break;
	case 2:
		PJ_LOG(3, (ACORE_NAME , "create audio source without resample"));
		rport = p_port;
		acore_mem_bind(pool, rport, (acore_callback_clear) port_destroy);
		break;
	}
	return rport;
}
static pjmedia_port* wav_media_create_callback(pj_pool_t *pool,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud,
		acore_conf_t *sys_conf, acore_media_fail_callback fail,
		void *fail_callback_data, void *factory_data) {
	if (!acore_mem_is_available(wav))
		return NULL;
	acore_context_t *ctx = acore_conf_find_context2(sys_conf, "wav");
	if (ctx == NULL)
		return NULL;
	pjmedia_port *p_port = NULL, *c_port = NULL, *main_port;
	pj_status_t status;
	const pj_str_t *playback, *capture, *repeat;
	playback = acore_context_get_str2(ctx, "playback");
	capture = acore_context_get_str2(ctx, "capture");
	repeat = acore_context_get_str2(ctx, "repeat");

	if (dir & ACORE_MEDIA_DIR_INPUT) {
		p_port = media_create_playback(pool, playback, aud);
		if (!p_port) {
			PJ_LOG(1, (ACORE_NAME , "can not create wap port"));
			return NULL;
		}
		main_port = p_port;

		if (repeat && fail) {
			if (pj_strcmp2(repeat, "yes") == 0) {
				struct eof_cb *ecb = (struct eof_cb*) pj_pool_alloc(pool,
						sizeof(struct eof_cb));
				ecb->data = fail_callback_data;
				ecb->fail = fail;
				pjmedia_wav_player_set_eof_cb2(main_port, (void*) ecb,
						eof_call_back);
			}
		}
		acore_mem_bind(pool, p_port, (acore_callback_clear) port_destroy);
		acore_mem_add_ref(wav);
	}
	if (dir & ACORE_MEDIA_DIR_OUTPUT) {
		status = pjmedia_wav_writer_port_create(pool, capture->ptr,
				aud->clock_rate, aud->channel_count, PJMEDIA_AFD_SPF(aud),
				aud->bits_per_sample, 0, 0, &c_port);
		if (status != PJ_SUCCESS) {
			if (p_port) {
				acore_mem_mask_destroy(p_port);
			}
			return NULL;
		}
		acore_mem_bind(pool, c_port, (acore_callback_clear) port_destroy);
		acore_mem_add_ref(wav);
		main_port = c_port;
	}
	if (dir - ACORE_MEDIA_DIR_FULL == 0) {
		pjmedia_bidirectional_port_create(pool, p_port, c_port, &main_port);
		struct m_list *mlist = pj_pool_alloc(pool, sizeof(struct m_list));
		mlist->mport[0] = p_port;
		mlist->mport[1] = c_port;
		acore_mmap_set(pool, wav->map, main_port, mlist);
		acore_mem_bind(pool, main_port, wav_media_bi_port_destroy);
		acore_mem_add_ref(wav);
	}
	return main_port;
}
static void wav_media_port_destroy(pjmedia_port *port, void *user) {
	(void) user;
	acore_mem_mask_destroy(port);
}

void wav_module_init() {
	pj_pool_t *pool = acore_pool_create("pool", ACORE_POOL_SIZE,
	ACORE_POOL_SIZE);

	wav = pj_pool_alloc(pool, sizeof(wav_mod));
	wav->pool = pool;
	acore_mem_bind(pool, wav, wav_module_destroy);
	// init runtime module
	wav->fun = pj_pool_alloc(pool, sizeof(wav_function));
	wav->fun->input_port = wav_api_create_input;
	wav->fun->output_port = wav_api_create_output;
	wav->fun->clear_port = port_destroy;

	acore_mem_add_ref(wav);
	acore_runtime_register_api(pool, wav_api_name, wav->fun, wav_clear);
// create media module
	wav->map = acore_mmap_create(pool);
	wav->fac = acore_media_audio_register_factory(pool, wav_api_name,
			wav_media_create_callback, wav_media_port_destroy, wav_clear, wav);
	acore_mem_add_ref(wav);
}
void wav_module_close() {
	acore_runtime_unregister_api(wav->fun);
	acore_media_audio_unregister_factory(wav->fac);
	acore_mem_mask_destroy(wav);
}
