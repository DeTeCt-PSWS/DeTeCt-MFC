/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    WRAPPER: Different acquisitions format routing functions					*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdio.h>
#include <ctype.h>

extern "C" {
#include "wrapper.h"
}
#include "wrapper2.h"
#include "serfmt.h"
#include "dtcgui.hpp"

/**********************************************************************************************//**
* @fn	DtcCapture *dtcCaptureFromFile2(const char *fname, int *pframecount)
*
* @brief	Dtc capture from file. Originally meant to use new functions, but have been failing:
* 			cv::VideoCapture seems to throw exceptions.
* 			TO-DO: Check VideoCapture errors
*
* @author	Jon
* @date	2017-05-12
*
* @param 		  	fname	   	Filename.
* @param [in,out]	pframecount	If non-null, the pframecount.
*
* @return	Null if it fails, else a pointer to a DtcCapture.
**************************************************************************************************/

DtcCapture *dtcCaptureFromFile2(const char *fname, int *pframecount)
{
	char ext[EXT_MAX];
	DtcCapture *capt;

	get_fileextension(fname, ext, EXT_MAX);
	lcase(ext, ext);
	capt = (DtcCapture *)malloc(sizeof(DtcCapture));
	if (capt == NULL) {
		assert(capt != NULL);
	}
	else {
		if (opts.debug) { fprintf(stderr, "dtcCaptureFromFile: capt @ %d\n", (int)(capt)); }
		if (!strcmp(ext, "ser")) {
			capt->type = CAPTURE_SER;
			if (!(capt->u.sercapture = serCaptureFromFile(fname)))
			{
				free(capt);
				capt = NULL;
				OutputDebugString(L"ERROR in dtcCaptureFromFile processing ser file\n");
				//fprintf(stderr, "ERROR in dtcCaptureFromFile processing ser file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = capt->u.sercapture->header.FrameCount;
			if (capt->u.sercapture->header.PixelDepth > 8)
				serFixPixelDepth(capt->u.sercapture, 0);
			serPrintHeader(capt->u.sercapture);
		}
		else if ((!strcmp(ext, "fit")) || (!strcmp(ext, "fits"))) {
			capt->type = CAPTURE_FITS;
			if (!(capt->u.filecapture = FileCaptureFromFile(fname, pframecount, capt->type)))
			{
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile processing fits file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = capt->u.filecapture->FrameCount;
		}
		else if ((!strcmp(ext, "bmp")) || (!strcmp(ext, "dib")) || (!strcmp(ext, "jpeg")) || (!strcmp(ext, "jpg")) || (!strcmp(ext, "jpe")) || (!strcmp(ext, "jp2")) || (!strcmp(ext, "png")) || (!strcmp(ext, "pbm")) || (!strcmp(ext, "pgm")) || (!strcmp(ext, "ppm")) || (!strcmp(ext, "sr")) || (!strcmp(ext, "ras")) || (!strcmp(ext, "tiff")) || (!strcmp(ext, "tif"))) {
			capt->type = CAPTURE_FILES;
			if (!(capt->u.filecapture = FileCaptureFromFile(fname, pframecount, capt->type)))
			{
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile processing fits file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = capt->u.filecapture->FrameCount;
		}
		else {
			capt->type = CAPTURE_CV;
			//if (!(capt->u.capture = cvCreateFileCapture(fname, CV_LOAD_IMAGE_ANYCOLOR))) {
			if (!(capt->u.capture = cvCaptureFromFile(fname))) {

				//if (!(capt->u.videocapture = cv::makePtr<cv::VideoCapture>(cv::VideoCapture(fname)))) {
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile opening file %s\n", fname);
				exit(EXIT_FAILURE);
			}
		}
		(*pframecount) = capt->framecount;
	}
	return capt;
}


/**********************************************************************************************//**
* @fn	cv::Mat dtcQueryFrame2(DtcCapture *capture, const int ignore, int *perror)
*
* @brief	Query frame. New version. Converts the IplImage* instance returned by the respective
*			QueryFrame (CV and FILE) and QueryFrameData (SER) function into a cv::Mat.
* 			In case of SER files, a pointer (void*) to the data is used to create the cv::Mat
*			taking into account the byte depth and number of channels. When Bayer filters are
*			used, the image must change colourspaces.
*
* @author	Jon
* @date	2017-05-12
*
* @param [in,out]	capture	If non-null, the capture.
* @param 		  	ignore 	The ignore.
* @param [in,out]	perror 	If non-null, the perror.
*
* @return	A cv::Mat.
**************************************************************************************************/

cv::Mat dtcQueryFrame2(DtcCapture *capture, const int ignore, int *perror) {
	(*perror) = 0;
	void	*ser_frame_data = nullptr;	// raw SER frame data
	int		conversion = 0;				// conversion for SER Bayer modes
	cv::Mat	ser_frame;					// matrix used to contain frame data
	cv::Scalar black;					// in case of frame reading error we create an empty black matrix
	switch (capture->type) {
	case CAPTURE_SER:
		ser_frame_data = serQueryFrameData(capture->u.sercapture, ignore, perror);
		switch (capture->u.sercapture->header.ColorID) {
		case SER_BAYER_RGGB: conversion = cv::COLOR_BayerRG2RGB; break;
		case SER_BAYER_BGGR: conversion = cv::COLOR_BayerBG2RGB; break;
		case SER_BAYER_GRBG: conversion = cv::COLOR_BayerGR2RGB; break;
		case SER_BAYER_GBRG: conversion = cv::COLOR_BayerGB2RGB; break;
		default: conversion = 0; break;
		}
		/* Creation of the matrix with the SER frame data: w·h, 8/16 unsigned bit 1/3 channel image */
		if (capture->u.sercapture->current_frame <= capture->u.sercapture->header.FrameCount && ser_frame_data == NULL) {
			if (capture->u.sercapture->nChannels == 3)
				black = cv::Scalar(0, 0, 0);
			else
				black = cv::Scalar(0);
			ser_frame = cv::Mat(capture->u.sercapture->header.ImageHeight, capture->u.sercapture->header.ImageWidth,
				capture->u.sercapture->mat_type, black);
		} else {
			ser_frame = cv::Mat(capture->u.sercapture->header.ImageHeight, capture->u.sercapture->header.ImageWidth,
				capture->u.sercapture->mat_type, ser_frame_data);
		}
		/* The matrix might be empty (after the last frame) */
		if (ser_frame.data) {
			/*
			 * Normalise 16U matrices
			 * CV_8UC1 = 0 (0 * 8) and CV_8C3 = 16 (2 * 8)
			 */
			int mat_type = (capture->u.sercapture->nChannels - 1) * 8;
			if (capture->u.sercapture->byte_depth == 2) ser_frame.convertTo(ser_frame, mat_type, 1 / 256.0);
			/*
			 * Conversion from Bayer to colour, when applicable
			 * Conversion from RGB to BGR (used by OpenCV) to conserve the original colours
			 * done when getting the gray frame
			 */
			if (conversion != 0) cvtColor(ser_frame, ser_frame, conversion);
		}
		return ser_frame;
		break;
	case CAPTURE_FITS: case CAPTURE_FILES:
		return cv::cvarrToMat(fileQueryFrame(capture->u.filecapture, ignore, perror));
		break;
	default: // CAPTURE_CV
		return cv::cvarrToMat(cvQueryFrame(capture->u.capture));
		break;
	}
}