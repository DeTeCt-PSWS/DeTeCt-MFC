/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    IMG: Frame images analysis and handling functions							*/
/*                                                                              */
/********************************************************************************/


#include <stdio.h>

#include "img2.h"
#include "dtc.h"
#include "wrapper2.h"
#include "auxfunc.h"

static void dtcWriteFrame(cv::VideoWriter writer, cv::Mat img);
int doublecmp(const void *a, const void *b);
void printtbuf(double *uc, size_t s);

/**********************************************************************************************//**
 * @fn	void dtcGetROI(Image img, int *xorig, int *yorig, int *width, int *height)
 *
 * @brief	Gets the ROI of the frame
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param 		  	img   	The image.
 * @param [in,out]	xorig 	If non-null, the ROI's x origin point.
 * @param [in,out]	yorig 	If non-null, the ROI's y origin point.
 * @param [in,out]	width 	If non-null, the ROI's width.
 * @param [in,out]	height	If non-null, the ROI's height.
 **************************************************************************************************/

void dtcGetROI(Image img, int *xorig, int *yorig, int *width, int *height)
{
	if (img.roi.area()) {
		*xorig = img.roi.x;
		*yorig = img.roi.y;
		*width = img.roi.width;
		*height = img.roi.height;
	}
	else {
		*xorig = 0;
		*yorig = 0;
		*width = img.frame.cols;
		*height = img.frame.rows;
	}
}

/**********************************************************************************************//**
 * @fn	cv::Point dtcGetCM(cv::Mat img)
 *
 * @brief	Gets the centre of brightness (here defined as the centre of mass, CM) of the frame
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The frame
 *
 * @return	A cv::Point defining the CM.
 **************************************************************************************************/

cv::Point dtcGetCM(cv::Mat img)
{
	double xcm = 0, ycm = 0, Y = 0, l;
	uchar r, g, b;

	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;
	int x, y;

	for (y = yorig; y < height; y++) {
		uchar *ptr = (uchar*)(img.data + y * img.step);
		for (x = xorig; x < width; x++) {
			r = *ptr++;
			g = *ptr++;
			b = *ptr++;
			if (img.channels() == 4) ptr++;
			Y += (l = KR * r + KG * g + KB * b);
			xcm += x * l;
			ycm += y * l;
		}
	}

	return cv::Point((int)floor(xcm / Y), (int)floor(ycm / Y));
}

/**********************************************************************************************//**
 * @fn	cv::Point dtcGetGrayCM(cv::Mat img)
 *
 * @brief	Dtc gets the Center of Mass for a grayscale image.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The image.
 *
 * @return	A cv::Point which defines the CM.
 **************************************************************************************************/

cv::Point dtcGetGrayCM(cv::Mat img)
{
	double xcm = 0, ycm = 0, Y = 0;

	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;
	int x, y;

	for (y = yorig; y < height; y++) {
		uchar *ptr = (uchar*)(img.data + y * img.step);
		for (x = xorig; x < width; x++) {
			xcm += x * (*ptr);
			ycm += y * (*ptr);
			Y += *ptr++;
		}
	}

	return cvPoint((int)floor(xcm / Y), (int)floor(ycm / Y));
}

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
	double   Y = 0.0;
	uchar *ptr;

	int xorig = 0;
	int yorig = 0;
	int width = mat.cols;
	int height = mat.rows;
	int x, y;
	int step = mat.step;

	for (y = yorig; y < height; y++)
	{
		ptr = (mat.data + y*step);
		for (ptr += (x = xorig); x < width; x++)
		{
			xcm += x * (*ptr);
			ycm += y * (*ptr);
			Y += *ptr++;
		}
	}

	return cv::Point((int)floor(xcm / Y), (int)floor(ycm / Y));
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
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

