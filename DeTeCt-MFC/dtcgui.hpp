#pragma once

#include <string>
#include <iostream>
#include <stdio.h>
#include <sys/stat.h>
#if !defined(_MSC_VER)
#include <unistd.h>
#endif

#include "dtc.h"
#include "img2.h"
extern "C" {
	#include "max.h"
	#include "datation.h"
	//#include "serfmt.h"
}
#include "serfmt.h"
#include "wrapper.h"
#include "wrapper2.h"
#include "auxfunc.h"
#include "datation2.h"
//#include "max2.h"
#include "dtcas3.h"
//#include "processes_queue.h"

#include <vector>

extern std::string full_version;

struct options {
	char *filename;
	char *ofilename;
	char *darkfilename;
	char *ovfname;
	char *sfname;
	char *dirname;
	int nsaveframe; // Frame number to <ofilename>
	int ostype; // Source video type to extract frame
	int ovtype; // Output video type to create
	double timeImpact; // seconds
	double incrLumImpact; // mean value factor
	int incrFrameImpact; // Number of frames
	double radius; // Impact radio (pixels)
	unsigned long nframesROI; // Number of frames for ROI calculation
	unsigned long nframesRef; // Number of frames for ROI calculation
	unsigned long wROI; // ROI width  (CM centered)
	unsigned long hROI; // ROI height (CM centered)
	int bayer; //debayering code
	double medSize; // Median buffer size
	double facSize; // Size factor (ROI)
	double secSize; // Security factor (ROI)
	double threshold;
	double learningRate; // "Alpha Blending" learning rate
	double histScale; // Histogram scale
	int wait;      // milliseconds
	int thrWithMask;// Use Mask (!=0) or not (0) for frame reference
	BOOL viewROI; // View ROI
	BOOL viewTrk; // View planet tracking
	BOOL viewDif; // View differential frame
	BOOL viewRef; // View reference frame
	BOOL viewMsk; // View mask
	BOOL viewThr; // View threshold
	BOOL viewSmo; // View frame after filter application
	BOOL viewHis; // View differential frame histogram
	BOOL viewRes; // View final frame
	BOOL verbose;
	BOOL debug;
	BOOL videotest; // Test input video file
	BOOL ADUdtconly; // Use ADUdtc algorithm only
	BOOL detail; // Use ADUdtc algorithm only with 2 more images as output
	BOOL allframes; // Save all intermediate mac frames from ADUdtc algorithm
	int minframes; // Minimum # of frames to start processing
	struct Filter filter;
	BOOL dateonly; // Display date information and stops processing
	BOOL ignore; // Ignore incorrect frames
	BOOL interactive; // Normal interactive mode or automatic mode
	int maxinstances; // Maximum number of DeTeCt instances running in parallel
	BOOL reprocessing;
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

void	read_files(std::string folder, std::vector<std::string> *file_list, std::vector<std::string> *acquisition_file_list);

int		itemcmp(const void *a, const void *b);

int		framecmp(const void *a, const void *b);

int		detect_impact(DTCIMPACT *dtc, DTCIMPACT *dtcout, double meanValue, LIST *list, ITEM** dtcMax, int fps, double radius,
	double incrLum, int incrFrame);

//int		detect(std::vector<std::string> file_list, OPTS opts);

int		detect(std::vector<std::string> file_list, OPTS opts, std::string scan_folder_path);

void	StreamDeTeCtOSversions(std::wstringstream *ss);

void	GetOSversion(std::string *pos_version);