/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    IMG: Frame images analysis and handling functions							*/
/*                                                                              */
/********************************************************************************/

//#include "stdafx.h"


#include <stdio.h>

#include "img2.hpp"
#include "dtc.h"
#include "wrapper2.hpp"
#include "auxfunc.hpp"
#include <opencv2/imgproc.hpp>  // test OpenCV 4.7.0 
#include <opencv2/imgcodecs/legacy/constants_c.h>  // test OpenCV 4.7.0 

//static void dtcWriteFrame(cv::VideoWriter writer, cv::Mat img);   // test OpenCV 4.7.0 
int doublecmp(const void *a, const void *b);
void printtbuf(double *uc, size_t s);

/**********************************************************************************************//**
 * @fn	cv::Point dtcGetGrayMatCM(cv::Mat mat)
 *
 * @brief	Dtc get gray matrix Centre of Mass.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	mat	The frame.
 *
 * @return	A cv::Point defining the CM.
 **************************************************************************************************/

cv::Point  dtcGetGrayMatCM(cv::Mat mat)
{
	double xcm = 0.0;
	double ycm = 0.0;
	double Y = 0.0;
	uchar *ptr;

	int xorig = 0;
	int yorig = 0;
	int width = mat.cols;
	int height = mat.rows;
	int x, y;
	int step = (int) mat.step;

	double min_ROI_value = 0.00;

/* computes mean of brightness to setup minimum value for taking into account pixels in center of mass calculation */
/*	for (y = yorig; y < height; y++)
	{
		ptr = (mat.data + y * step);
		for (ptr += (x = xorig); x < width; x++)
		{
				Y += *ptr++;
		}
	}
	min_ROI_value = Y / (width*height);*/
	min_ROI_value = dtcGetBackgroundFromHistogram(mat, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0);
	//if (min_ROI_value < opts.ROI_min_px_val)	min_ROI_value = opts.ROI_min_px_val;
	
	Y = 0.0;
	for (y = yorig; y < height; y++)
	{
		ptr = (mat.data + y*step);
		for (ptr += (x = xorig); x < width; x++)
		{
			if ((*ptr) >= min_ROI_value) {
				xcm += x * (*ptr);
				ycm += y * (*ptr);
				Y += *ptr++;
//			} else *ptr++; 
			} else ptr++; // warning C6269
			}
	}

	return cv::Point((int)round(xcm / Y), (int)round(ycm / Y));
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetGrayImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
 *
 * @brief	Get the ROI in a grayscale frame with the Centre of Mass
 *
 * @author	Jon
 * @date	2017-05-12
 * 			
 * @param	img	   	The frame in question.
 * @param	cm	   	The centre of mass of the frame.
 * @param	medsize	The median size of the buffer for the calculation.
 * @param	fact   	The size factor of the ROI.
 * @param	secfact	The security factor of the ROI.
 *
 * @return	A cv::Rect which defines the ROI.
 **************************************************************************************************/

cv::Rect dtcGetGrayImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
{
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf = NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;

	int background = dtcGetBackgroundFromHistogram(img, opts.bg_detection_peak_factor, opts.bg_detection_consecutive_values, 0);
	cv::Mat img_thr = img.clone();
	cv::threshold(img, img_thr, background, 0, CV_THRESH_TOZERO);

	if ((tbuf = (double*)calloc((size_t) (ceil(medsize)), sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROIcCM allocating memory");
		 char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot allocate memory");
		ErrorExit(TRUE, "cannot allocate memory", __func__, msgtext);
	} else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int)floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;

	// Horizontal
	for (y = cm.y, src = (uchar *)img_thr.data + y * img.step + xorig, x = xorig; x < (width - medsize); x++, src += 1) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += 1) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t) (ceil(medsize)), sizeof(double), doublecmp);
		mbuf[x] = tbuf[posmed];
		if (mbuf[x] < mbuf[xmin]) xmin = x;
		if (mbuf[x] > mbuf[xmax]) xmax = x;
	}
	val = mbuf[xmax] - fact*(mbuf[xmax] - mbuf[xmin]);
	for (i = 0; i < (width - medsize) && mbuf[i] < val; i++);
	for (j = (width - 1); j >= 0 && mbuf[j] < val; j--);
	//Width
	hwd = (int)floor((j - i) * secfact);

	// Vertical
	for (x = cm.x, y = yorig, src = (uchar *)img_thr.data + x + y * img.step;
		y < (height - medsize);
		y++, src += img.step) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += img.step) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t)(ceil(medsize)), sizeof(double), doublecmp);
		mbuf[y] = tbuf[posmed];
		if (mbuf[y] < mbuf[ymin]) ymin = y;
		if (mbuf[y] > mbuf[ymax]) ymax = y;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax] - mbuf[ymin]);

	for (i = 0; i < (height - medsize) && mbuf[i] < val; i++);
	for (j = height - 1; j >= 0 && mbuf[j] < val; j--);
	// Height
	hht = (int)floor((j - i) * secfact);

	/* 2018-03-13: Test for bad ROI */
	if (hht > (1.1 * hwd)) {
		hwd = (int)round(1.2*hwd);
		hht = hwd; // hht = 1.2 * hwd
	} else if (hwd > (1.1 * hht)) {
		hht = (int)round(1.2 * hht);
		hwd = hht; // hwd = 1.2 * hht
	}
	/* Test for bad ROI */

	free(tbuf);
	tbuf = NULL;
	free(mbuf);
	mbuf = NULL;
	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
}

