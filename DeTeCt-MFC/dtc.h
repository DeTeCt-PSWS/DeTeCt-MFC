#ifndef __DTC_H__
#define __DTC_H__

//#include "common.h"
#include "cmdline.h"

#define PROGNAME  "DeTeCt"
#define LONGNAME  "jovian impact DeTeCtion"
#define VERSION_NB   "3.1.8"
#define VERSION_DATE "(Apr.10,2019)"

//#define VERSION_MSVC ""

#if defined _WIN64
	#define DETECT_TARGET "x64"
#elif defined  _WIN32
	#define DETECT_TARGET "x86"
#endif

#define FULL_PROGNAME PROGNAME " v" VERSION_NB "_" DETECT_TARGET " (" __DATE__ ")"


#if defined(_MSC_VER) && (_MSC_VER >= 1600)
#define VERSION_MSVC "_MSVS"
#endif

#define COPYRIGHT "Luis Calderon/Marc Delcroix/Jon Juaristi"

#define DTC_MAX_MEAN1_SUFFIX  "_dtc_max-mean1.jpg"
#define DTC_MAX_MEAN2_SUFFIX  "_dtc_max-mean2.jpg"
#define DTC_MEAN_SUFFIX  "_dtc_mean.jpg"
#define DTC_DIFF_MEAN_SUFFIX  "_dtc_diff_mean.jpg"
#define VIDEOTEST_SUFFIX  "_videotest.jpg"
#define DTC_MAX_SUFFIX  "_dtc_max.jpg"
#define MEAN_SUFFIX  "_mean.jpg"
#define DTC_SUFFIX  "_dtc.jpg"

#define DTC_QUEUE_SUFFIX L"_processes_queue.lst"
#define DTC_INI_SUFFIX L".ini"
#define DTC_LOG_SUFFIX L".log"

#define DTC_MAX_FRAME_PREFIX  "_dtc_max_frame"
#define DTC_DIFF_FRAME_PREFIX  "_dtc_diff_frame"
#define SINGLE_PREFIX "_single_"

#ifdef __cplusplus
extern "C" {
#endif
extern int debug_mode;
#ifdef __cplusplus
	}
#endif

#endif /* __DTC_H__ */