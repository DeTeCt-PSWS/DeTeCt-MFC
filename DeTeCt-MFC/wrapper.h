#ifndef __WRAPPER_H__
#define __WRAPPER_H__

//#include <opencv/highgui.h>
//#include <opencv2\highgui\highgui.hpp>


#include "serfmt.h"
#include "filefmt.h"
//#include "common.h"

//#include <opencv2/videoio/videoio_c.h> //TEST opencv3

enum _CaptureType { CAPTURE_CV, CAPTURE_SER, CAPTURE_FITS, CAPTURE_FILES };
typedef enum _CaptureType CaptureType;

/*#define CAPTURE_CV		0
#define CAPTURE_SER		1
#define CAPTURE_FITS	2
#define CAPTURE_FILES	3*/

#define DEFAULT_FPS		25.0

//#pragma warning (disable: 4430)

enum			_BOOL_TYPE{ NotSet, False, True};
typedef	 enum	_BOOL_TYPE BOOL_TYPE;

struct _DtcCaptureInfo {
	char			observer[MAX_STRING]; //  = {};
	char			location[MAX_STRING]; //  = {};
	char			scope[MAX_STRING];	//  = {};
	char			camera[MAX_STRING]; //  = {};
	char			filter[MAX_STRING]; //  = {};
	char			profile[MAX_STRING]; //  = {};
	double			diameter_arcsec;	// = -1.0;
	double			magnitude;			// = -1000.0;
	char			centralmeridian[MAX_STRING]; //  = {};
	int				focallength_mm;		// = -1;
	double			resolution;			// = -1.0;
	char			binning[MAX_STRING]; //  = {};
	int				bitdepth;			// = -1;
	BOOL_TYPE		debayer;			// = NotSet;
	double			exposure_ms;			// = -1.0;
	int				gain;				// = -1;
	int				gamma;				// = -1;
	BOOL_TYPE		autoexposure;		// = NotSet;
	int				softwaregain;		// = -1;
	int				autohisto;			// = -1;
	int				brightness;			// = -1;
	BOOL_TYPE		autogain;			// = NotSet;
	int				histmin;			// = -1;
	int				histmax;			// = -1;
	int				histavg_pc;			// = -1;
	double			noise;				// = -1.0;
	char			prefilter[MAX_STRING];	// = {};
	double			temp_C;				// = -1000.0;
	char			target[MAX_STRING]; //  = {};
};

typedef struct _DtcCaptureInfo DtcCaptureInfo;


struct _DtcCapture
{
	CaptureType				type;
	int						framecount;
	union
	{
		CvCapture			*capture;
		SerCapture			*sercapture;
		FileCapture			*filecapture;
		//cv::VideoCapture *videocapture;
	} u;
//	DtcCaptureInfo			*pCaptureInfo;
	DtcCaptureInfo			CaptureInfo;
};

typedef struct _DtcCapture DtcCapture;

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/

double 		dtcGetCaptureProperty(DtcCapture *capture, int property_id);
void		initDtcCaptureInfo(DtcCaptureInfo* pCaptureInfo);

#endif /* __WRAPPER_H__ */