cv::Scalar dtcGetSimilarity(const cv::Mat m1, const cv::Mat m2)
{
	const double C1 = 6.5025, C2 = 58.5225;
	/***************************** INITS **********************************/
	int d = CV_32F;
	cv::Mat I1, I2;
	m1.convertTo(I1, d);           // cannot calculate on one byte large values
	m2.convertTo(I2, d);
	cv::Mat I2_2 = I2.mul(I2);        // I2^2
	cv::Mat I1_2 = I1.mul(I1);        // I1^2
	cv::Mat I1_I2 = I1.mul(I2);        // I1 * I2
	/***********************PRELIMINARY COMPUTING ******************************/
	cv::Mat mu1, mu2;   //
	cv::GaussianBlur(I1, mu1, cv::Size(11, 11), 1.5);
	cv::GaussianBlur(I2, mu2, cv::Size(11, 11), 1.5);
	cv::Mat mu1_2 = mu1.mul(mu1);
	cv::Mat mu2_2 = mu2.mul(mu2);
	cv::Mat mu1_mu2 = mu1.mul(mu2);
	cv::Mat sigma1_2, sigma2_2, sigma12;
	cv::GaussianBlur(I1_2, sigma1_2, cv::Size(11, 11), 1.5);
	sigma1_2 -= mu1_2;
	cv::GaussianBlur(I2_2, sigma2_2, cv::Size(11, 11), 1.5);
	sigma2_2 -= mu2_2;
	cv::GaussianBlur(I1_I2, sigma12, cv::Size(11, 11), 1.5);
	sigma12 -= mu1_mu2;
	cv::Mat t1, t2, t3;
	t1 = 2 * mu1_mu2 + C1;
	t2 = 2 * sigma12 + C2;
	t3 = t1.mul(t2);              // t3 = ((2*mu1_mu2 + C1).*(2*sigma12 + C2))
	t1 = mu1_2 + mu2_2 + C1;
	t2 = sigma1_2 + sigma2_2 + C2;
	t1 = t1.mul(t2);               // t1 =((mu1_2 + mu2_2 + C1).*(sigma1_2 + sigma2_2 + C2))
	cv::Mat ssim_map;
	cv::divide(t3, t1, ssim_map);      // ssim_map =  t3./t1;
	cv::Scalar mssim = mean(ssim_map); // mssim = average of ssim map
	return mssim;
}

