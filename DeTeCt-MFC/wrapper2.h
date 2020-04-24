#pragma once
#ifndef __WRAPPER2_H__
#define __WRAPPER2_H__

//#include <opencv2\highgui\highgui.hpp>

extern "C" {
	//#include "serfmt.h"
	//#include "filefmt.h"
	//#include "common.h"
}

#define CAPTURE_CV		0
#define CAPTURE_SER		1
#define CAPTURE_FITS	2
#define CAPTURE_FILES	3

#define DEFAULT_FPS		25.0


/*struct _DtcCapture2
{
	int type;
	int framecount;
	union
	{
		CvCapture		*capture;
		SerCapture		*sercapture;
		FileCapture		*filecapture;
		cv::VideoCapture *videocapture;
	} u;
};

typedef struct _DtcCapture2 DtcCapture2;*/

	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	DtcCapture	*dtcCaptureFromFile2(const char *fname, int *pframecount);
	cv::Mat 	dtcQueryFrame2(DtcCapture *capture, const int ignore, int *perror);
	void		dtcReinitCaptureRead2(DtcCapture **pcapture, const char *fname);
	void 		dtcReleaseCapture(DtcCapture *capture);

	BOOL		Is_Capture_OK_from_File(const std::string file, std::string *pfilename_acquisition, int *pnframe, std::wstringstream *pmessage);
	BOOL		Is_Capture_Long_Enough(const std::string file, const int nframe, std::wstringstream *pmessage);
	BOOL		Is_Capture_Special_Type(const std::string file, std::wstringstream *pmessage);
	BOOL		Is_CaptureFile_To_Be_Processed(const std::string filename_acquisition, std::wstringstream *pmessage);

#endif /* __WRAPPER_H__ */