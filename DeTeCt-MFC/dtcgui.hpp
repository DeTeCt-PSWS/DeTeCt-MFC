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

/*
struct options {
	// variables
		//char			*filename = nullptr;
		//char			*ofilename = nullptr;
		//char			*ovfname = nullptr;
		//char			*sfname;
		//char			*dirname = nullptr;
	char			filename[MAX_STRING] = { 0 };
	char			ofilename[MAX_STRING] = { 0 };
	char			darkfilename[MAX_STRING] = { 0 };
	std::string		message[MAX_STRING];
	char			ovfname[MAX_STRING] = { 0 };
	char			dirname[MAX_STRING] = { 0 };
	char			impactdirname[MAX_STRING] = { 0 };
	char			zipname[MAX_STRING] = { 0 };
	int				nsaveframe = 0;	// Frame number to <ofilename>
	int				ostype = 0;	// Source video type to extract frame
	int				ovtype = 0;	// Output video type to create

// options?
	double			timeImpact = 0;				// seconds
	double			incrLumImpact = 0;				// mean value factor
	int				incrFrameImpact = 0;				// Minimum number of frames for impact
	double			impact_duration_min = 0;				// Min duration for impact
	double			radius = 0;				// Impact radio (pixels)
	unsigned long	nframesROI = 0;				// Number of frames for ROI calculation
	unsigned long	nframesRef = 0;				// Number of frames for ROI calculation
	unsigned long	wROI = 0; 				// ROI width  (CM centered)
	unsigned long	hROI = 0;				// ROI height (CM centered)
	int				bayer = 0;				//debayering code
	double			medSize = 0;				// Median buffer size
	double			facSize = 0; 				// Size factor (ROI)
	double			secSize = 0; 				// Security factor (ROI)
	int				ROI_min_px_val = 0; 				// Minimum value of pixel to take into account pixels for ROI calculation
	int				ROI_min_size = 0; 				// Minimum valid pixel size for a ROI 
	double			threshold = 0;
	double			learningRate = 0;				// "Alpha Blending" learning rate
	double			histScale = 0;				// Histogram scale
	int				wait = 0;				// milliseconds
	int				thrWithMask = 0;				// Use Mask (!=0) or not (0) for frame reference
	BOOL			viewROI = FALSE; 			// View ROI
	BOOL			viewTrk = FALSE; 			// View planet tracking
	BOOL			viewDif = FALSE; 			// View differential frame
	BOOL			viewRef = FALSE; 			// View reference frame
	BOOL			viewMsk = FALSE; 			// View mask
	BOOL			viewThr = FALSE; 			// View threshold
	BOOL			viewSmo = FALSE;			// View frame after filter application
	BOOL			viewHis = FALSE;			// View differential frame histogram
	BOOL			viewRes = FALSE;			// View final frame
	BOOL			verbose = FALSE;
	BOOL			debug = FALSE;			// debug mode with more information
	BOOL			videotest = FALSE;			// Test input video file
	BOOL			ADUdtconly = FALSE;			// Use ADUdtc algorithm only
	BOOL			detail = FALSE;			// Use ADUdtc algorithm only with 2 more images as output
	BOOL			zip = TRUE;				// Creates zip of impact_detection folder at the end of processing
	BOOL			email = TRUE;				// Send email at the end of processing
	BOOL			allframes = FALSE;			// Save all intermediate mac frames from ADUdtc algorithm
	double			impact_distance_max = 0;				// Maximum value for distance between old algorithm and max in detection image for being a possible impact
	double			impact_max_avg_min = 0;				// Minimum value for max - mean value of dtc_max-mean image for being a possible impact
	double			impact_confidence_min = 0;				// Minimum value for confidence for being a possible impact
	int				minframes = 0;				// Minimum # of frames to start processing
	struct Filter	filter = { 0, {0,0,0,0} };
	BOOL			dateonly = FALSE;			// Display date information and stops processing
	BOOL			ignore = FALSE;			// Ignore incorrect frames
	int				maxinstances = 1;				// Maximum number of DeTeCt instances running in parallel
	BOOL			reprocessing = FALSE;			// Reprocessing files already in DeTeCt.log
	BOOL			interactive = FALSE;			// Normal interactive mode or automatic mode
	BOOL			exit = FALSE;			// Automatic exit when processing done
	BOOL			shutdown = FALSE;			// Automatic PC shutdownn when auto exit
	BOOL			flat_preparation = FALSE;			// Flag to create flat
	BOOL			clean_dir = FALSE;			// Cleans directory before processing
	BOOL			show_detect_image = TRUE;				// show detection image
	BOOL			show_mean_image = FALSE;			// show mean image
// Status
	BOOL			interactive_bak = FALSE;			// Backup of interactive status
	BOOL			autostakkert = FALSE;			// Launched from autostakkert
	DWORD			autostakkert_PID = 0;				// Parent autostakkert PID
	DWORD			detect_PID = 0;				// Parent detect PID
	char			version[MAX_STRING] = { 0 };
	char			DeTeCtQueueFilename[MAX_STRING] = { 0 };
	char			LogConsolidatedDirname[MAX_STRING] = { 0 };
	BOOL			parent_instance = FALSE;
};
typedef struct options OPTS;
*/

extern std::string message_lines[MAX_STRING];

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
	std::vector<int>		 nb_prealigned_frames	= {};
	std::vector<int64>		 acquisition_size		= {};
};

enum class _Instance_type { autostakkert_parent, parent, autostakkert_single, single, autostakkert_child, child };
typedef enum _Instance_type Instance_type;

void			read_files(std::string folder, AcquisitionFilesList *acquisition_files);

int				itemcmp(const void *a, const void *b);
int				framecmp(const void *a, const void *b);

int				detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, double radius, double incrLum, int impact_frames_min);


//int				detect(std::vector<std::string> current_file_list, OPTS *opts, std::string scan_folder_path);
int				detect(std::vector<std::string> current_file_list, std::string scan_folder_path);

char			*dtc_full_filename			(const char *acquisition_filename, const char *suffix,						const char *path_name, char *full_filename);
char			*dtc_full_filename_2suffix	(const char *acquisition_filename, const char *suffix, const char *suffix2,	const char *path_name, char *full_filename);

void			zip(char *zipfilename, char *item_to_be_zipped, std::wstring output_filename, int* log_counter);

Instance_type	DisplayInstanceType(int *nbinstances);

void			WriteIni();
void			AcquisitionFileListToQueue(AcquisitionFilesList* pacquisition_files, const CString tag_current, const int index_current, const CString out_directory, int* acquisitions_to_be_processed);

//BOOL			RemoveFromIni(const CString line_to_remove);