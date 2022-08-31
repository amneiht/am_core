/*
 * avoip.h
 *
 *  Created on: Aug 17, 2022
 *      Author: amneiht
 */

#ifndef CORE_AVOIP_H_
#define CORE_AVOIP_H_

#ifdef __cplusplus
	extern "C" {
#endif

#include <acore_conf.h>

#include <acore/acore_base.h>
#include <acore/acore_event.h>
#include <acore/acore_handle.h>
#include <acore/acore_timer.h>
#include <acore/acore_job.h>

#include <acore/acore_ui.h>



#include <string.h>
#define ACORE_NAME (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)  // file name
#define ACORE_LINE   __LINE__  // line in name
#define ACORE_FUNC	 __func__  // funtion name_

#ifdef __cplusplus
	}
#endif

#endif /* CORE_AVOIP_H_ */
