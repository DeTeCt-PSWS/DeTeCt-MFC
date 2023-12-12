#ifndef __DTC_H__
#define __DTC_H__

#include "common.h"
#include "cmdline.h"

#define PROGNAME		"DeTeCt"
#define LONGNAME		"jovian impact DeTeCtion"
#define VERSION_NB		"3.7.2"
#define VERSION_DATE	"(Aug.26,2020)"

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
#define DTC_FLAT_PREP_SUFFIX	"_flat_prep.jpg"
#define VIDEOTEST_SUFFIX		"_videotest.jpg"
#define DTC_MAX_SUFFIX			"_dtc_max.jpg"
#define MEAN_SUFFIX				"_mean.jpg"
#define DTC_SUFFIX				"_dtc.jpg"

#define DTC_INI_SUFFIX			L".ini"
#define DTC_LOG_SUFFIX			L".log"
#define OUTPUT_FILENAME			L"output"
#define WARNINGS_FILENAME		L"output_warnings"
#define ERRORS_FILENAME			L"output_errors"

#define DTC_QUEUE_PREFIX		"_processes_queue"
#define DTC_MAX_FRAME_PREFIX	"_dtc_max_frame"
#define DTC_DIFF_FRAME_PREFIX	"_dtc_diff_frame"
#define SINGLE_PREFIX			"_single_"

#define DTC_QUEUE_EXT			".lst"
#define VIDEOS_EXT				"m4v", "avi", "ser", "wmv" , "mp4", "mov"
#define FILES_EXT				"bmp", "jpg", "jpeg", "jp2", "dib", "png", "p?m", "sr", "ras", "tif", "tiff", "fit", "fits"
#define AUTOSTAKKERT_EXT		"as3"
#define WJ_DEROT_EXT			"drs.xml"
#define WJ_DEROT_EXT_OLD		"drs"
#define FULLFILENAME_NUMBER		"_000000.", "_000001.", "_00000.", "_00001.", "_0000.", "_0001.", "000000.", "000001.", "00000.", "00001.", "0000.", "0001.", "F0.", "nb1.", "_0."
#define FILENAME_NUMBER			"000000_", "000001_", "00000_", "00001_", "0000_", "0001_", "000000", "000001", "00000", "00001", "0000", "0001"
#define DARK_STRING				"dark"
#define PIPP_STRING				"_pipp"
#define WJ_DEROT_STRING			"-DeRot."

#define IGNORE_WJ_DEROTATION	FALSE
#define IGNORE_PIPP				FALSE
#define IGNORE_DARK				TRUE

#ifdef __cplusplus
//extern "C" {
#endif
extern int debug_mode;
#ifdef __cplusplus
//}
#endif

enum _Planet_type { Mercury, Venus, Mars, Jupiter, Saturn, Uranus, Neptun, Notdefined };
typedef enum _Planet_type Planet_type;

#ifdef __cplusplus
//extern "C" {
#endif
struct options {
	// variables
		/*char			*filename = nullptr;
		char			*ofilename = nullptr;
		char			*ovfname = nullptr;
		char			*sfname;
		char			*dirname = nullptr;*/
	char			filename[MAX_STRING];
	char			ofilename[MAX_STRING];
	char			darkfilename[MAX_STRING];
	//std::string			message[MAX_STRING];
	char			ovfname[MAX_STRING];
	char			dirname[MAX_STRING];
	char			impactdirname[MAX_STRING];
	char			zipname[MAX_STRING];
	int				nsaveframe;	// Frame number to <ofilename>
	int				ostype;	// Source video type to extract frame
	int				ovtype;	// Output video type to create

// options?
	double			timeImpact;				// seconds
	double			incrLumImpact;				// mean value factor
	int				incrFrameImpact;				// Minimum number of frames for impact
	double			impact_duration_min;				// Min duration for impact
	double			radius;				// Impact radio (pixels)
	unsigned long	nframesROI;				// Number of frames for ROI calculation
	unsigned long	nframesRef;				// Number of frames for ROI calculation
	unsigned long	wROI; 				// ROI width  (CM centered)
	unsigned long	hROI;				// ROI height (CM centered)
	int				bayer;				//debayering code
	double			medSize;				// Median buffer size
	double			facSize; 				// Size factor (ROI)
	double			secSize; 				// Security factor (ROI)
	int				ROI_min_px_val; 				// Minimum value of pixel to take into account pixels for ROI calculation
	int				ROI_min_size; 				// Minimum valid pixel size for a ROI 
	double			threshold;
	double			learningRate;				// "Alpha Blending" learning rate
	double			histScale;				// Histogram scale
	int				wait;				// milliseconds
	int				thrWithMask;				// Use Mask (!=0) or not (0) for frame reference
	bool			viewROI; 			// View ROI
	bool			viewTrk; 			// View planet tracking
	bool			viewDif; 			// View differential frame
	bool			viewRef; 			// View reference frame
	bool			viewMsk; 			// View mask
	bool			viewThr; 			// View threshold
	bool			viewSmo;			// View frame after filter application
	bool			viewHis;			// View differential frame histogram
	bool			viewRes;			// View final frame
	bool			verbose;
	bool			debug;			// debug mode with more information
	bool			videotest;			// Test input video file
	bool			ADUdtconly;			// Use ADUdtc algorithm only
	bool			detail;			// Use ADUdtc algorithm only with 2 more images as output
	bool			zip;				// Creates zip of impact_detection folder at the end of processing
	bool			email;				// Send email at the end of processing
	bool			allframes;			// Save all intermediate mac frames from ADUdtc algorithm
	double			impact_distance_max;				// Maximum value for distance between old algorithm and max in detection image for being a possible impact
	double			impact_max_avg_min;				// Minimum value for max - mean value of dtc_max-mean image for being a possible impact
	double			impact_confidence_min;				// Minimum value for confidence for being a possible impact
	int				minframes;				// Minimum # of frames to start processing
	struct Filter	filter;
	bool			dateonly;			// Display date information and stops processing
	bool			ignore;				// Ignore incorrect frames
	int				maxinstances;				// Maximum number of DeTeCt instances running in parallel
	bool			reprocessing;			// Reprocessing files already in DeTeCt.log
	bool			interactive;			// Normal interactive mode or automatic mode
	bool			autoexit;			// Automatic exit when processing done
	bool			shutdown;			// Automatic PC shutdownn when auto exit
	bool			flat_preparation;			// Flag to create flat
	bool			clean_dir;			// Cleans directory before processing
	bool			show_detect_image;				// show detection image
	bool			show_mean_image;			// show mean image
	double			bg_detection_peak_factor;			// for min threshold to detect background
	int				bg_detection_consecutive_values;	// # of consecutive frames to be below peak factor for background detection
	int				transparency_min_pc;					// tolerance in transparency for a frame compared to 1st frame
	int				similarity_decrease_max_pc;			// max decrease between two frames similarity
// Status
	bool			interactive_bak;			// Backup of interactive status
	bool			autostakkert;			// Launched from autostakkert
	DWORD			autostakkert_PID;				// Parent autostakkert PID
	DWORD		 	detect_PID;				// Parent detect PID
	char			version[MAX_STRING];
	char			DeTeCtQueueFilename[MAX_STRING];
	char			LogConsolidatedDirname[MAX_STRING];
	char			WarningsFilename[MAX_STRING];
	char			ErrorsFilename[MAX_STRING];
	bool			parent_instance;
};
typedef struct options OPTS;

#ifdef __cplusplus
//}
#endif

#endif /* __DTC_H__ */