/*
 * medsize:	median buffer size
 * fact:	size factor
 * secfact:	security factor
 */

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetGrayImageROIcCM2(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
 *
 * @brief	Dtc get gray image ro ic centimetres 2.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	   	The image.
 * @param	cm	   	The centimetres.
 * @param	medsize	The medsize.
 * @param	fact   	The fact.
 * @param	secfact	The secfact.
 *
 * @return	A cv::Rect.
 **************************************************************************************************/

cv::Rect dtcGetGrayImageROIcCM2(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
{
	cv::Mat frame;
	cv::medianBlur(img, frame, 3);
	cv::Mat left, right, top, bottom;
	left = frame.col(0);
	right = frame.col(frame.cols - 1);
	top = frame.row(0);
	bottom = frame.row(frame.rows - 1);
	double avg = (cv::mean(left)[0] + cv::mean(right)[0] + cv::mean(top)[0] + cv::mean(bottom)[0]) / 4;	
	cv::threshold(frame, frame, avg, 0, cv::THRESH_TOZERO);
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf = NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = frame.cols;
	int height = frame.rows;

	if ((tbuf = (double*)calloc((size_t)(ceil(medsize)), sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROIcCM2 allocating memory");
		 char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot allocate memory");
		ErrorExit(TRUE, "cannot allocate memory", __func__, msgtext);
	}
	else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int)floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;

	// Horizontal
	for (y = cm.y, src = (uchar *)frame.data + y * frame.step + xorig, x = xorig;
		x < (width - medsize);
		x++, src += 1) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += 1) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t)(ceil(medsize)), sizeof(double), doublecmp);
		mbuf[x] = tbuf[posmed];
		if (mbuf[x] < mbuf[xmin]) xmin = x;
		if (mbuf[x] > mbuf[xmax]) xmax = x;
	}
	val = mbuf[xmax] - fact*(mbuf[xmax] - mbuf[xmin]);
	for (i = 0; i < (width - medsize) && mbuf[i] < val; i++);
	for (j = (width - 1); j >= 0 && mbuf[j] < val; j--);
	hwd = (int)floor((j - i) * secfact);

	// Vertical
	for (x = cm.x, y = yorig, src = (uchar *)frame.data + x + y * frame.step;
		y < (height - medsize);
		y++, src += frame.step) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += frame.step) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t)(ceil(medsize)), sizeof(double), doublecmp);
		mbuf[y] = tbuf[posmed];
		if (mbuf[y] < mbuf[ymin]) ymin = y;
		if (mbuf[y] > mbuf[ymax]) ymax = y;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax] - mbuf[ymin]);

	for (i = 0; i < (height - medsize) && mbuf[i] < val; i++);
	for (j = height - 1; j >= 0 && mbuf[j] < val; j--);
	hht = (int)floor((j - i) * secfact);

	free(tbuf);
	tbuf = NULL;
	free(mbuf);
	mbuf = NULL;
	frame.~Mat();
	left.~Mat();
	right.~Mat();
	top.~Mat();
	bottom.~Mat();
	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
}


/**********************************************************************************************//**
 * @fn	cv::Mat dtcApplyMask(cv::Mat img)
 *
 * @brief	Applies the mask to the frame for a better cross-correlation.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The frame.
 *
 * @return	A cv::Mat with the masked frame.
 **************************************************************************************************/

cv::Mat dtcApplyMask(cv::Mat img) {
	//int sx, sy; // size of the image
	double min_brightness, max_brightness;
	cv::Mat image, mask, background;

	image = img.clone();
	//sx = image.cols;
	//sy = image.rows;
	cv::minMaxLoc(image, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = (int) round((cv::mean(image.col(0))[0] + cv::mean(image.col(image.cols - 1))[0] + cv::mean(image.row(0))[0] + cv::mean(image.row(image.rows - 1))[0]) / 4.0);
	img -= avgBackground;
	int medianSize = 3;
	int smoothSize = 30;
	//img.convertTo(img, CV_8U);
	cv::medianBlur(image, image, medianSize);
	cv::blur(image, image, cv::Size(smoothSize, smoothSize));
	cv::minMaxLoc(image, &min_brightness, &max_brightness, NULL, NULL);
	mask = cv::Mat(image.size(), CV_8U);
	// Mask will be scaled between 0 and 1
	// Alternative method (maybe better)
	//cv::threshold(img, mask, min_brightness + (max_brightness - min_brightness) / 5.0, 255, CV_THRESH_BINARY);
	mask = image > min_brightness + (max_brightness - min_brightness) / 5.0;
	//cv::blur(mask, mask, cv::Size(smoothSize, smoothSize)); // Not really necessary
	//cv::imshow("Frame mask", mask);
	//cv::waitKey(1);
	image.copyTo(img, mask);
	mask.~Mat();
	background.~Mat();
	return img;
}

cv::Rect dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Point roi_tl_coords, cv::Size roi_size) {

	cv::Mat img, region, corrMat;
	cv::Point maxLoc;

	frame.copyTo(img);
	roi.copyTo(region);

	int corrMatCols = img.cols - region.cols + 1;
	int corrMatRows = img.rows - region.rows + 1;

	corrMat = cv::Mat(corrMatRows, corrMatCols, CV_32F);
	cv::matchTemplate(img, region, corrMat, CV_TM_CCORR_NORMED);
	cv::normalize(corrMat, corrMat, -1, 1, cv::NORM_MINMAX, -1, cv::Mat());
	cv::minMaxLoc(corrMat, NULL, NULL, NULL, &maxLoc, cv::Mat());
	roi_tl_coords += maxLoc;

	img.~Mat();
	region.~Mat();
	corrMat.~Mat();

	return cv::Rect(roi_tl_coords, roi_size);

}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetGrayImageROI(cv::Mat img, float medsize, double fact, double secfact)
 *
 * @brief	Gets the image ROI with the CM as its centre
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	   	The frame in question.
 * @param	cm	   	The centre of mass of the frame.
 * @param	medsize	The median size of the buffer for the calculation.
 * @param	fact   	The size factor of the ROI.
 * @param	secfact	The security factor of the ROI.
 *
 * @return	A cv::Rect which defines the ROI.
 **************************************************************************************************/

