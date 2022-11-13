/*
 * loop_back.c
 *
 *  Created on: Nov 5, 2022
 *      Author: amneiht
 */

#include <acore_media/base.h>
#include <acore_media/sound.h>

struct acore_loop_media* acore_media_loop_back(pj_pool_t *pool, int playload) {
	const pjmedia_codec_info *pika;
	pjmedia_stream_info si;
	pj_status_t st;
	st = pjmedia_codec_mgr_get_codec_info(
			pjmedia_endpt_get_codec_mgr(acore_media_endpt()), playload, &pika);
	if (st)
		return NULL;
	pjmedia_codec_param *prm = acore_alloca(pjmedia_codec_param);
	st = pjmedia_codec_mgr_get_default_param(
			pjmedia_endpt_get_codec_mgr(acore_media_endpt()), pika, prm);
	if (st)
		return NULL;

	struct acore_loop_media *loop = pj_pool_alloc(pool,
			sizeof(struct acore_loop_media));
	st = pjmedia_transport_loop_create(acore_media_endpt(), &loop->tp);
	if (st) {
		PJ_LOG(1, (ACORE_NAME ,"mod loop"));
		return NULL;
	}
	// setting param
	pj_bzero(&si, sizeof(si));
	si.type = PJMEDIA_TYPE_AUDIO;
	si.proto = PJMEDIA_TP_PROTO_RTP_AVP;
	si.dir = PJMEDIA_DIR_ENCODING_DECODING;
	pj_sockaddr_in_init(&si.rem_addr.ipv4, NULL, 4000);
	pj_sockaddr_in_init(&si.rem_rtcp.ipv4, NULL, 4001);
	pj_memcpy(&si.fmt, pika, sizeof(pjmedia_codec_info));
	si.param = NULL;
	si.tx_pt = pika->pt;
	si.tx_event_pt = 101;
	si.rx_event_pt = 101;
	si.ssrc = pj_rand();
	si.jb_init = si.jb_min_pre = si.jb_max_pre = si.jb_max = -1;

	st = pjmedia_stream_create(acore_media_endpt(), pool, &si, loop->tp,
	NULL, &loop->stream);

	if (st) {
		PJ_LOG(1, (ACORE_NAME ,"mod no loop on stream"));
		return NULL;
	}

	st |= pjmedia_stream_get_port(loop->stream, &loop->loop_port);
	if (st) {
		PJ_LOG(1, (ACORE_NAME ,"mod no loop on port"));
		return NULL;
	}
	return loop;
}