cv::Rect dtcGetImageROIcCM(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
{
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf = NULL;
	double *mbuf = NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	const double kr = 0.299;
	const double kg = 0.587;
	const double kb = 0.114;

	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;

	if ((tbuf = (double*)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("dtcGetImageROI");
		throw std::bad_alloc();
		//abort();
	}
	assert(tbuf != NULL);
	assert(mbuf != NULL);

	posmed = (int)floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;

	// Horizontal line
	for (y = cm.y,
		src = (uchar *) img.data + y * img.step
		+ xorig * img.channels(),
		x = xorig;
		x < width - medsize;
		x++, src += img.channels())
	{
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += img.channels()) {
			tbuf[i] = kr * tsrc[0] + kg * tsrc[1] + kb * tsrc[2];
		}

		qsort(tbuf, medsize, sizeof(double), doublecmp);
		mbuf[x] = tbuf[posmed];
		if (mbuf[x] < mbuf[xmin]) xmin = x;
		if (mbuf[x] > mbuf[xmax]) xmax = x;
	}

	val = mbuf[xmax] - fact*(mbuf[xmax] - mbuf[xmin]);
	for (i = 0; mbuf[i] < val; i++)
		;
	for (j = width - 1; mbuf[j] < val; j--)
		;
	hwd = (int)floor((j - i) * secfact);

	// Vertical line
	for (x = cm.x, y = yorig,
		src = (uchar *)img.data + x * img.channels()
		+ y * img.step;
		y < height - medsize;
		y++, src += img.step)
	{
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += img.step) {
			tbuf[i] = kr * tsrc[0] + kg * tsrc[1] + kb * tsrc[2];
		}

		qsort(tbuf, medsize, sizeof(double), doublecmp);
		mbuf[y] = tbuf[posmed];
		if (mbuf[y] < mbuf[ymin]) ymin = y;
		if (mbuf[y] > mbuf[ymax]) ymax = y;

	}
	val = mbuf[ymax] - fact*(mbuf[ymax] - mbuf[ymin]);
	for (i = 0; mbuf[i] < val; i++)
		;
	for (j = height - 1; mbuf[j] < val; j--)
		;
	hht = (int)floor((j - i) * secfact);

	free(tbuf);
	tbuf = NULL;
	free(mbuf);
	mbuf = NULL;

	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
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
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = img.cols;
	int height = img.rows;

	if ((tbuf = (double*)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROI allocating memory");
		exit(EXIT_FAILURE);
	} else {
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
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
	for (x = cm.x, y = yorig, src = (uchar *)img.data + x + y * img.step;
		y < (height - medsize);
		y++, src += img.step) {
		for (i = 0, tsrc = src; i < medsize; i++, tsrc += img.step) {
			tbuf[i] = tsrc[0];
		}
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
		hwd *= 1.2;
		hht = hwd; // hht = 1.2 * hwd
	} else if (hwd > (1.1 * hht)) {
		hht *= 1.2;
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
	int avg = (cv::mean(left)[0] + cv::mean(right)[0] + cv::mean(top)[0] + cv::mean(bottom)[0]) / 4;	
	cv::threshold(frame, frame, avg, 0, cv::THRESH_TOZERO);
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = frame.cols;
	int height = frame.rows;

	if ((tbuf = (double*)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROI allocating memory");
		exit(EXIT_FAILURE);
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
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
	frame.release();
	frame = NULL;
	left.release();
	left = NULL;
	right.release();
	right = NULL;
	top.release();
	top = NULL;
	bottom.release();
	bottom = NULL;
	return cv::Rect(cm.x - hwd / 2, cm.y - hht / 2, hwd, hht);
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetGrayImageROIcCM3(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
 *
 * @brief	Dtc get gray image ro ic centimetres 3.
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

cv::Rect dtcGetGrayImageROIcCM3(cv::Mat img, cv::Point cm, float medsize, double fact, double secfact)
{
	cv::Mat frame;
	cv::medianBlur(img, frame, 3);
	cv::Mat left, right, top, bottom;
	left = frame.col(0);
	right = frame.col(frame.cols - 1);
	top = frame.row(0);
	bottom = frame.row(frame.rows - 1);
	DBOUT(frame.channels() << "\n");
	int avg = (cv::mean(left)[0] + cv::mean(right)[0] + cv::mean(top)[0] + cv::mean(bottom)[0]) / 4;
	cv::threshold(frame, frame, avg, 0, cv::THRESH_TOZERO);
	double min_brightness, max_brightness;
	cv::minMaxLoc(frame, &min_brightness, &max_brightness, NULL, NULL);
	int H = max_brightness - ((max_brightness - min_brightness) * 0.9);
	cv::Point window_upper_left_corner(0, 0);
	cv::Point window_lower_right_corner(0, 0);
	for (int y = 0; y < frame.rows; y++) {
	if ((int)frame.at<uchar>(y, cm.x) > H) {
	window_upper_left_corner.y = y;
	break;
	}
	}
	for (int x = 0; x < frame.cols; x++) {
	if ((int)frame.at<uchar>(cm.y, x) > H) {
	window_upper_left_corner.x = x;
	break;
	}
	}
	frame.release();
	frame = NULL;
	left.release();
	left = NULL;
	right.release();
	right = NULL;
	top.release();
	top = NULL;
	bottom.release();
	bottom = NULL;
	window_lower_right_corner.x = window_upper_left_corner.x + ((cm.x - window_upper_left_corner.x) * 2);
	window_lower_right_corner.y = window_upper_left_corner.y + ((cm.y - window_upper_left_corner.y) * 2);
	return cv::Rect(window_upper_left_corner, window_lower_right_corner);
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
	int sx, sy; // size of the image
	double min_brightness, max_brightness;
	cv::Mat image, mask, background;

	image = img.clone();
	sx = image.cols;
	sy = image.rows;
	cv::minMaxLoc(image, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = (cv::mean(image.col(0))[0] + cv::mean(image.col(image.cols - 1))[0] + cv::mean(image.row(0))[0] +
		cv::mean(image.row(image.rows - 1))[0]) / 4;
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
	cv::waitKey(1);
	image.copyTo(img, mask);
	mask.release();
	mask = NULL;
	background.release();
	background = NULL;
	return img;
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

void dtcApplyMaskToFrame(cv::Mat img) {
	int sx, sy; // size of the image
	double min_brightness, max_brightness;
	cv::Mat image, mask, background;

	image = img.clone();
	sx = image.cols;
	sy = image.rows;
	cv::minMaxLoc(image, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = (cv::mean(image.col(0))[0] + cv::mean(image.col(image.cols - 1))[0] + cv::mean(image.row(0))[0] +
		cv::mean(image.row(image.rows - 1))[0]) / 4;
	img -= avgBackground;
	int medianSize = 3;
	int smoothSize = 30;
	//img.convertTo(img, CV_8U);
	cv::medianBlur(image, image, medianSize);
	cv::blur(image, image, cv::Size(smoothSize, smoothSize));
	cv::minMaxLoc(image, &min_brightness, &max_brightness, NULL, NULL);
	// Mask will be scaled between 0 and 1
	// Alternative method (maybe better)
	cv::threshold(img, img, min_brightness + (max_brightness - min_brightness) / 20.0, 255, CV_THRESH_TOZERO);
	mask.release();
	mask = NULL;
	background.release();
	background = NULL;
}

/**********************************************************************************************//**
* @fn	cv::Mat dtcGetMask(cv::Mat img)
*
* @brief	Gets the mask to the frame for a better cross-correlation.
*
* @author	Jon
* @date		2017-05-12
*
* @param	img	The frame.
*
* @return	A cv::Mat with the mask.
**************************************************************************************************/

cv::Mat dtcGetMask(cv::Mat img) {
	int sx, sy; // size of the image
	double min_brightness, max_brightness;
	cv::Mat mask, background;

	sx = img.cols;
	sy = img.rows;
	cv::minMaxLoc(img, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = (cv::mean(img.col(0))[0] + cv::mean(img.col(img.cols - 1))[0] + cv::mean(img.row(0))[0] +
		cv::mean(img.row(img.rows - 1))[0]) / 4;
	img -= avgBackground;
	int medianSize = 3;
	int smoothSize = 30;
	//img.convertTo(img, CV_8U);
	cv::medianBlur(img, img, medianSize);
	cv::blur(img, img, cv::Size(smoothSize, smoothSize));
	cv::minMaxLoc(img, &min_brightness, &max_brightness, NULL, NULL);
	mask = cv::Mat(img.size(), CV_8U);
	// Mask will be scaled between 0 and 1
	// Alternative method (maybe better)
	//cv::threshold(img, mask, min_brightness + (max_brightness - min_brightness) / 5.0, 255, CV_THRESH_BINARY);
	mask = img > min_brightness + (max_brightness - min_brightness) / 5.0;
	//cv::blur(mask, mask, cv::Size(smoothSize, smoothSize)); // Not really necessary
	cv::imshow("Mask", mask);
	cv::waitKey(1);
	background.release();
	background = NULL;
	return mask;
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Rect current_roi, int *x_offset, int *y_offset)
 *
 * @brief	Correlate roi.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param 		  	frame	   	The frame as an OpenCV matrix.
 * @param 		  	roi		   	The roi (the region of the frame) as an OpenCV matrix.
 * @param 		  	current_roi	The current roi as a rectangle (with the coordinates, width and height).
 * @param [in,out]	x_offset   	If non-null, the x offset between the ROI and the CM.
 * @param [in,out]	y_offset   	If non-null, the y offset between the ROI and the CM.
 *
 * @return	A cv::Rect which delimits the new ROI.
 **************************************************************************************************/

cv::Rect dtcCorrelateROI(cv::Mat frame, cv::Mat roi, cv::Rect current_roi, int *x_offset, int *y_offset) {

	cv::Mat img, region, newRoi;
	cv::Point maxLoc;
	
	frame.copyTo(img);
	roi.copyTo(region);

	int newRoi_cols = img.cols - region.cols + 1;
	int newRoi_rows = img.rows - region.rows + 1;

	newRoi = cv::Mat(newRoi_rows, newRoi_cols, CV_32F);
	cv::matchTemplate(img, region, newRoi, CV_TM_CCORR_NORMED);
	cv::normalize(newRoi, newRoi, -1, 1, cv::NORM_MINMAX, -1, cv::Mat());
	cv::minMaxLoc(newRoi, NULL, NULL, NULL, &maxLoc, cv::Mat());
	*x_offset = maxLoc.x - current_roi.x;
	*y_offset = maxLoc.y - current_roi.y;
	current_roi.x = maxLoc.x;
	current_roi.y = maxLoc.y;
	img.release();
	img = NULL;
	region.release();
	region = NULL;
	newRoi.release();
	newRoi = NULL;

	return current_roi;

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

	img.release();
	img = NULL;
	region.release();
	region = NULL;
	corrMat.release();
	corrMat = NULL;

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
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	sx = img.cols;
	sy = img.rows;
	cv::minMaxLoc(img, &min_brightness, &max_brightness, NULL, NULL);
	int avgBackground = cv::mean(img.col(0))[0] + cv::mean(img.col(img.cols - 1))[0] + cv::mean(img.row(0))[0] + cv::mean(img.row(img.rows - 1))[0];
	img -= avgBackground / 4;
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

	if ((tbuf = (double*)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double*)calloc(MAX(width, height), sizeof(double))) == NULL) {
		perror("ERROR in dtcGetGrayImageROI allocating memory");
		exit(EXIT_FAILURE);
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
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
		qsort(tbuf, medsize, sizeof(double), doublecmp);
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
	mask.release();
	mask = NULL;
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
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	float val;
	int hwd, hht;

	int xorig = 0;
	int yorig = 0;
	int width = img->cols;
	int height = img->rows;
	int step = img->step/sizeof (float);

	if ((tbuf = (double *)calloc(medsize, sizeof(double))) == NULL ||
		(mbuf = (double *)calloc(MAX(width, height), sizeof(double))) == NULL) {
		//perror("EROOR in dtcGetGrayMatImageROI allocating memory");
		DBOUT("ERROR in dtcGetGrayMatImageROI allocating memory\n");
		exit(EXIT_FAILURE);
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
 * @fn	cv::Mat dtcLumThreshold_ToZero2(Image imgsrc, Image imgdst, double threshold)
 *
 * @brief	Applies the threshold to zero to the frame and saves it in the destination.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	imgsrc   	The imgsrc.
 * @param	imgdst   	The imgdst.
 * @param	threshold	The threshold.
 *
 * @return	A cv::Mat.
 **************************************************************************************************/

cv::Mat dtcLumThreshold_ToZero2(Image imgsrc, Image imgdst, double threshold)
{
	int xorig, yorig, width, height;
	int i, j;
	uchar *src;
	uchar *srcx;

	dtcGetROI(imgsrc, &xorig, &yorig, &width, &height);

	src = (uchar *)(imgsrc.frame.data + xorig * imgsrc.frame.channels() + yorig * imgsrc.frame.step);
	for (i = 0; i < height; i++, src += imgsrc.frame.step)
	{
		srcx = src;
		for (j = 0; j < width; j++, srcx += imgsrc.frame.channels())
		{
			if (threshold > KR * srcx[0] + KG * srcx[1] + KB * srcx[2])
			{
				uchar *dst = (uchar *)imgdst.frame.data + (srcx - (uchar *)imgsrc.frame.data);
				dst[0] = dst[1] = dst[2] = 0;
			}
		}
	}

	return imgsrc.frame;
}

/**********************************************************************************************//**
 * @fn	double dtcGetImageLum(Image img)
 *
 * @brief	Get the luminosity of the image.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The image (frame + ROI).
 *
 * @return	A double.
 **************************************************************************************************/

double dtcGetImageLum(Image img)
{
	int xorig, yorig, width, height;
	double lum = 0.0;
	double Y;
	int i, j;
	uchar *src;
	uchar *srcx;

	if (img.roi.area()) {
		xorig = img.roi.x;
		yorig = img.roi.y;
		width = img.roi.width;
		height = img.roi.height;
	}
	else {
		xorig = 0;
		yorig = 0;
		width = img.frame.cols;
		height = img.frame.rows;
	}

	src = (uchar *)(img.frame.data + xorig * img.frame.channels() + yorig * img.frame.step);
	for (i = 0; i < height; i++, src += img.frame.step) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img.frame.channels()) {
			Y = KR * srcx[0] + KG * srcx[1] + KB * srcx[2];
			if (Y < opts.threshold) Y = 0.0;
			lum += Y;
		}
	}

	return lum / (height * width);
}

/**********************************************************************************************//**
 * @fn	DtcImageVals dtcGetImageVals(Image img)
 *
 * @brief	Get image vals.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The image.
 *
 * @return	The DtcImageVals.
 **************************************************************************************************/

DtcImageVals dtcGetImageVals(Image img)
{
	int xorig, yorig, width, height;
	double lum;
	DtcImageVals vals = { 0.0, 256.0, 0.0 };
	int i, j;
	uchar *srcx;
	uchar *src;

	if (img.roi.area()) {
		xorig = img.roi.x;
		yorig = img.roi.y;
		width = img.roi.width;
		height = img.roi.height;
	}
	else {
		xorig = 0;
		yorig = 0;
		width = img.frame.cols;
		height = img.frame.rows;
	}

	src = (uchar *)(img.frame.data + xorig * img.frame.channels() + yorig * img.frame.step);
	for (i = 0; i < height; i++, src += img.frame.step) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img.frame.channels()) {
			lum = KR * srcx[0] + KG * srcx[1] + KB * srcx[2];
			if (vals.maxlum < lum) vals.maxlum = lum;
			if (vals.minlum > lum) vals.minlum = lum;
			vals.lum += lum;
		}
	}

	vals.lum /= width * height;

	return vals;
}

/**********************************************************************************************//**
 * @fn	DtcImageVals dtcGetGrayImageVals(Image img)
 *
 * @brief	Get gray image vals.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	img	The image.
 *
 * @return	The DtcImageVals.
 **************************************************************************************************/

DtcImageVals dtcGetGrayImageVals(Image img)
{
	int xorig, yorig, width, height;
	double lum;
	DtcImageVals vals = { 0.0, 256.0, 0.0 };
	int i, j;
	uchar *src;
	uchar *srcx;

	if (img.roi.area()) {
		xorig = img.roi.x;
		yorig = img.roi.y;
		width = img.roi.width;
		height = img.roi.height;
	} else {
		xorig = 0;
		yorig = 0;
		width = img.frame.cols;
		height = img.frame.rows;
	}

	src = (uchar *)(img.frame.data + xorig * img.frame.channels() + yorig * img.frame.step);
	for (i = 0; i < height; i++, src += img.frame.step) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img.frame.channels()) {
			lum = srcx[0];
			if (vals.maxlum < lum) vals.maxlum = lum;
			if (vals.minlum > lum) vals.minlum = lum;
			vals.lum += lum;
		}
	}

	vals.lum /= width * height;

	return vals;
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

/**********************************************************************************************//**
 * @fn	cv::Mat dtcGetGrayMat(cv::Mat frame)
 *
 * @brief	Get grayscale frame
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	frame	The original frame from the video.
 *
 * @return	A cv::Mat with the frame in grayscale.
 **************************************************************************************************/

cv::Mat dtcGetGrayMat(cv::Mat frame, DtcCapture *capture)
{
	
	if (!frame.data) return cv::Mat();

	cv::Mat gray, frame_to_gray;

	gray = cv::Mat(frame.size(), CV_32F);
	frame.copyTo(frame_to_gray);

	if (frame.channels() > 1) {
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
				cv::split(frame, frame_channels);
				frame.convertTo(frame, CV_8UC1);
				frame = (frame_channels[0] + frame_channels[1] + frame_channels[2]) / 3;
				cv::cvtColor(frame, frame, opts.bayer);
				frame.copyTo(frame_to_gray);
				cv::cvtColor(frame_to_gray, gray, CV_RGB2GRAY);
			} else {
				cv::cvtColor(frame_to_gray, gray, CV_BGR2GRAY);
			}
		}
	} else if (frame.channels() == 1) {
		if (opts.bayer > 0) {
			cv::Mat debayered = cv::Mat(frame.size(), CV_32FC3);
			cv::cvtColor(frame, debayered, opts.bayer);
			cv::cvtColor(debayered, gray, CV_RGB2GRAY);
			debayered.release();
			debayered = NULL;
		} else {
			frame_to_gray.copyTo(gray);
		}
	} else {
		gray.release();
		gray = NULL;
		exit(EXIT_FAILURE);
//		return cv::Mat();
	}
	frame_to_gray.release();
	frame_to_gray = NULL;
	return gray;
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
		gray.release();
		gray = NULL;
		exit(EXIT_FAILURE);
//		return cv::Mat();
	}
	frame_to_gray.release();
	frame_to_gray = NULL;
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

cv::Rect dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore, int ign) {
	int error = 0;
	cv::Rect win, roi = cv::Rect(0, 0, 0, 0);
	cv::Mat gray;
	cv::Mat frame;

	unsigned long nframe;
	for (nframe = 1; !opts.nframesROI || nframe <= opts.nframesROI; nframe++) {
		error = 0;
		frame = dtcQueryFrame2(pcapture, ignore, &error);
		if (!frame.data) return roi;
		if (error == 0) {
			cv::Point cm;
			gray = dtcGetGrayMat(&frame, pcapture);
			gray = dtcApplyMask(gray.clone());
			cm = dtcGetGrayMatCM(gray);
			if (cm.x <= 0 || cm.y < 0) throw std::logic_error("Negative or zero centre of mass, can't obtain Region of Interest");
			win = dtcGetGrayImageROIcCM(gray, cm, opts.medSize, opts.facSize, opts.secSize);
			roi = dtcMaxRect(win, roi);
			gray.release();
			gray = NULL;
			frame.release();
			frame = NULL;
			if (opts.debug) { 
				DBOUT("dtcGetFileROIcCM: frame " << nframe << "\n")
			}
		}
	}

	return roi;
}

/**********************************************************************************************//**
 * @fn	cv::Rect dtcGetFrameROIcCM(cv::Mat frame, cv::Point cm, const int ignore, int ign)
 *
 * @brief	Dtc get frame ro ic centimetres.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	frame 	The frame.
 * @param	cm	  	The centimetres.
 * @param	ignore	The ignore.
 * @param	ign   	The ign.
 *
 * @return	A cv::Rect.
 **************************************************************************************************/

cv::Rect dtcGetFrameROIcCM(cv::Mat frame, cv::Point cm, const int ignore, int ign) {
	cv::Rect win, roi = cv::Rect(0, 0, 0, 0);
	if (!frame.data) return roi;
	win = dtcGetGrayImageROIcCM(frame, cm, opts.medSize, opts.facSize, opts.secSize);
	roi = dtcMaxRect(win, roi);
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

void dtcDrawImpact(cv::Mat frame, cv::Point point, cv::Scalar colour) {
	cv::line(frame, cv::Point(point.x + 20, point.y), cv::Point(point.x + 30, point.y), colour, 1, 8, 0);
	cv::line(frame, cv::Point(point.x - 30, point.y), cv::Point(point.x - 20, point.y), colour, 1, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y - 30), cv::Point(point.x, point.y - 20), colour, 1, 8, 0);
	cv::line(frame, cv::Point(point.x, point.y + 20), cv::Point(point.x, point.y + 30), colour, 1, 8, 0);
}

/**********************************************************************************************//**
 * @fn	cv::Mat dtcRunningAvg(Image imgsrc, Image imgdst, double lR)
 *
 * @brief	Running average.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	imgsrc	The source image.
 * @param	imgdst	The destination imaget.
 * @param	lR	  	The learning rate.
 *
 * @return	A cv::Mat with the destination image.
 **************************************************************************************************/

cv::Mat dtcRunningAvg(Image imgsrc, Image imgdst, double lR)
{
	int x_srco, y_srco;
	int x_dsto, y_dsto;
	int width, height, nChannels;
	uchar *src;
	uchar *dst;
	uchar *srcx;
	uchar *dstx;

	dtcGetROI(imgsrc, &x_srco, &y_srco, &width, &height);
	dtcGetROI(imgdst, &x_dsto, &y_dsto, &width, &height);
	nChannels = imgsrc.frame.channels();

	src = (uchar *)(imgsrc.frame.data + x_srco * imgsrc.frame.channels() + y_srco * imgsrc.frame.step);
	dst = (uchar *)(imgdst.frame.data + x_dsto * imgdst.frame.channels() + y_dsto * imgdst.frame.step);
	for (int i = 0; i < height; i++, src += imgsrc.frame.step, dst += imgdst.frame.step)
	{
		srcx = src;
		dstx = dst;
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < nChannels; k++)
			{
				dstx[k] = (uchar)(lR * srcx[k] + (1.0 - lR) * dstx[k]);
				++srcx;
				++dstx;
			}
		}
	}

	return imgdst.frame;
}

/**********************************************************************************************//**
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
	pHisImg = cv::Mat::zeros(vsize*scale, hsize*scale, CV_8UC1);
	if (thr) {
		pHis.at<float>(0) = 0;
	}
	cv::minMaxLoc(pHis, 0, &max_val, 0, 0);
	pHisImg = cv::Scalar(255, 255, 255, 0);
	if (max_val > 0) {
		for (i = 0; i < hsize; i++){
			bval = pHis.at<float>(i);
			cv::rectangle(pHisImg, cv::Point(i*scale, (vsize - cvRound(bval*hsize / max_val))*scale), 
				cv::Point((i + 1)*scale - 1, vsize*scale), CV_RGB(0, 0, 0), CV_FILLED, 8, 0);
		}
	}
	pHis.release();
	pHis = NULL;
	return pHisImg;
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

cv::VideoWriter *dtcWriteVideo(const char *file, cv::VideoWriter writer, DtcCapture *capture, cv::Mat img)
{
	cv::Mat color;
	double fps;
	cv::Size size;

	if (!img.data) return nullptr;

	if (!writer.isOpened()) {
		fps = dtcGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		size = cv::Size(img.cols, img.rows);
		writer = cv::VideoWriter(file,
		#if defined(_WIN32)
			CV_FOURCC('D', 'I', 'B', ' '),
		#else
			CV_FOURCC('M', 'J', 'P', 'G'),
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

bool isEqual(cv::Mat m1, cv::Mat m2) {
	if (m1.empty() && m2.empty()) return true;
	if (m1.cols != m2.cols || m1.rows != m2.rows || m1.dims != m2.dims) return false;
	if (m1.type() != m2.type()) return false;
	return cv::countNonZero(m1 != m2) == 0;
}

void dtcApplyDifferentialPhotometry(cv::Mat& original, cv::Mat& reference, cv::Mat& difference,
	cv::Mat& tracking) {

}