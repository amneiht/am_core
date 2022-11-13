/*
 * media_local.h
 *
 *  Created on: Nov 1, 2022
 *      Author: amneiht
 */

#ifndef CORE_MEDIA_MEDIA_LOCAL_H_
#define CORE_MEDIA_MEDIA_LOCAL_H_

#include <acore/base.h>
#include <pjmedia.h>

enum acore_port_mode {
	acore_port_mode_snd_full, /// get data freo sound system
	acore_port_mode_mod_full, /// source and player as same
	acore_port_mode_mod_mix, /// mix from tow diffrence audio out put
	acore_port_mode_mix_playback, /// mix audio input from media module and system sound
	acore_port_mode_mix_capture /// mix audio output from media module and system sound
};

struct acore_media_sound_control {
	enum acore_port_mode mode;
	pjmedia_port *stream_port;
	pj_bool_t mute;
	pjmedia_clock *clock;
	union {
		struct {
			pjmedia_aud_stream *aud;
		} snd;
		struct {
			// capture and play back from one module
			pjmedia_port *media;
		} one_p;
		struct {
			//diffrence capture and play back module
			pjmedia_port *input;
			pjmedia_port *output;
		} two_p;
		struct custom_sound {
			pjmedia_aud_stream *strm;
			pjmedia_port *lport;
		} combine;
	} port;

};
#endif /* CORE_MEDIA_MEDIA_LOCAL_H_ */
