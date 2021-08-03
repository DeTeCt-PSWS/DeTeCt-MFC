/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    WRAPPER: Different acquisitions format routing functions					*/
/*                                                                              */
/********************************************************************************/
#include "processes_queue.hpp"
#include <windows.h> //after processes_queue.h

#include "common.h"

#include <stdio.h>
#include <ctype.h>

extern "C" {
#include "wrapper.h"
}
#include "wrapper2.h"
#include "serfmt.h"
#include "dtcgui.hpp"

#include "common2.h"

//#include <opencv2/imgproc.hpp> //TEST opencv3

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
		/*capt->pCaptureInfo = (DtcCaptureInfo*)malloc(sizeof(DtcCaptureInfo));

		if (capt->pCaptureInfo == NULL) {
			assert(capt->pCaptureInfo != NULL);
		}*/
		if (opts.debug) { fprintf(stderr, "!Debug info: dtcCaptureFromFile: capt @ %p\n", capt); }
		if (!strcmp(ext, "ser")) {
			capt->type = CAPTURE_SER;
			capt->u.sercapture = NULL;
			if (!(capt->u.sercapture = serCaptureFromFile(fname)))
			{
				free(capt);
				capt = NULL;
				OutputDebugString(L"ERROR in dtcCaptureFromFile processing ser file\n");
				//fprintf(stderr, "ERROR in dtcCaptureFromFile processing ser file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = (int)capt->u.sercapture->header.FrameCount;
			if (capt->u.sercapture->header.PixelDepth > 8)
				serFixPixelDepth(capt->u.sercapture, 0);
			serPrintHeader(capt->u.sercapture);
		}
		else if ((!strcmp(ext, "fit")) || (!strcmp(ext, "fits"))) {
			capt->type = CAPTURE_FITS;
			capt->u.filecapture = NULL;
			if (!(capt->u.filecapture = FileCaptureFromFile(fname, pframecount, capt->type)))
			{
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile processing fits file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = (int)capt->u.filecapture->FrameCount;
		}
		else if ((!strcmp(ext, "bmp")) || (!strcmp(ext, "dib")) || (!strcmp(ext, "jpeg")) || (!strcmp(ext, "jpg")) || (!strcmp(ext, "jpe")) || (!strcmp(ext, "jp2")) || (!strcmp(ext, "png")) || (!strcmp(ext, "pbm")) || (!strcmp(ext, "pgm")) || (!strcmp(ext, "ppm")) || (!strcmp(ext, "sr")) || (!strcmp(ext, "ras")) || (!strcmp(ext, "tiff")) || (!strcmp(ext, "tif"))) {
			capt->type = CAPTURE_FILES;
			capt->u.filecapture = NULL;
			if (!(capt->u.filecapture = FileCaptureFromFile(fname, pframecount, capt->type)))
			{
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile processing fits file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = (int)capt->u.filecapture->FrameCount;
		}
		else {
			capt->type = CAPTURE_CV;
			capt->u.capture = NULL;
			//if (!(capt->u.capture = cvCreateFileCapture(fname, CV_LOAD_IMAGE_ANYCOLOR))) {
			/*capt->u.capture = cvCaptureFromFile(fname); 
			if (capt->u.capture == NULL) {*/
			//if ((capt->u.capture = cvCaptureFromFile(fname)) != 0) {
			if (!(capt->u.capture = cvCaptureFromFile(fname))) {

				//if (!(capt->u.videocapture = cv::makePtr<cv::VideoCapture>(cv::VideoCapture(fname)))) {
				//free(capt->u.capture);
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile opening file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = (int)(dtcGetCaptureProperty(capt, CV_CAP_PROP_FRAME_COUNT));
		}
		(*pframecount) = capt->framecount;
		//initDtcCaptureInfo(capt->pCaptureInfo);
		initDtcCaptureInfo(&capt->CaptureInfo);
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
	void		*ser_frame_data = nullptr;	// raw SER frame data
	int			conversion = 0;				// conversion for SER Bayer modes
	int			mat_type = 0;
	cv::Mat		matrix_frame;					// matrix used to contain frame data
	cv::Scalar	black;					// in case of frame reading error we create an empty black matrix
	double		minVal;
	double		maxVal;
	cv::Point	minLoc;
	cv::Point	maxLoc;
	int			maxbits;

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
			matrix_frame = cv::Mat((int)capture->u.sercapture->header.ImageHeight, (int)capture->u.sercapture->header.ImageWidth,
				capture->u.sercapture->mat_type, black);
		} else {
			matrix_frame = cv::Mat((int)capture->u.sercapture->header.ImageHeight, (int)capture->u.sercapture->header.ImageWidth,
				(size_t)capture->u.sercapture->mat_type, ser_frame_data);
		}
		/* The matrix might be empty (after the last frame) */
		if (matrix_frame.data) {
			/*
			 * Normalise 16U matrices
			 * CV_8UC1 = 0 (0 * 8) and CV_8C3 = 16 (2 * 8)
			 */
			mat_type = (capture->u.sercapture->nChannels - 1) * 8;
			if (capture->u.sercapture->byte_depth == 2) matrix_frame.convertTo(matrix_frame, mat_type, 1 / 256.0);
			/*
			 * Conversion from Bayer to colour, when applicable
			 * Conversion from RGB to BGR (used by OpenCV) to conserve the original colours
			 * done when getting the gray frame
			 */
			if (conversion != 0) cvtColor(matrix_frame, matrix_frame, conversion);
		}
		return matrix_frame;
		break;
	case CAPTURE_FITS: case CAPTURE_FILES:
		matrix_frame = cv::cvarrToMat(fileQueryFrame(capture->u.filecapture, ignore, perror));
		/* The matrix might be empty (after the last frame) */
		if (matrix_frame.data) {
			/*
			* Normalise 16U matrices
			* CV_8UC1 = 0 (0 * 8) and CV_8C3 = 16 (2 * 8)
			*/
			mat_type = (capture->u.filecapture->image->nChannels - 1) * 8;
			// find maximum number of bits used
			minMaxLoc(matrix_frame, &minVal, &maxVal, &minLoc, &maxLoc);
			if (maxVal > pow(2, capture->u.filecapture->MaxBits)) {
				maxbits = 16;
				while (maxVal <= pow(2, maxbits)) maxbits--;
				maxbits++;
				if (maxbits < 8) maxbits = 8; // image coded on 2 bytes ...
				// changes maximum number of bits used from capture if this frame has a higher value
				if (maxbits > capture->u.filecapture->MaxBits) capture->u.filecapture->MaxBits = maxbits;
			} else maxbits = capture->u.filecapture->MaxBits;
			// Scale matrix, ie:
			//	MaxBit (2^y)		Scalebit (1/2^x)
			//	8b					0
			//	12b					4
			//	16b					8
			if (capture->u.filecapture->BytesPerPixel == 2) matrix_frame.convertTo(matrix_frame, mat_type, 1 / pow(2.0, capture->u.filecapture->MaxBits - 8));
		}
		return matrix_frame;
		break;
	default: // CAPTURE_CV
		return cv::cvarrToMat(cvQueryFrame(capture->u.capture));
		break;
	}
}

void dtcReinitCaptureRead2(DtcCapture **pcapture, const char *fname)
{
	int framecount;
	DtcCaptureInfo CaptureInfoSave;

	//CaptureInfoSave = (*(*pcapture)->pCaptureInfo);
	CaptureInfoSave = (*pcapture)->CaptureInfo;
	switch ((*pcapture)->type)
	{
	case CAPTURE_SER:
		serReinitCaptureRead((*pcapture)->u.sercapture, fname);
		break;
	case CAPTURE_FITS:
	case CAPTURE_FILES:
		fileReinitCaptureRead((*pcapture)->u.filecapture, fname);
		break;
	default: // CAPTURE_CV	
		dtcReleaseCapture(*pcapture);
		if (!(*pcapture = dtcCaptureFromFile2(fname, &framecount))) {
			fprintf(stderr, "ERROR in dtcReinitCaptureRead2 opening file %s\n", fname);
			exit(EXIT_FAILURE);
		}
		break;
	}
	//(*(*pcapture)->pCaptureInfo) = CaptureInfoSave;
	(*pcapture)->CaptureInfo = CaptureInfoSave;
}


void dtcReleaseCapture(DtcCapture *capture)
{
	switch (capture->type)
	{
	case CAPTURE_SER:
		/*fprintf(stderr,"dtcReleaseCapture: Releasing ser capture\n");
		fprintf(stderr, "dtcReleaseCapture: Image pointer %d\n", capture->u.sercapture->image);*/
		serReleaseCapture(capture->u.sercapture);
		break;
	case CAPTURE_FITS:
	case CAPTURE_FILES:
		fileReleaseCapture(capture->u.filecapture);
		break;
	default: // CAPTURE_CV
		//capture->u.videocapture->release();
		cvReleaseCapture(&capture->u.capture);
	}
	//free(capture->pCaptureInfo);
}

/**********************************************************************************************//**
* @fn	BOOL Is_Capture_OK_From_File(const std::string file, std::string *pfilename_acquisition, int *pnframe, std::wstringstream *pmessage)
*
* @brief	Check if capture is ok (file can be opened, capture from autostakkert session file ok)
*			Returns number of frames
*			Returns potential error message
*
* @author	Marc
* @date	2020-04-13
*
* @param [in]		file						string of capture filename
* @param [out]		pfilename_acquisition	   	pointer to acquisition filename
* @param [out]		pnframe						pointer to number of acquisition frames
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
*
* @return			True if ok, False if error
**************************************************************************************************/

BOOL Is_Capture_OK_from_File(const std::string file, std::string *pfilename_acquisition, int *pnframe, std::wstringstream *pmessage)
{
	DtcCapture *pCapture;

	pCapture = NULL;
	std::string extension = file.substr(file.find_last_of(".") + 1, file.size() - file.find_last_of(".") - 1);
	(*pnframe) = -1;

	if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
		int tmp1 = 0;
		int tmp2 = 9999999;
		int tmp3 = 0;
		read_autostakkert_session_file(file, pfilename_acquisition, NULL, &tmp1, &tmp2, &tmp3);
		//					filename_acquisition = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());
	}
	else
		(*pfilename_acquisition) = std::string(file.begin(), file.end());

	if (!filesys::exists((*pfilename_acquisition).c_str())) {
		if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
			// Error if autostakkert acquisition file cannot be found
			(*pmessage) << "Error, ignoring " << file.substr(file.find_last_of("\\") + 1, file.length()).c_str() << " (cannot open file acquisition file " << (*pfilename_acquisition).c_str() << ", " << strerror(errno) << ")\n";
		}
		else
			(*pmessage) << "Error, ignoring " << (*pfilename_acquisition).c_str() << ", cannot open file (" << strerror(errno) << ")\n";
		return FALSE;
	}

	if (!(pCapture = dtcCaptureFromFile2((*pfilename_acquisition).c_str(), pnframe))) {
		// ********* Error if acquisition file cannot be opened / is missing
		dtcReleaseCapture(pCapture);
		pCapture = NULL;
		if (extension.compare(AUTOSTAKKERT_EXT) == 0) {
			// Error if autostakkert acquisition file cannot be found
			(*pmessage) << "Error, ignoring " << file.substr(file.find_last_of("\\") + 1, file.length()).c_str() << " (cannot open file acquisition file " << (*pfilename_acquisition).c_str() << ", " << strerror(errno) << ")\n";
		}
		else
			(*pmessage) << "Error, ignoring  " << (*pfilename_acquisition).c_str() << ", cannot open file (" << strerror(errno) << ")\n";
		return FALSE;
	}
	else {
		dtcReleaseCapture(pCapture);
		return TRUE;
	}
}