cv::Rect dtcGetGrayImageROI(cv::Mat img, float medsize, double fact, double secfact)
{
	int sx, sy; // size of the image
	cv::Point cm;
	double min_brightness, max_brightness;
	cv::Mat mask;
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf = NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	sx = img.cols;
	sy = img.rows;
	cv::minMaxLoc(img, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = (int)round((cv::mean(img.col(0))[0] + cv::mean(img.col(img.cols - 1))[0] + cv::mean(img.row(0))[0] + cv::mean(img.row(img.rows - 1))[0]) / 4.0);
	img -= avgBackground;
	int medianSize = 15;
	int smoothSize = 30;
	img.convertTo(img, CV_8U);
	cv::medianBlur(img, img, medianSize);
	cv::blur(img, img, cv::Size(smoothSize, smoothSize));
	cv::minMaxLoc(img, &min_brightness, &max_brightness, NULL, NULL);
	mask = cv::Mat(img.size(), CV_8U);
	mask = img > min_brightness + (max_brightness - min_brightness) / 3.0;
	cv::blur(mask, mask, cv::Size(smoothSize, smoothSize));
	cv::circle(img, cm, 10, cv::Scalar(0, 0, 0));
	img.convertTo(img, CV_32F);
	img.copyTo(img, mask);
	cv::minMaxLoc(img, NULL, NULL, NULL, &cm);
	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;

	if ((tbuf = (double*)calloc((size_t)(ceil(medsize)), sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROI allocating memory");
		 char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot allocate memory");
		ErrorExit(TRUE, "cannot allocate memory", __func__, msgtext);
	}
	else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int)floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;

	// Horizontal
	for (y = cm.y, src = (uchar *)img.data + y * img.step + xorig, x = xorig;
		x < (width - medsize);
		x++, src += 1) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += 1) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t)(ceil(medsize)), sizeof(double), doublecmp);
		mbuf[x] = tbuf[posmed];
		if (mbuf[x] < mbuf[xmin]) xmin = x;
		if (mbuf[x] > mbuf[xmax]) xmax = x;
	}
	val = mbuf[xmax] - fact*(mbuf[xmax] - mbuf[xmin]);
	for (i = 0; i < (width - medsize) && mbuf[i] < val; i++);
	for (j = (width - 1); j >= 0 && mbuf[j] < val; j--);
	hwd = (int)floor((j - i) * secfact);

	// Vertical
	for (x = cm.x, y = yorig, src = (uchar *)img.data + x + y * img.step;
		y < (height - medsize);
		y++, src += img.step) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += img.step) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, (size_t)(ceil(medsize)), sizeof(double), doublecmp);
		mbuf[y] = tbuf[posmed];
		if (mbuf[y] < mbuf[ymin]) ymin = y;
		if (mbuf[y] > mbuf[ymax]) ymax = y;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax] - mbuf[ymin]);

	for (i = 0; i < (height - medsize) && mbuf[i] < val; i++);
	for (j = height - 1; j >= 0 && mbuf[j] < val; j--);
	hht = (int)floor((j - i) * secfact);

	free(tbuf);
	tbuf = NULL;
	free(mbuf);
	mbuf = NULL;
	mask.~Mat();
	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetGrayMatROIcCM(cv::Mat *img, cv::Point cm, int medsize, double fact, double secfact)
 *
 * @brief	Gets the image ROI with the CM as its centre
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	   	Pointer to the frame in question.
 * @param	cm	   	The centre of mass of the frame.
 * @param	medsize	The median size of the buffer for the calculation.
 * @param	fact   	The size factor of the ROI.
 * @param	secfact	The security factor of the ROI.
 *
 * @return	A cv::Rect which defines the ROI.
 **************************************************************************************************/

