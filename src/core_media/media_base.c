#include <acore_media/base.h>
#include <acore/mem.h>
#include <acore/list.h>
#include <pjmedia-codec.h>

#define this "media"

#define _begin 	pj_lock_acquire(msys->lock); {
#define _end	} pj_lock_release(msys->lock);
static pj_bool_t init = PJ_FALSE;
struct acore_audio_factory_t {
	ACORE_DECL_LIST_ELEMENT(acore_audio_factory_t )
	;
	pj_str_t *name;
	acore_audio_create_callback create;
	acore_audio_destroy_callback destroy;
};
typedef struct media_sys {
	pj_pool_t *pool;
	pj_lock_t *lock;
	pjmedia_endpt *m_endpt;
	acore_conf_t *conf;
	acore_mmap_t *map;
	acore_list_t *aud;
	// for future
	acore_list_t *vid;
	pjmedia_event_mgr *mgr;
} media_sys;

static media_sys *msys = NULL;

acore_conf_t* acore_media_default_conf() {
	if (msys)
		return msys->conf;
	return NULL;
}
static void media_destroy(void *data) {
	media_sys *media = (media_sys*) data;
	pj_lock_destroy(media->lock);
	pjmedia_aud_subsys_shutdown();
	acore_pool_release(media->pool);
}
static void seting_codec(void *data, const pj_str_t *value) {
	pjmedia_endpt *mep = data;
	if (pj_strcmp2(value, "g711") == 0) {
#if defined (PJMEDIA_HAS_G711_CODEC) && PJMEDIA_HAS_G711_CODEC !=0
		pjmedia_codec_g711_init(mep);
		PJ_LOG(3, ("media" , "G711 codec init"));
#endif
	} else if (pj_strcmp2(value, "g721") == 0) {
#if defined (PJMEDIA_HAS_G7221_CODEC) && PJMEDIA_HAS_G7221_CODEC
		pjmedia_codec_g7221_init(mep);
		PJ_LOG(3, ("media" , "G721 codec init"));
#else
		PJ_LOG(3, ("media" , "G721 is not suppport"));
#endif
	} else if (pj_strcmp2(value, "g729") == 0) {
#if defined (PJMEDIA_HAS_BCG729) && PJMEDIA_HAS_BCG729 !=0
	pjmedia_codec_bcg729_init(mep);
	PJ_LOG(3, (this , "BCG729 codec init"));

#else
		PJ_LOG(3, ("media" , "G729 is not suppport"));
#endif
	} else if (pj_strcmp2(value, "speex") == 0) {
#if  defined (PJMEDIA_HAS_SPEEX_CODEC) && PJMEDIA_HAS_SPEEX_CODEC !=0
		pjmedia_codec_speex_init_default(mep);
		PJ_LOG(3, (this , "Speex codec init"));
#else
		PJ_LOG(3, ("media" , "speex is not suppport"));
#endif
	} else if (pj_strcmp2(value, "l16") == 0) {
//PJMEDIA_HAS_ILBC_CODEC
#if defined (PJMEDIA_HAS_L16_CODEC) && PJMEDIA_HAS_L16_CODEC !=0
		PJ_LOG(3, (this , "L16 codec init"));
		pjmedia_codec_l16_init(mep, 0);
#endif
	} else if (pj_strcmp2(value, "ilbc") == 0) {
#if defined (PJMEDIA_HAS_ILBC_CODEC)  && PJMEDIA_HAS_ILBC_CODEC !=0
		PJ_LOG(3, (this , "Ilbc codec init"));
		pjmedia_codec_ilbc_init(mep, 20);
#endif
	} else if (pj_strcmp2(value, "amr") == 0) {
#if defined (PJMEDIA_HAS_OPENCORE_AMRNB_CODEC) && PJMEDIA_HAS_OPENCORE_AMRNB_CODEC !=0
PJ_LOG(3, (this , "AMRNB_CODEC codec init"));
pjmedia_codec_opencore_amrnb_init(mep);
#endif
	} else if (pj_strcmp2(value, "silk") == 0) {
#if  defined (PJMEDIA_HAS_SILK_CODEC) && PJMEDIA_HAS_SILK_CODEC != 0
PJ_LOG(3,(this , "SILK codec init"));
pjmedia_codec_silk_init(mep);
#endif
	}
}
pj_status_t acore_media_init(acore_conf_t *conf) {
	if (init)
		return PJ_SUCCESS;
	pj_status_t status = pjmedia_aud_subsys_init(acore_pool_factory());
	if (status != PJ_SUCCESS)
		return status;
	// create endpoint
	pjmedia_endpt *mep;
	status = pjmedia_endpt_create2(acore_pool_factory(), NULL, 0, &mep);
	if (status != PJ_SUCCESS)
		return status;
	// create sys
	pj_pool_t *pool = acore_pool_create("core_media", ACORE_POOL_SIZE,
	ACORE_POOL_SIZE);
	msys = pj_pool_alloc(pool, sizeof(media_sys));
	msys->pool = pool;
	msys->m_endpt = mep;
	msys->map = acore_mmap_create(pool);
	msys->conf = conf;
	//	media_list
	msys->aud = acore_list_create(pool, NULL);
	msys->vid = acore_list_create(pool, NULL);

	// create even manager wiht thread allow
	pjmedia_event_mgr_create(pool, 0, &msys->mgr);

	pj_lock_create_recursive_mutex(pool, "media_lock", &msys->lock);
	acore_context_t *ctx = acore_conf_find_context2(conf, "core");

	ctx = acore_conf_find_context2(conf, "audio");
	if (ctx) {
		acore_context_list_value2(ctx, "codec", seting_codec, mep);
	}
	acore_mem_bind(pool, msys, media_destroy);

	return PJ_SUCCESS;
}
void acore_media_close() {
	acore_mem_mask_destroy(msys);
}

