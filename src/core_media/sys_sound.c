/*
 * mod_alsa.c
 *
 *  Created on: Oct 17, 2022
 *      Author: amneiht
 */

#include <pjmedia/types.h>
#include <pjmedia.h>
#include <acore/runtime.h>
#include <acore/mem.h>
#include <acore/context.h>
#include <acore_media/base.h>

#define THIS_FILE "alsa"

#define alsa_copystr(a , b)   a = acore_alloca(pj_str_t); \
a->slen = b->slen ; \
a->ptr = alloca( b->slen + 1) ; \
a->ptr [ a->slen ] = 0 ;\
pj_memcpy( a->ptr , b->ptr  , b->slen )

typedef struct alsa_mod {
	PJ_DECL_LIST_MEMBER(void)
	;
	pj_pool_t *pool;
	acore_audio_factory_t *fac;
} alsa_mod;

static alsa_mod *alsa;
static int get_dev_index(const pj_str_t *dev_name) {
	if (pj_strcmp2(dev_name, "default") == 0)
		return -1;
	pj_status_t status;
	int dev_count = pjmedia_aud_dev_count();
	pjmedia_aud_dev_info info;
	pj_int32_t devid = -1;

	if (dev_count == 0) {
		PJ_LOG(3, ( THIS_FILE, "No devices found"));
		return -1;
	}
	PJ_LOG(3, (THIS_FILE, "Found %d devices:", dev_count));
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
static void alsa_port_destroy(pjmedia_port *port, void *user_data) {
	acore_mem_dec_ref(user_data);
	pjmedia_port_destroy(port);
}
static pjmedia_port* alsa_port_create(pj_pool_t *pool, const pjmedia_dir dir,
		const pjmedia_audio_format_detail *aud, acore_conf_t *sys_conf,
		void *user_data) {

	if (!acore_mem_is_available(user_data)) {
		PJ_LOG(1, ("alsa","module will be remove soon"));
		return NULL;
	}
	pj_status_t status;
	const acore_context_t *conf = acore_conf_find_context2(sys_conf, "alsa");
	if (!conf) {
		PJ_LOG(3, ("alsa","config file must has alsa context"));
		return NULL;
	}
	pjmedia_aud_dev_index dev_p = -1, dev_c = -1;
	if (dir == PJMEDIA_DIR_CAPTURE) {
		const pj_str_t *dev = acore_context_get_str2(conf, "capture");
		if (dev != NULL) {
			dev_c = get_dev_index(dev);
		}
	} else if (dir == PJMEDIA_DIR_PLAYBACK) {
		const pj_str_t *dev = acore_context_get_str2(conf, "playback");
		if (dev != NULL) {
			dev_p = get_dev_index(dev);
		}
	} else if (dir == PJMEDIA_DIR_CAPTURE_PLAYBACK) {
		const pj_str_t *dev = acore_context_get_str2(conf, "playback");
		if (dev != NULL) {
			dev_p = get_dev_index(dev);
		}
		dev = acore_context_get_str2(conf, "capture");
		if (dev != NULL) {
			dev_c = get_dev_index(dev);
		}
	}
	if (dev_c == dev_p) {
		return NULL;
	}
//	custom data
	pjmedia_snd_port_param *prm = acore_alloca(pjmedia_snd_port_param);
	pjmedia_snd_port_param_default(prm);
	prm->base.dir = dir;
	prm->base.rec_id = dev_c;
	prm->base.play_id = dev_p;
	prm->base.clock_rate = aud->clock_rate;
	prm->base.channel_count = aud->channel_count;
	prm->base.bits_per_sample = aud->bits_per_sample;
	prm->options = 0;
	prm->ec_options = 0;

	pjmedia_snd_port *port = NULL;
	switch (dir) {
	case PJMEDIA_DIR_CAPTURE:
		status = pjmedia_snd_port_create_rec(pool, dev_c, aud->clock_rate,
				PJMEDIA_AFD_SPF(aud), aud->bits_per_sample,
				aud->bits_per_sample, 0, &port);
		break;
	case PJMEDIA_DIR_PLAYBACK:
		status = pjmedia_snd_port_create_player(pool, dev_p, aud->clock_rate,
				PJMEDIA_AFD_SPF(aud), aud->bits_per_sample,
				aud->bits_per_sample, 0, &port);
		break;
	case PJMEDIA_DIR_CAPTURE_PLAYBACK:
		status = pjmedia_snd_port_create(pool, dev_c, dev_p, aud->clock_rate,
				PJMEDIA_AFD_SPF(aud), aud->bits_per_sample,
				aud->bits_per_sample, 0, &port);
		break;
	default:
		break;
	}
	if (status) {
		char buff[300];
		pj_str_t lerr = pjmedia_strerror(status, buff, 300);
		PJ_LOG(1, ("alsa","Port create : %.*s", (int)lerr.slen, lerr.ptr));
		return NULL;
	}
	pjmedia_port *custom = pj_pool_alloc(pool, sizeof(pjmedia_port));
	acore_mem_add_ref(user_data);
	return pjmedia_snd_port_get_port(port);

}
static void module_close() {
	acore_media_audio_unregister_factory(alsa->fac);
	acore_mem_mask_destroy(alsa);
}
static void module_destroy(void *data) {
	(void) data;
	acore_pool_release(alsa->pool);
}
// close alsa media factory
static void alsa_clear(void *mod) {
	acore_mem_dec_ref(mod);
}
static void module_init() {
	pj_pool_t *pool = acore_pool_create("ALSA", ACORE_POOL_SIZE,
	ACORE_POOL_SIZE);
	alsa = pj_pool_alloc(pool, sizeof(alsa_mod));
	alsa->pool = pool;
	acore_mem_bind(pool, alsa, module_destroy);
	acore_mem_add_ref(alsa);
	alsa->fac = acore_media_audio_register_factory(pool, "alsa",
			alsa_port_create, alsa_port_destroy, alsa_clear, alsa);
}
//for test
void alsa_module_init() {
	module_init();
}
void alsa_module_close() {
	module_close();
}
