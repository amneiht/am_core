///*
// * haft_snd.c
// *
// *  Created on: Nov 1, 2022
// *      Author: amneiht
// */
//
//#include "media_local.h"
//#include <acore_media/sound.h>
//
//// combine unidirectional port and aud stream to port
//static void clock_callback(const pj_timestamp *ts, void *user_data) {
//	(void) ts;
//	acore_media_sound_control *dtl = user_data;
//	pjmedia_frame *fm = acore_alloca(pjmedia_frame);
//	pj_status_t st;
//	if (dtl->mode == acore_port_mode_mix_playback) {
//		st = pjmedia_port_get_frame(dtl->stream_port, fm);
//		if (st == PJ_SUCCESS) {
//			pjmedia_port_put_frame(dtl->port.combine.lport, fm);
//		}
//	} else if (dtl->mode == acore_port_mode_mix_capture) {
//		st = pjmedia_port_get_frame(dtl->port.combine.lport, fm);
//		if (st == PJ_SUCCESS) {
//			if (dtl->mute)
//				fm->type = PJMEDIA_TYPE_NONE;
//			pjmedia_port_put_frame(dtl->stream_port, fm);
//		}
//	}
//}
//ACORE_LOCAL pj_status_t _media_snd_combine_with_player(pj_pool_t *pool,
//		acore_media_sound_control *dtl, pjmedia_aud_stream *aud,
//		pjmedia_port *ply_port) {
//
//	struct custom_sound *cus = &dtl->port.combine;
//	cus->strm = aud;
//	cus->lport = ply_port;
//	dtl->mode = acore_port_mode_mix_playback;
//	pjmedia_port_info *pia = *dtl->stream_port->info;
//	return pjmedia_clock_create(pool, PJMEDIA_PIA_SRATE(pia),
//			PJMEDIA_PIA_CCNT(pia), PJMEDIA_PIA_SPF(pia), 0, clock_callback, dtl,
//			&cus->clock);
//
//}
//ACORE_LOCAL pj_status_t _media_snd_combine_with_capture(pj_pool_t *pool,
//		acore_media_sound_control *dtl, pjmedia_aud_stream *aud,
//		pjmedia_port *cap_port) {
//	struct custom_sound *cus = &dtl->port.combine;
//	cus->strm = aud;
//	cus->lport = cap_port;
//	dtl->mode = acore_port_mode_mix_capture;
//	pjmedia_port_info *pia = *dtl->stream_port->info;
//	return pjmedia_clock_create(pool, PJMEDIA_PIA_SRATE(pia),
//			PJMEDIA_PIA_CCNT(pia), PJMEDIA_PIA_SPF(pia), 0, clock_callback, dtl,
//			&cus->clock);
//}
