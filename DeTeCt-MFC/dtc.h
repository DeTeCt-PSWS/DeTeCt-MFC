#ifndef __DTC_H__
#define __DTC_H__

//#include "common.h"
#include "cmdline.h"

#define PROGNAME		"DeTeCt"
#define LONGNAME		"jovian impact DeTeCtion"
#define VERSION_NB		"3.2.3"
#define VERSION_DATE	"(Apr.10,2019)"

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

#define DTC_MAX_MEAN_SUFFIX		"_dtc_max-mean.jpg"
#define DTC_MAX_MEAN1_SUFFIX	"_dtc_max-mean1.jpg"
#define DTC_MAX_MEAN2_SUFFIX	"_dtc_max-mean2.jpg"
#define DTC_MEAN_SUFFIX			"_dtc_mean.jpg"
#define DTC_MEAN2_SUFFIX		"_dtc_mean2.jpg"
#define DTC_DIFF_SUFFIX			"_dtc_diff.jpg"
#define DTC_DIFF2_SUFFIX		"_dtc_diff2.jpg"
#define VIDEOTEST_SUFFIX		"_videotest.jpg"
#define DTC_MAX_SUFFIX			"_dtc_max.jpg"
#define MEAN_SUFFIX				"_mean.jpg"
#define DTC_SUFFIX				"_dtc.jpg"

#define DTC_INI_SUFFIX			L".ini"
#define DTC_LOG_SUFFIX			L".log"
#define OUTPUT_FILENAME			L"output"

#define DTC_QUEUE_PREFIX		"_processes_queue"
#define DTC_MAX_FRAME_PREFIX	"_dtc_max_frame"
#define DTC_DIFF_FRAME_PREFIX	"_dtc_diff_frame"
#define SINGLE_PREFIX			"_single_"

#define DTC_QUEUE_EXT			".lst"
#define AUTOSTAKKERT_EXT		"as3"
#define WJ_DEROT_STRING			"-DeRot."
#define WJ_DEROT_EXT			"drs.xml"
#define WJ_DEROT_EXT_OLD		"drs"
#define PIPP_STRING				"_pipp."
#define DARK_STRING				"dark"
#define IGNORE_WJ_DEROTATION	FALSE
#define IGNORE_PIPP				FALSE
#define IGNORE_DARK				TRUE

#ifdef __cplusplus
extern "C" {
#endif
extern int debug_mode;
#ifdef __cplusplus
	}
#endif

enum _Planet_type { Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptun, Notdefined };
typedef enum _Planet_type Planet_type;

enum _Rating_type { Error, Null, Low, High };
typedef enum _Rating_type Rating_type;

enum _Instance_type { autostakkert_parent, parent, autostakkert_single, single, autostakkert_child, child };
typedef enum _Instance_type Instance_type;

#endif /* __DTC_H__ */