/**********************************************************************************************//**
* @fn	BOOL Is_Capture_Long_Enough(const std::string file, const int nframe, std::wstringstream *pmessage)
*
* @brief	Check if capture has more than the minimum number of frames required
*			Returns potential error message
*
* @author	Marc
* @date	2020-04-13
*
* @param [in]		file						string of capture filename
* @param [out]		nframe						number of acquisition frames
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
*
* @return			True if capture has more than the minimum number of frames
**************************************************************************************************/

BOOL Is_Capture_Long_Enough(const std::string file, const int nframe, std::wstringstream *pmessage) {
	std::string shortfile = file.substr(file.find_last_of("\\") + 1, file.length());

	if ((nframe > 0) && (nframe < opts.minframes)) {
//********* Error if acquisition has not enough frames
		(*pmessage) << "Ignoring " << shortfile.c_str() << ", only " << nframe << " frame";
		if (nframe != 1)  (*pmessage) << "s";
		(*pmessage) << " (minimum is " << opts.minframes << ")\n";
		return FALSE;
	}
	return TRUE;
}

/**********************************************************************************************//**
* @fn	BOOL Is_Capture_Special_Type(const std::string file, std::wstringstream *pmessage)
*
* @brief	Check if capture is of a special type (dark, from PIPP, from WinJupos)
*			Ignore the special type capture if flag is setup for it
*			Returns potential error message
*
* @author	Marc
* @date	2020-04-13
*
* @param [in]		file						string of capture filename
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
*
* @return			True if of special type to be ignored, False otherwise if not to be ignored
**************************************************************************************************/