cv::Rect dtcGetGrayMatROIcCM(cv::Mat *img, cv::Point cm, int medsize, double fact, double secfact)
{
	float *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf = NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = img->cols;
	int height = img->rows;
	int step = (int)img->step/sizeof (float);

	if ((tbuf = (double *)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double *)calloc(MAX(width, height), sizeof(double))) == NULL) {
		//perror("EROOR in dtcGetGrayMatROIcCM allocating memory");
		DBOUT("ERROR in dtcGetGrayMatROIcCM allocating memory\n");
		 char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext, MAX_STRING, "cannot allocate memory");
		ErrorExit(TRUE, "cannot allocate memory", __func__, msgtext);
	} else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int)floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;
	// Horizontal
	y = cm.y;
	src = (float *) img->datastart + y*step + xorig;
	for (x = xorig; x < width - medsize; x++)
	{
		for (i = 0, tsrc = src; i < medsize; i++, tsrc++)
		{
			tbuf[i] = *tsrc;
		}

		qsort(tbuf, medsize, sizeof(double), doublecmp);
		mbuf[x] = tbuf[posmed];
		if (mbuf[x] < mbuf[xmin]) xmin = x;
		if (mbuf[x] > mbuf[xmax]) xmax = x;
		src++;
	}

	val = mbuf[xmax] - fact*(mbuf[xmax] - mbuf[xmin]);
	for (i = 0; i < width - medsize && mbuf[i] < val; i++)
		;
	for (j = width - medsize - 1; j >= 0 && mbuf[j] < val; j--)
		;
	hwd = (int)floor((j - i) * secfact);

	// Vertical
	x = cm.x;
	src = (float *) img->datastart + x + yorig*step;
	for (y = yorig; y < height - medsize; y++)
	{
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += step)
		{
			tbuf[i] = *tsrc;
		}

		qsort(tbuf, medsize, sizeof(double), doublecmp);
		mbuf[y] = tbuf[posmed];
		if (mbuf[y] < mbuf[ymin]) ymin = y;
		if (mbuf[y] > mbuf[ymax]) ymax = y;
		src += step;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax] - mbuf[ymin]);

	for (i = 0; i < height - medsize && mbuf[i] < val; i++)
		;
	for (j = height - medsize - 1; j >= 0 && mbuf[j] < val; j--)
		;
	hht = (int)floor((j - i) * secfact);

	free(tbuf);
	tbuf = NULL;
	free(mbuf);
	mbuf = NULL;
	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
}


/**********************************************************************************************//**
 * @fn	cv::Rect dtcMaxRect(cv::Rect one, cv::Rect two)
 *
 * @brief	Get the maximum rectangle.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	one	First rectangle.
 * @param	two	Second rectangle.
 *
 * @return	A cv::Rect.
 **************************************************************************************************/

cv::Rect dtcMaxRect(cv::Rect one, cv::Rect two)
{
	int x, y, w, h;

	x = one.width  > two.width ? one.x : two.x;
	y = one.height > two.height ? one.y : two.y;
	w = one.width  > two.width ? one.width : two.width;
	h = one.height > two.height ? one.height : two.height;

	return cv::Rect(x, y, w, h);
}

cv::Mat dtcGetGrayMat(cv::Mat *frame, DtcCapture *capture)
{

	if (!frame->data) return cv::Mat();

	cv::Mat gray, frame_to_gray;

	gray = cv::Mat(frame->size(), CV_32F);
	frame_to_gray = frame->clone();

	if (frame->channels() > 1) {
		if (capture->type == CAPTURE_SER) {
			// All colours except MONO and BGR
			if (capture->u.sercapture->header.ColorID != SER_BGR) {
				cv::cvtColor(frame_to_gray, gray, CV_RGB2GRAY);
			} else {
				cv::cvtColor(frame_to_gray, gray, CV_BGR2GRAY);
			}
		} else {
			if (opts.bayer > 0) {
				std::vector<cv::Mat> frame_channels;
				cv::split(*frame, frame_channels);
				if (isEqual(frame_channels[0], frame_channels[1]) && isEqual(frame_channels[1], frame_channels[2])) {
					frame->convertTo(*frame, CV_8UC1);
					*frame = (frame_channels[0] + frame_channels[1] + frame_channels[2]) / 3;
					cv::cvtColor(*frame, *frame, opts.bayer);
					frame->copyTo(frame_to_gray);
					cv::cvtColor(frame_to_gray, gray, CV_RGB2GRAY);
				} else {
					cv::cvtColor(frame_to_gray, gray, CV_BGR2GRAY);
				}
			} else {
				cv::cvtColor(frame_to_gray, gray, CV_BGR2GRAY);
			}
		}
	} else if (frame->channels() == 1) {
		if (opts.bayer > 0) {
			cv::cvtColor(*frame, *frame, opts.bayer);
			cv::cvtColor(*frame, gray, CV_RGB2GRAY);
		} else {
			frame_to_gray.copyTo(gray);
		}
	} else {
		//gray.release();
		//gray = NULL;
		//exit(EXIT_FAILURE);
		frame_to_gray.copyTo(gray);		 //return original if no correct channel
//		return cv::Mat();
	}
	frame_to_gray.~Mat();
	return gray;
}


/**********************************************************************************************//**
 * @fn	cv::Mat dtcReduceMatToROI(cv::Mat src, cv::Rect roi)
 *
 * @brief	Reduces frame to roi.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	src	Frame to be reduces.
 * @param	roi	The ROI defined as a rectangle.
 *
 * @return	A cv::Mat with a cut of the frame.
 **************************************************************************************************/

