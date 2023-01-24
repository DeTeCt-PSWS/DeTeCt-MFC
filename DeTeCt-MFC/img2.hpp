#pragma once
#ifndef __IMG2_H__
#define __IMG2_H__

// test OpenCV 4.7.0 #include <opencv/cv.h>
//#include <opencv/cv.h> // test OpenCV 4.7.0 
#include <opencv2/highgui.hpp>
//#include <opencv2/videoio/videoio_c.h>  // test OpenCV 4.7.0 
#include <opencv2/videoio.hpp>  // test OpenCV 4.7.0 

//extern "C" {
	#include "dirent.h"
	#include "common.h"
	#include "wrapper.h"
//}

#include "wrapper2.hpp"

#define KR	0.299
#define KG	0.587
#define KB	0.114

	struct _DtcImageVals {
		double lum;
		double minlum;
		double maxlum;
	};

	typedef struct _DtcImageVals DtcImageVals;

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
		int x_shift = 0;
		int y_shift = 0;
	};

	typedef struct _Image Image;

	typedef struct _DiffImage DiffImage;

	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	cv::Point 		dtcGetGrayMatCM(cv::Mat mat);


	cv::Rect 		dtcGetGrayImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact);


	cv::Mat 		dtcGetGrayMat(cv::Mat *frame, DtcCapture *capture);					//cvCreateMat

	cv::Mat 		dtcReduceMatToROI(cv::Mat src, cv::Rect roi);		//cvCreateMat (cvRelease src)

	cv::Rect 		dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore);
	
	cv::Rect 		dtcMaxRect(cv::Rect one, cv::Rect two);

	void 			dtcDrawCM(Image image, cv::Point cm);
	
	void			dtcDrawImpact(cv::Mat frame, cv::Point point, cv::Scalar colour, int lmin, int lmax);

	cv::Mat			dtcApplyMask(cv::Mat img);

	cv::Rect		dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Point roi_coords, cv::Size roi_size);

	//cv::VideoWriter *dtcWriteVideo(const char *file, cv::VideoWriter writer, DtcCapture *capture, cv::Mat img);   // test OpenCV 4.7.0 

	cv::Mat 		dtcGetHistogramImage(cv::Mat src, float scale, double thr);	//cvCreateImage

//	static void		dtcWriteFrame(cv::VideoWriter writer, cv::Mat img);   // test OpenCV 4.7.0 
	int				doublecmp(const void *a, const void *b);
	void			printtbuf(double *uc, size_t s);

	bool			isEqual(cv::Mat m1, cv::Mat m2);

	cv::Scalar		dtcGetSimilarity(const cv::Mat m1, const cv::Mat m2);

#endif /* __IMG_H__ */