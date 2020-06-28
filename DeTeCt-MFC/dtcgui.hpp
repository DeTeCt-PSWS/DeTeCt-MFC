#pragma once
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "dtc.h"
#include "img2.h"
extern "C" {
	#include "max.h"
	#include "datation.h"
}
#include "serfmt.h"
#include "wrapper.h"
#include "wrapper2.h"
#include "auxfunc.h"
#include "datation2.h"
#include "dtcas3.h"

extern std::string app_title;

struct options {
// variables
	char			*filename;
	char			*ofilename;
	char			darkfilename[MAX_STRING];
	std::string		message[100];
	char			*ovfname;
	char			*sfname;
	char			*dirname;
	char			impactdirname[MAX_STRING];
	char			zipname[MAX_STRING];
	int				nsaveframe;							// Frame number to <ofilename>
	int				ostype;								// Source video type to extract frame
	int				ovtype;								// Output video type to create

// options?
	double			timeImpact;							// seconds
	double			incrLumImpact;						// mean value factor
	int				incrFrameImpact;					// Minimum number of frames for impact
	double			impact_duration_min;				// Min duration for impact
	double			radius;								// Impact radio (pixels)
	unsigned long	nframesROI;							// Number of frames for ROI calculation
	unsigned long	nframesRef;							// Number of frames for ROI calculation
	unsigned long	wROI; 								// ROI width  (CM centered)
	unsigned long	hROI;								// ROI height (CM centered)
	int				bayer;								//debayering code
	double			medSize;							// Median buffer size
	double			facSize; 							// Size factor (ROI)
	double			secSize; 							// Security factor (ROI)
	int				ROI_min_px_val; 					// Minimum value of pixel to take into account pixels for ROI calculation
	int				ROI_min_size; 						// Minimum valid pixel size for a ROI 
	double			threshold;
	double			learningRate;						// "Alpha Blending" learning rate
	double			histScale;							// Histogram scale
	int				wait;								// milliseconds
	int				thrWithMask;						// Use Mask (!=0) or not (0) for frame reference
	BOOL			viewROI; 							// View ROI
	BOOL			viewTrk; 							// View planet tracking
	BOOL			viewDif; 							// View differential frame
	BOOL			viewRef; 							// View reference frame
	BOOL			viewMsk; 							// View mask
	BOOL			viewThr; 							// View threshold
	BOOL			viewSmo;							// View frame after filter application
	BOOL			viewHis;							// View differential frame histogram
	BOOL			viewRes;							// View final frame
	BOOL			verbose;
	BOOL			debug;								// debug mode with more information
	BOOL			videotest;							// Test input video file
	BOOL			ADUdtconly;							// Use ADUdtc algorithm only
	BOOL			detail;								// Use ADUdtc algorithm only with 2 more images as output
	BOOL			zip;								// Creates zip of impact_detection folder at the end of processing
	BOOL			email;								// Send email at the end of processing
	BOOL			allframes;							// Save all intermediate mac frames from ADUdtc algorithm
	double			impact_distance_max;				// Maximum value for distance between old algorithm and max in detection image for being a possible impact
	double			impact_max_avg_min;					// Minimum value for max - mean value of dtc_max-mean image for being a possible impact
	double			impact_confidence_min;				// Minimum value for confidence for being a possible impact
	int				minframes;							// Minimum # of frames to start processing
	struct Filter	filter;
	BOOL			dateonly;							// Display date information and stops processing
	BOOL			ignore;								// Ignore incorrect frames
	int				maxinstances;						// Maximum number of DeTeCt instances running in parallel
	BOOL			reprocessing;						// Reprocessing files already in DeTeCt.log
	BOOL			interactive;						// Normal interactive mode or automatic mode
	BOOL			exit;								// Automatic exit when processing done
	BOOL			shutdown;							// Automatic PC shutdownn when auto exit

// Status
	BOOL			interactive_bak;					// Backup of interactive status
	BOOL			autostakkert;						// Launched from autostakkert
	DWORD			autostakkert_PID;					// Parent autostakkert PID
	DWORD			detect_PID;							// Parent detect PID
	char			DeTeCtQueueFilename[MAX_STRING];
	BOOL			parent_instance;
};
typedef struct options OPTS;
extern OPTS opts;

struct FrameOrder {
	bool operator()(ITEM* a, ITEM* b) {
		return a->point->frame < b->point->frame;
	}
};

struct BrightnessOrder {
	bool operator()(ITEM* a, ITEM* b) {
		return a->point->val > b->point->val;
	}
};

struct AcquisitionFilesList {
	std::vector<std::string> file_list				= {};
	std::vector<std::string> acquisition_file_list	= {};
	std::vector<int> nb_prealigned_frames			= {};
};

void			read_files(std::string folder, AcquisitionFilesList *acquisition_files);

int				itemcmp(const void *a, const void *b);
int				framecmp(const void *a, const void *b);

int				detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, double radius, double incrLum, int impact_frames_min);


int				detect(std::vector<std::string> current_file_list, OPTS *opts, std::string scan_folder_path);

char			*dtc_full_filename(const char *acquisition_filename, const char *suffix, const char *path_name, char *full_filename);

void			zip(char *zipfilename, char *item_to_be_zipped);

Instance_type	DisplayInstanceType(int *nbinstances);

void			WriteIni();

//void	LogString(CString log_cstring);