#ifndef __WRAPPER_H__
#define __WRAPPER_H__

//#include <opencv/highgui.h>
#include <opencv2\highgui\highgui.hpp>


#include "serfmt.h"
#include "filefmt.h"
#include "common.h"

#define CAPTURE_CV		0
#define CAPTURE_SER		1
#define CAPTURE_FITS	2
#define CAPTURE_FILES	3

#define DEFAULT_FPS		25.0

struct _DtcCapture
{
	int type;
	int framecount;
	union
	{
		CvCapture		*capture;
		SerCapture		*sercapture;
		FileCapture		*filecapture;
		//cv::VideoCapture *videocapture;
	} u;
};

typedef struct _DtcCapture DtcCapture;

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/

DtcCapture	*dtcCaptureFromFile(const char *fname, int *pframecount);
void 		dtcReinitCaptureRead(DtcCapture **pcapture, const char *fname);
void 		dtcReleaseCapture(DtcCapture *capture);
IplImage 	*dtcQueryFrame(DtcCapture *capture, const int ignore, int *perror);
double 		dtcGetCaptureProperty(DtcCapture *capture, int property_id);


#endif /* __WRAPPER_H__ */