cv::Mat dtcReduceMatToROI(cv::Mat src, cv::Rect roi)
{
	return src(roi);
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore, int ign)
 *
 * @brief	Dtc get file ro ic centimetres.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pcapture	If non-null, the pcapture.
 * @param 		  	ignore  	The ignore.
 * @param 		  	ign			The ign.
 *
 * @return	A cv::Rect.
 **************************************************************************************************/

cv::Rect dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore) {
	int error = 0;
	cv::Rect win, roi = cv::Rect(0, 0, 0, 0);
	cv::Mat gray;
	cv::Mat frame;

	unsigned long nframe;
	for (nframe = 1; !opts.nframesROI || nframe <= opts.nframesROI; nframe++) {
		error = 0;
		frame = dtcQueryFrame2(pcapture, ignore, &error);
/*if (frame.data) {
	cv::Mat frame_img;
	frame.convertTo(frame_img, CV_8U);
	cv::namedWindow("Debug1");
	cv::imshow("Debug1", frame_img);
	cv::waitKey(0);
	cv::destroyWindow("Debug1");
}*/
	if ((frame.empty()) ||(frame.dims == 0)) return roi;
		if (error == 0) {
			cv::Point cm;
			gray = dtcGetGrayMat(&frame, pcapture);
			gray = dtcApplyMask(gray.clone());
//AS3
			cm = dtcGetGrayMatCM(gray); // gets Center of Mass
			//if (cm.x < 0 || cm.y < 0) throw std::logic_error("ROI cannot be obtained, negative or zero centre of brightness");
			if (cm.x <= 0 || cm.y <= 0) return cv::Rect(0, 0, 0, 0);
			win = dtcGetGrayImageROIcCM(gray, cm, (float)opts.medSize, opts.facSize, opts.secSize); // gets ROI
			roi = dtcMaxRect(win, roi);
			gray.~Mat();
			frame.~Mat();
			if (opts.debug) { 
				DBOUT("!Debug info: dtcGetFileROIcCM: frame " << nframe << "\n")
			}
		}
	}

	return roi;
}

/**********************************************************************************************//**
 * @fn	void dtcDrawCM(Image image, cv::Point cm)
 *
 * @brief	Dtc draw the centre of Brightness.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	image	The image.
 * @param	cm   	The centre of brightness.
 **************************************************************************************************/

void dtcDrawCM(Image image, cv::Point cm)
{
	cv::line(image.frame, cm, cm, CV_RGB(255, 0, 0), 1, 8, 0);
	cv::line(image.frame, cv::Point(cm.x - 25, cm.y), cv::Point(cm.x - 5, cm.y), CV_RGB(255, 0, 0), 1, 8, 0);
	cv::line(image.frame, cv::Point(cm.x + 5, cm.y), cv::Point(cm.x + 25, cm.y), CV_RGB(255, 0, 0), 1, 8, 0);
	cv::line(image.frame, cv::Point(cm.x, cm.y - 25), cv::Point(cm.x, cm.y - 5), CV_RGB(255, 0, 0), 1, 8, 0);
	cv::line(image.frame, cv::Point(cm.x, cm.y + 5), cv::Point(cm.x, cm.y + 25), CV_RGB(255, 0, 0), 1, 8, 0);
	cv::circle(image.frame, cm, 1, cv::Scalar(255, 255, 255));
	cv::rectangle(image.frame, image.roi, CV_RGB(0, 255, 0), 1, 8, 0);
}

/**********************************************************************************************//**
 * @fn	void dtcDrawImpact(cv::Mat frame, cv::Point point)
 *
 * @brief	Draw the point where the impact ocurrs as a "crosshair".
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	frame	The frame.
 * @param	point	The point of impact.
 **************************************************************************************************/

void dtcDrawImpact(cv::Mat frame, cv::Point point, cv::Scalar colour, int lmin, int lmax) {
	cv::line(frame, cv::Point(point.x + lmin, point.y), cv::Point(point.x + lmax, point.y), colour, 2, 8, 0);
	cv::line(frame, cv::Point(point.x - lmax, point.y), cv::Point(point.x - lmin, point.y), colour, 2, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y - lmax), cv::Point(point.x, point.y - lmin), colour, 2, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y + lmin), cv::Point(point.x, point.y + lmax), colour, 2, 8, 0);
}

/***************************************************************************************************
 * @fn	cv::Mat dtcGetHistogramImage(cv::Mat src, float scale, double thr)
 *
 * @brief	Get histogram of the frame
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	src  	Source frame for the histogram.
 * @param	scale	The scale for the histogram (0-1).
 * @param	thr  	The threshold value.
 *
 * @return	A cv::Mat with the histogram to be shown.
 **************************************************************************************************/