BOOL Is_Capture_Special_Type(const std::string file, std::wstringstream *pmessage)
{
	std::string shortfile = file.substr(file.find_last_of("\\") + 1, file.length());

	if (IGNORE_DARK && (file.find(DARK_STRING) != std::string::npos)) {
		(*pmessage) << "Ignoring " << shortfile.c_str() << " dark file\n";
		return TRUE;
	} else if (IGNORE_PIPP && (file.find(PIPP_STRING) != std::string::npos)) {
		(*pmessage) << "Ignoring " << shortfile.c_str() << " PIPP file\n";
		return TRUE;
	} else if (IGNORE_WJ_DEROTATION && (file.find(WJ_DEROT_STRING) != std::string::npos)) {
		(*pmessage) << "Ignoring " << shortfile.c_str() << " WinJupos derotated file\n";
		return TRUE;
	}
	return FALSE;
}

/**********************************************************************************************//**
* @fn	BOOL Is_CaptureFile_To_Be_Processed(const std::string filename_acquisition, std::wstringstream *pmessage)
*
* @brief	Depending if no reprocessing option is on, checks if file has already been processed
*			Returns if file has to be processed depending on the option or if already processed
*			Returns potential error message

* @author	Marc
* @date	2020-04-13
*
* @param [in]		filename_acquisition		string of capture filename
* @param [in]		log_filename				string of log filename to be checked
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
*
* @return			True if file has to be processed, False otherwise
**************************************************************************************************/

