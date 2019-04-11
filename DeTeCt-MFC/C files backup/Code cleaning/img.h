#ifndef __IMG_H__
#define __IMG_H__
#include "common.h"

#include <opencv/cv.h>
#include <opencv/highgui.h> 
#include "dirent.h"

#include "wrapper.h"

#define KR	0.299
#define KG	0.587
#define KB	0.114

struct _DtcImageVals {
	double lum;
	double minlum;
	double maxlum;
};

typedef struct _DtcImageVals DtcImageVals;

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/
	
void 			dtcGetROI(IplImage *img, int *xorig, int *yorig, int *width, int *height);

CvPoint 		dtcGetCM(IplImage* img);
CvPoint 		dtcGetGrayCM(IplImage *img);
CvPoint 		dtcGetGrayMatCM(CvMat *mat);

CvRect 			dtcGetImageROIcCM(IplImage *img, CvPoint cm, int medsize, double fact, double secfact);

IplImage 		*dtcGetGray(IplImage *frame);					//cvCreateImage
CvMat 			*dtcGetGrayMat(IplImage *frame);					//cvCreateMat

IplImage 		*dtcReduceToROI(IplImage **src, CvRect roi);	//cvCreateImage (cvRelease src)
CvMat 			*dtcReduceMatToROI(CvMat **src, CvRect roi);		//cvCreateMat (cvRelease src)

CvRect 			dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore);

CvRect 			dtcMaxRect(CvRect one, CvRect two);

void 			dtcDrawCM(IplImage *image, CvPoint cm);

IplImage 		*dtcRunningAvg(IplImage *imgsrc, IplImage *imgdst, double lR);

CvVideoWriter 	*dtcWriteVideo(const char *file, CvVideoWriter *writer, DtcCapture *capture, IplImage *img);										//cvCreateVideoWriter
	
IplImage 		*dtcLumThreshold_ToZero(IplImage *src, IplImage *dst, double threshold);

double 			dtcGetImageLum(IplImage *img);

DtcImageVals 	dtcGetGrayImageVals(IplImage *img);
DtcImageVals 	dtcGetImageVals(IplImage *img);

IplImage 		*dtcGetHistogramImage(CvArr *src, int scale, double thr);	//cvCreateImage

void 			dtcShowPhotometry(CvMat *mat, int frame);

#endif /* __IMG_H__ */