cv::Mat dtcGetHistogramImage(cv::Mat src, float scale, double thr)
{
	cv::Mat pHis, pHisImg;
	const int hsize = 256;
	const int vsize = 300;
	int phsize[] = { hsize };
	float range[] = { 0 , (float)hsize };
	const float *ranges[] = { range };
	int i;
	double bval;
	double max_val = 0.0;
	int channels[] = { 0 , 1 };

	cv::calcHist(cv::makePtr<cv::Mat>(src), 1, 0, cv::Mat(), pHis, 1, phsize, ranges, true, false);
	pHisImg = cv::Mat::zeros( (int)(ceil(vsize * scale)), (int)(ceil(hsize * scale)), CV_8UC1);
	if (thr) {
		pHis.at<float>(0) = 0;
	}
	cv::minMaxLoc(pHis, 0, &max_val, 0, 0);
	pHisImg = cv::Scalar(255, 255, 255, 0);
	if (max_val > 0) {
		for (i = 0; i < hsize; i++){
			bval = pHis.at<float>(i);
			cv::rectangle(pHisImg, cv::Point(cvRound(i*scale), cvRound((vsize - bval*hsize / max_val)*scale)), 
				cv::Point(cvRound((i + 1)*scale - 1), cvRound(vsize*scale)), CV_RGB(0, 0, 0), CV_FILLED, 8, 0);
		}
	}
	pHis.~Mat();
	return pHisImg;
}

/**********************************************************************************************
 * @fn	cv::Mat dtcGetBackgroundFromHistogram(cv::Mat src, const double background_threshold_max_factor, const int number_below_threshold, const double thr)
 *
 * @brief	Get background from histogram of the frame
 *
 * @author	Marc
 * @date	2023-04-03
 *
 * @param	src  							Source frame for the histogram.
 * @param	background_threshold_max_factor	The % of max number of pixels in histogram of the same brightness to select background
 * @param	number_below_threshold			The number of consecutive values in histogrambelow the max factor to select background
 * @param	thr  							The threshold value.
 *
 * @return	A cv::Mat with the histogram to be shown.
 **************************************************************************************************/

int dtcGetBackgroundFromHistogram(cv::Mat src, const double background_threshold_max_factor, const int number_below_threshold, const double thr)
{
	int background					= 0;
	cv::Mat pHis;
	const int hsize					= 256;
	int phsize[]					= { hsize };
	float range[]					= { 0 , (float)hsize };
	const float* ranges[]			= { range };
	int counter_below_threshold		= 0;
	double min_val					= 0;
	double max_val					= 0;
	double bg_val					= 0;
	double nb_val					= 0;
	cv::Point min_loc				= { 0,0 };
	cv::Point max_loc				= { 0,0 };

	cv::calcHist(cv::makePtr<cv::Mat>(src), 1, { 0 }, cv::Mat(), pHis, 1, phsize, ranges, true, false);
	if (thr) {
		pHis.at<float>(0) = 0;
	}
	cv::minMaxLoc(pHis, &min_val, &max_val, &min_loc, &max_loc);
	bg_val = MAX(max_val * background_threshold_max_factor, min_val + 1);
//	if (bg_val > 0) {
		for (int i = 0; i < hsize; i++) {
			nb_val = pHis.at<float>(i);
			if (nb_val <= bg_val) {
				counter_below_threshold++;
				if (counter_below_threshold >= number_below_threshold) {
					background = i;
					break;
				}
			} else counter_below_threshold = 0;
		}
	//}
	pHis.~Mat();
	if (background > 0)	return background + 1;
	else return 0;
}


/**********************************************************************************************//**
 * @fn	void dtcWriteFrame(cv::VideoWriter writer, cv::Mat img)
 *
 * @brief	Write frame.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	writer	The video writer.
 * @param	img   	The source frame.
 **************************************************************************************************/