pjmedia_endpt* acore_media_endpt() {
	return msys->m_endpt;

}
static void release_audio(void *data) {
	acore_audio_factory_t *fac = data;
	_begin
		acore_list_remove(msys->aud, fac);
	_end
}
acore_audio_factory_t* acore_media_audio_register_factory(pj_pool_t *pool,
		const char *name, acore_audio_create_callback create,
		acore_audio_destroy_callback des, acore_callback_clear clear,
		void *user_data) {
	acore_audio_factory_t *fax = NULL;
	if (!acore_mem_is_available(msys))
		return NULL;
	_begin
		fax = acore_list_ele_create(pool, sizeof(acore_audio_factory_t));
		fax->clear = clear;
		fax->create = create;
		fax->destroy = des;
		fax->user_data = user_data;
		fax->name = pj_pool_alloc(pool, sizeof(pj_str_t));
		pj_strdup2_with_null(pool, fax->name, name);
		acore_mem_bind(pool, fax, release_audio);
		acore_list_add(msys->aud, fax);
	_end
	return fax;
}

void acore_media_audio_unregister_factory(acore_audio_factory_t *fac) {
	acore_mem_mask_destroy(fac);
}

static int find_by_name(void *value, const void *list) {
	const acore_audio_factory_t *fac = list;
	if (pj_strcmp(fac->name, value) == 0) {
		return acore_ele_found;
	}
	return acore_ele_notfound;
}
pjmedia_port* acore_media_audio_create(pj_pool_t *pool, const pj_str_t *name,
		const pjmedia_dir dir, const pjmedia_audio_format_detail *aud,
		acore_media_fail_callback fail, void *callback_data) {
	pjmedia_port *res = NULL;
	if (!acore_mem_is_available(msys))
		return NULL;

	acore_audio_factory_t *fac = acore_list_search(msys->aud, find_by_name,
			(void*) name);
	if (!fac || !acore_mem_is_available(fac))
		return NULL;
	res = fac->create(pool, dir, aud, msys->conf, fail, callback_data,
			fac->user_data);
	if (res) {
		acore_mmap_set(pool, msys->map, res, fac);
		acore_mem_add_ref(fac);
		acore_mem_add_ref(msys);
	}
	return res;
}
pj_bool_t acore_media_port_destroy(pjmedia_port *port) {
	acore_audio_factory_t *fac = acore_mmap_get(msys->map, port);
	if (fac == NULL) {
		PJ_LOG(2, ("media","this port is not create by acore media"));
		return PJ_FALSE;
	}
	_begin
		acore_mmap_unset(msys->map, port);
		fac->destroy(port, fac->user_data);
	_end
	acore_mem_dec_ref(fac);
	acore_mem_dec_ref(msys);
	return PJ_TRUE;
}