BOOL Is_CaptureFile_To_Be_Processed(const std::string filename_acquisition, const std::string log_filename, std::wstringstream *pmessage) {

// ***** if option noreprocessing on (or autostakkert mode), check in detect log file if file already processed or processed with in datation only mode
	if ((!opts.reprocessing) || (opts.autostakkert)) {

		/*std::string input_file_name;
		if (log_filename == "") {
			std::string folder_path = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\"));
			CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder((CString)folder_path.c_str(), DTC_LOG_SUFFIX));
			input_file_name = DeTeCtLogFilename;
		} else {
			input_file_name = log_filename;
		}*/
		std::ifstream input_file(log_filename, std::ios_base::in);
		if (input_file) {
			std::string line;
			//0.0000		 0       ; 2015/01/17 04:42,059300 UT; 2015/01/17 04:44,058567 UT; 2015/01/17 04:43,058933 UT; 119.9560 s; 43.000 fr/s; G:\Work\Impact\tests\data_set\bugs\ACo\J_2015Jan17_044203_RGB.avi; DeTeCt v3.1.8.20190512_x64 (Firecapture 2.4beta); Win8(or_above)_64b
			while (std::getline(input_file, line)) {
				if ((!starts_with(line, PROGNAME)) && (!starts_with(line, "PLEASE")) && (!starts_with(line, "confidence"))) {
					BOOL line_rated;
					if (starts_with(line, "N/A")) line_rated = FALSE;
					else line_rated = TRUE;
					size_t nb_separators = 0;
					size_t pos_separator;
					do {
						pos_separator = (int)line.find_first_of(";");
						if (pos_separator != std::string::npos) {
							nb_separators++;
							line = line.substr(pos_separator + 1, line.size() - pos_separator - 1);
						}
					} while ((pos_separator != std::string::npos) && (nb_separators < 6));
					while (starts_with(line, " ")) line = line.substr(1, line.size() - 1);
					while (starts_with(line, "\t")) line = line.substr(1, line.size() - 1);
					std::for_each(line.begin(), line.end(), [](char& c) {
						c = (char) ::tolower(c);
						});
					std::string filename_acquisition_local = filename_acquisition;
					std::for_each(filename_acquisition_local.begin(), filename_acquisition_local.end(), [](char& c) {
						c = (char) ::tolower(c);
						});
					if ((nb_separators == 6)
						&& (line.find_first_of(";") != std::string::npos)
						&& (starts_with(line, filename_acquisition_local))
						&& (!opts.dateonly)
						&& (line_rated)) {
							std::string shortfile = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());

							(*pmessage) << "Ignoring " << shortfile.c_str() << ", already processed\n";
							input_file.close();
							return FALSE;
					}
				}
			}
			input_file.close();
			return TRUE;
		}
		input_file.close();
	}
	return TRUE;
}