/*   // test OpenCV 4.7.0  deactivated 2023.01.10
void dtcWriteFrame(cv::VideoWriter writer, cv::Mat img)
{
	cv::Mat tmp;
	char *src, *dst;
	if (img.cols * img.channels() == img.step)
		writer.write(img);
	else
	{
		tmp = cv::Mat(img.size(), img.depth(), img.channels());
		tmp.datastart = img.datastart;
		tmp.datalimit = img.datalimit;

		src = (char *) img.data;
		dst = (char *) tmp.data;
		for (int i = 0; i < tmp.rows; i++) {
			src = (char *) img.data + i * img.step;
			for (int j = 0; j < (img.cols * img.channels()); j++) {
				*dst++ = *src++;
			}
		}
		writer.write(tmp);
		tmp.release();
		tmp = NULL;
		
	}
}
*/
/**********************************************************************************************//**
 * @fn	cv::VideoWriter *dtcWriteVideo(const char *file, cv::VideoWriter writer, DtcCapture *capture, cv::Mat img)
 *
 * @brief	Write video.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param 		  	file   	The file where the video will be saved.
 * @param 		  	writer 	The video writer.
 * @param [in,out]	capture	If non-null, the capture.
 * @param 		  	img	   	The frame to be written.
 *
 * @return	Null if it fails, else a pointer to a cv::VideoWriter.
 **************************************************************************************************/
 /* deactivated 2023.01.10 test OpenCV 4.7.0 
cv::VideoWriter *dtcWriteVideo(const char *file, cv::VideoWriter writer, DtcCapture *capture, cv::Mat img)
{
	cv::Mat color;
	double fps;
	cv::Size size;

	if (!img.data) return nullptr;

	if (!writer.isOpened()) {
		fps = dtcGetCaptureProperty(capture, cv::CAP_PROP_FPS);  // test OpenCV 4.7.0 
		size = cv::Size(img.cols, img.rows);
		writer = cv::VideoWriter(file,
		#if defined(_WIN32)
			cv::VideoWriter::fourcc('D', 'I', 'B', ' '),  // test OpenCV 4.7.0 
		#else
			cv::VideoWriter::fourcc('M', 'J', 'P', 'G'),  // test OpenCV 4.7.0 
		#endif
			fps, size, CV_LOAD_IMAGE_COLOR);
	} else {
		if (img.channels() < 3) {
			color = cv::Mat(img.size(), CV_8UC3);
			cv::cvtColor(img, color, CV_GRAY2BGR);
		} else {
			color = img;
		}

		dtcWriteFrame(writer, color);
		if (img.channels() < 3)
			color.release();
		color = NULL;
	}

	return &writer;
}
*/
/**********************************************************************************************//**
 * @fn	void dtcShowPhotometry(cv::Mat pmat, int nframe)
 *
 * @brief	Show the photometry.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	pmat  	The frame.
 * @param	nframe	The frame number.
 **************************************************************************************************/
/*
void dtcShowPhotometry(cv::Mat pmat, int nframe)
{
	cv::Scalar lum;
	cv::Point minPoint;
	cv::Point maxPoint;
	double minLum;
	double maxLum;

	lum = cv::sum(pmat);
	cv::minMaxLoc(pmat, &minLum, &maxLum, &minPoint, &maxPoint);
	printf("%03d   %10.6f %d ( %3d , %3d )   %3d ( %3d , %3d )\n",
		nframe, lum.val[0] / (pmat.cols * pmat.rows),
		(int)minLum, minPoint.x, minPoint.y, (int)maxLum, maxPoint.x, maxPoint.y);
}
*/
/**********************************************************************************************//**
 * @fn	int doublecmp(const void *a, const void *b)
 *
 * @brief	Doublecmps.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	a	A void to process.
 * @param	b	A void to process.
 *
 * @return	An int.
 **************************************************************************************************/

int doublecmp(const void *a, const void *b)
{
	if (*((double *)a) < *((double *)b)) return -1;
	else if (*((double *)a) > *((double *)b)) return 1;
	else return 0;
}

/**********************************************************************************************//**
 * @fn	void printtbuf(double *uc, size_t s)
 *
 * @brief	Printtbufs.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	uc	If non-null, the uc.
 * @param 		  	s 	A size_t to process.
 **************************************************************************************************/

void printtbuf(double *uc, size_t s)
{
	printf("[ ");
	for (size_t i = 0; i < s; i++) {
		printf("%6.3f ", uc[i]);
	}
	printf("]\n");
}
/*
cv::Mat	correctBayerFilterImages(cv::Mat frame, int bayerCode) {
	if (!frame.data) return cv::Mat();
	if (frame.channels() > 1) {
		std::vector<cv::Mat> frame_channels;
		cv::split(frame, frame_channels);
		frame.convertTo(frame, CV_8UC1);
		frame = (frame_channels[0] + frame_channels[1] + frame_channels[2]) / 3;
	}
	cv::cvtColor(frame, frame, bayerCode);
	return frame;
}
*/
bool isEqual(cv::Mat m1, cv::Mat m2) {
	if (m1.empty() && m2.empty()) return true;
	if (m1.cols != m2.cols || m1.rows != m2.rows || m1.dims != m2.dims) return false;
	if (m1.type() != m2.type()) return false;
	return cv::countNonZero(m1 != m2) == 0;
}

/*
void dtcApplyDifferentialPhotometry(cv::Mat& original, cv::Mat& reference, cv::Mat& difference,
	cv::Mat& tracking) {

}*/