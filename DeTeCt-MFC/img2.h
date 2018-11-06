#pragma once
#ifndef __IMG2_H__
#define __IMG2_H__


#include <opencv2\imgproc\imgproc.hpp>
#include <opencv2\highgui\highgui.hpp>

extern "C" {
	#include "dirent.h"
	#include "common.h"
	#include "wrapper.h"
}

#include "img.h"
#include "wrapper2.h"

#define KR	0.299
#define KG	0.587
#define KB	0.114

	/*struct _DtcImageVals {
		double lum;
		double minlum;
		double maxlum;
	};

	typedef struct _DtcImageVals DtcImageVals;*/

	/**********************************************************************************************//**
	 * @struct	_Image
	 *
	 * @brief	An image, which consists of the frame (a matrix) and the ROI (a rectangle, which in turn
	 * 			has an x, y, width and height values).
	 *
	 * @author	Jon
	 * @date	2017-05-12
	 **************************************************************************************************/

	struct _Image {
		cv::Mat frame;
		cv::Rect roi;
	};
	
	//Not used as of now
	struct _DiffImage {
		cv::Mat frame;
		int x_shift;
		int y_shift;
	};

	typedef struct _Image Image;

	typedef struct _DiffImage DiffImage;

	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	void 			dtcGetROI(Image img, int *xorig, int *yorig, int *width, int *height);

	cv::Point 		dtcGetCM(cv::Mat img);
	cv::Point 		dtcGetGrayCM(cv::Mat img);
	cv::Point 		dtcGetGrayMatCM(cv::Mat mat);

	cv::Rect 		dtcGetImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact);

	cv::Rect 		dtcGetGrayImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact);

	cv::Mat 		dtcGetGrayMat(cv::Mat frame, DtcCapture *capture);					//cvCreateMat

	cv::Mat 		dtcGetGrayMat(cv::Mat *frame, DtcCapture *capture);					//cvCreateMat

	cv::Mat 		dtcReduceMatToROI(cv::Mat src, cv::Rect roi);		//cvCreateMat (cvRelease src)

	cv::Rect 		dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore, int ign = 0);
	
	cv::Rect 		dtcGetFrameROIcCM(cv::Mat frame, cv::Point cm, const int ignore, int ign = 0);

	cv::Rect 		dtcMaxRect(cv::Rect one, cv::Rect two);

	void 			dtcDrawCM(Image image, cv::Point cm);
	
	void			dtcDrawImpact(cv::Mat frame, cv::Point point, cv::Scalar colour);

	void			dtcApplyMaskToFrame(cv::Mat img);

	cv::Mat			dtcApplyMask(cv::Mat img);

	cv::Mat			dtcGetMask(cv::Mat img);

	cv::Rect		dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Rect current_roi, int *x_offset, int *y_offset);

	cv::Rect		dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Point roi_coords, cv::Size roi_size);

	cv::Mat 		dtcRunningAvg(Image imgsrc, Image imgdst, double lR);

	cv::VideoWriter *dtcWriteVideo(const char *file, cv::VideoWriter writer, DtcCapture *capture, cv::Mat img);

	cv::Mat 		dtcLumThreshold_ToZero2(Image src, Image dst, double threshold);

	double 			dtcGetImageLum(Image img);

	DtcImageVals 	dtcGetGrayImageVals(Image img);
	DtcImageVals 	dtcGetImageVals(Image img);

	cv::Mat 		dtcGetHistogramImage(cv::Mat src, float scale, double thr);	//cvCreateImage

	void 			dtcShowPhotometry(cv::Mat mat, int frame);

	cv::Mat			correctBayerFilterImages(cv::Mat mat, int bayerCode);

	static void dtcWriteFrame(cv::VideoWriter writer, cv::Mat img);
	int doublecmp(const void *a, const void *b);
	void printtbuf(double *uc, size_t s);

	bool			isEqual(cv::Mat m1, cv::Mat m2);

	cv::Scalar		dtcGetSimilarity(const cv::Mat m1, const cv::Mat m2);

#endif /* __IMG_H__ */