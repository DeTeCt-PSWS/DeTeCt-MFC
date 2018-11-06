#ifndef __CMDLINE_H__
#define __CMDLINE_H__

#include "common.h"

#define OTYPE_NO		0
#define OTYPE_DIF		1
#define OTYPE_TRK		2
#define OTYPE_ROI		3
#define OTYPE_HIS		4
#define OTYPE_MSK		5
#define OTYPE_ADU		6

// Additions by Jon
#define FILTER_BLUR		1
#define FILTER_MEDIAN	2
#define FILTER_GAUSSIAN 3

	struct Filter
	{
		int type;   // CV_BLUR, CV_MEDIAN o CV_GAUSSIAN
		int param[4]; // width, height, sigma, 0
	};

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
		int viewROI; // View ROI
		int viewTrk; // View planet tracking
		int viewDif; // View differential frame
		int viewRef; // View reference frame
		int viewMsk; // View mask
		int viewThr; // View threshold
		int viewSmo; // View frame after filter application
		int viewHis; // View differential frame histogram
		int viewRes; // View final frame
		int verbose;
		int debug;
		int videotest; // Test input video file
		int ADUdtconly; // Use ADUdtc algorithm only
		int detail; // Use ADUdtc algorithm only with 2 more images as output
		int allframes; // Save all intermediate mac frames from ADUdtc algorithm
		int minframes; // Minimum # of frames to start processing
		struct Filter filter;
		int dateonly; // Display date information and stops processing
		int ignore; // Ignore incorrect frames
	};
	typedef struct options OPTS;

	extern OPTS opts;


	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	void	parse_command_line_options(int argc, char *argv[], OPTS *opts);

#endif /* __CMDLINE_H__ */
