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

#include <stdbool.h>
#include <stdio.h>
#include <ctype.h>

//extern "C" {
#include "wrapper.h"
//}
#include "wrapper2.hpp"
#include "serfmt.h"
#include "dtcgui.hpp"

#include "common2.hpp"

#include <opencv2/imgproc.hpp>  // test OpenCV 4.7.0 
#include "filefmt2.hpp" // test OpenCV 4.7.0 

//internal functions

//exposed functions

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
	void		*ser_frame_data = NULL;	// raw SER frame data nullptr
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
		if (ser_frame_data == NULL) {
			(*perror) = 1;
			return matrix_frame; //empty matrix_frame
		}
		switch (capture->u.sercapture->header.ColorID) {
			case SER_BAYER_RGGB: conversion = cv::COLOR_BayerRG2RGB; break;
			case SER_BAYER_BGGR: conversion = cv::COLOR_BayerBG2RGB; break;
			case SER_BAYER_GRBG: conversion = cv::COLOR_BayerGR2RGB; break;
			case SER_BAYER_GBRG: conversion = cv::COLOR_BayerGB2RGB; break;
			default: conversion = 0; break;
		}
		/* Creation of the matrix with the SER frame data: w�h, 8/16 unsigned bit 1/3 channel image */
		if ((capture->u.sercapture->current_frame <= capture->u.sercapture->header.FrameCount) && (ser_frame_data == NULL)) {
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
		if (!matrix_frame.empty()) {
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
//		matrix_frame = cv::cvarrToMat(fileQueryFrame(capture->u.filecapture, ignore, perror));
		matrix_frame = fileQueryFrameMat(capture->u.filecapture, ignore, perror); // test OpenCV 4.7.0
		/* The matrix might be empty (after the last frame) */
		if (!matrix_frame.empty()) {
			/*
			* Normalise 16U matrices
			* CV_8UC1 = 0 (0 * 8) and CV_8C3 = 16 (2 * 8)
			*/
			//mat_type = (capture->u.filecapture->image->nChannels - 1) * 8;
			mat_type = (matrix_frame.channels() - 1) * 8;
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
		/*(*perror) =				cvGrabFrame(capture->u.capture);
		IplImage *image =		cvRetrieveFrame(capture->u.capture);
		cv::Mat return_mat= cv::cvarrToMat(image);
		return return_mat;*/
		//return cv::cvarrToMat(cvQueryFrame(capture->u.capture));
		capture->u.videocapture->read(matrix_frame);
		return matrix_frame;
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
			 char msgtext[MAX_STRING] = { 0 };
			snprintf(msgtext, MAX_STRING, "cannot open file %s", fname);
			ErrorExit(TRUE, "cannot reinitialize capture", __func__, msgtext);
		}
		break;
	}
	//(*(*pcapture)->pCaptureInfo) = CaptureInfoSave;
	(*pcapture)->CaptureInfo = CaptureInfoSave;
}


void dtcReleaseCapture(DtcCapture *capture)
{
	if (capture != NULL) {
		switch (capture->type)
		{
		case CAPTURE_SER:
			/*fprintf(stdout,"dtcReleaseCapture: Releasing ser capture\n");
			fprintf(stdout, "dtcReleaseCapture: Image pointer %d\n", capture->u.sercapture->image);*/
			//if (&capture->u.capture != NULL) serReleaseCapture(capture->u.sercapture);
			if (&capture->u.videocapture != NULL) serReleaseCapture(capture->u.sercapture);
			break;
		case CAPTURE_FITS:
		case CAPTURE_FILES:
			//if (&capture->u.capture != NULL) fileReleaseCapture(capture->u.filecapture);
			if (&capture->u.videocapture != NULL) fileReleaseCapture(capture->u.filecapture);
			break;
		default: // CAPTURE_CV
			//capture->u.videocapture->release();
			//if (&capture->u.capture != NULL) cvReleaseCapture(&capture->u.capture);
			//if (&capture->u.videocapture != NULL) capture->u.videocapture->release();
			if (&capture->u.videocapture != NULL) delete capture->u.videocapture;
		}
		//free(capture->pCaptureInfo);
		capture = NULL;
	}
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

/**********************************************************************************************/
/**
* @fn		Is_PIPP(const std::string file)
*
* @brief	Returns if file is PIPP video
*
* @author	Marc
* @date	2022-07-31
*
* @param [in]		file						string of capture filename
* @return true if file is a PIPP video, false otherwise
**************************************************************************************************/
BOOL Is_PIPP(const std::string file)
{
	if (file.find(PIPP_STRING) == std::string::npos) return FALSE;	// Not a PIPP file
	else return TRUE;
}

/**********************************************************************************************/
/**
* @fn	BOOL Is_PIPP_OK(const std::string file, int *start_frame, int *max_nb_frames, std::wstringstream* pmessage)
*
* @brief	Check if PIPP file is ok (not modified by processing loosing detection potential)
*			Returns start frame (if first frame(s) removed by PIPP)
*			Returns (max) number of frames (if end frame(s) removed by PIPP)
*
* @author	Marc
* @date	2022-07-31
*
* @param [in]		file						string of capture filename
* @param [out]		start_frame					pointer to start frame
* @param [out]		max_nb_frames				pointer to maximum number of frames taken by PIPP
* @param [in,out]	pmessage					pointer to wstring streal of message to be displayed
*
* @return			True if PIPP file intergrity is ok, False otherwise (no integrity or not known)
**************************************************************************************************/

BOOL Is_PIPP_OK(const std::string file, PIPPInfo* pipp_info, std::wstringstream* pmessage)
{
	initPIPPInfo(pipp_info);
	(*pipp_info).isPIPP = Is_PIPP(file);
	if (!(*pipp_info).isPIPP) return TRUE;	// Not a PIPP file

	// jupiter_ch4_1_pipp_log.txt
	std::string pipp_log_filename = file.substr(0, file.find_last_of("\\") + 1) + "logs\\" + file.substr(file.find_last_of("\\") + 1, file.find_last_of(".") - file.find_last_of("\\") - 1) + "_log.txt";
	std::string pipp_short_filename = file.substr(file.find_last_of("\\") + 1, file.length() - file.find_last_of("\\") + 1);
	std::ifstream input;
	input.open(pipp_log_filename, std::ios::binary);

	if (!input) {
		// wjupos naming  of output file 
		// 10001-13-2418961-0757_1-jupiter_ch4_1_pipp
		// 0001-01-01-0000_0-jupiter_ch4_1_pipp
		// checks *-0757_1-*
		int start_org_filename_index = 0;
		while (start_org_filename_index < pipp_short_filename.length() - 8) {
			if ((pipp_short_filename.substr(start_org_filename_index, 1) == "-")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 1, 4)))
				&& (pipp_short_filename.substr(start_org_filename_index + 5, 1) == "_")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 6, 1)))
				&& (pipp_short_filename.substr(start_org_filename_index + 7, 1) == "-")) {
				input._close();
				std::string pipp_log_org_filename = file.substr(0, file.find_last_of("\\") + 1) + "logs\\" + pipp_short_filename.substr(start_org_filename_index + 8, pipp_short_filename.find_last_of(".") - start_org_filename_index - 8) + "_log.txt";
				input.open(pipp_log_org_filename, std::ios::binary);
				if (input) {
					(*pipp_info).log_exists = TRUE;
					break;
				}
			}
			start_org_filename_index++;
		}
		if (!(*pipp_info).log_exists) {
			(*pmessage) << "Ignoring PIPP file " << pipp_short_filename.c_str() << ", integrity unknown (log file " << pipp_log_filename.c_str() << " not found)\n";
			return FALSE; // No pipp log file
		}
	}
	(*pipp_info).log_exists = TRUE;
	std::string line;
	bool capture_file_next = FALSE;
	//int total_lines =			0;

	while (std::getline(input, line)) {
		if (line.find("-ctrf") != std::string::npos)				(*pipp_info).centered_frames		= TRUE;
		if (line.find("-f=") != std::string::npos)					(*pipp_info).max_nb_frames = stoi(line.substr(line.find_last_of("=") + 1, line.length() - line.find_last_of("=") - 2));
		if (line.find("-sf=") != std::string::npos)					(*pipp_info).start_frame = stoi(line.substr(line.find_last_of("=") + 1, line.length() - line.find_last_of("=") - 2));
		if (line.find("-ifd=") != std::string::npos)				(*pipp_info).input_frames_dropped	= stoi(line.substr(line.find_last_of("=") + 1, line.length() - line.find_last_of("=") - 2));
		if (line.find("-drop=") != std::string::npos)				(*pipp_info).output_frames_dropped	= stoi(line.substr(line.find_last_of("=") + 1, line.length() - line.find_last_of("=") - 2));
		if (line.find("-q") != std::string::npos) {
			(*pipp_info).quality = TRUE;
			if (isNumeric((line.substr(line.find_last_of("-q") + 2, line.length() - line.find_last_of("-q") - 3)))) {
				(*pipp_info).qlimit = TRUE;
				(*pipp_info).qlimit_frames = stoi(line.substr(line.find_last_of("-q") + 2, line.length() - line.find_last_of("-q") - 3));
			}
		}
		if (line.find("-nr") != std::string::npos)					(*pipp_info).qreorder = FALSE;
		if (line.find("-wjpos_time=") != std::string::npos)			(*pipp_info).winjpos_time = stoi(line.substr(line.find_last_of("=") + 1, line.length() - line.find_last_of("=") - 2));
		if (capture_file_next) {
			capture_file_next = FALSE;
			//strcpy((*pipp_info).capture_filename, line.c_str());
			trim(line.c_str(), (*pipp_info).capture_filename);
			std::ifstream CaptureFile((*pipp_info).capture_filename, std::ifstream::in);
			if (CaptureFile) (*pipp_info).capture_exists = TRUE;
			CaptureFile.close();
		}
		if (line.find("Source image file") != std::string::npos)	capture_file_next = TRUE;
		if (line.find("Total input") != std::string::npos) {
			(*pipp_info).total_input_frames = stoi(line.substr(line.find_last_of(":") + 1, line.length() - line.find_last_of(":") - 2));
			// exotic case of q limit value too high and not effective filtering
			if ((*pipp_info).qlimit == TRUE) {
				if ((*pipp_info).qlimit_frames >= ((*pipp_info).total_input_frames)) (*pipp_info).qlimit = FALSE;
				else if (((*pipp_info).max_nb_frames > 0) && ((*pipp_info).qlimit_frames >= ((*pipp_info).max_nb_frames)))		(*pipp_info).qlimit = FALSE;
			}
		}
		if (line.find("Total output") != std::string::npos) {
			std::string tmpline = line.substr(0, line.length() - 2); // bug PIPP: value followed by ":"
			(*pipp_info).total_output_frames = stoi(tmpline.substr(tmpline.find_last_of(":") + 1, tmpline.length() - tmpline.find_last_of(":") - 1));
		}
		if (line.find("Frames discarded by ") != std::string::npos) (*pipp_info).total_discarded_frames += stoi(line.substr(line.find_last_of(":") + 1, line.length() - line.find_last_of(":") - 2));
		if (line.find("Frames discarded for Input ") != std::string::npos) (*pipp_info).total_discarded_frames += stoi(line.substr(line.find_last_of(":") + 1, line.length() - line.find_last_of(":") - 2));
		// "Frames discarded with no planet detected:" during alignment
	}
	input.close();

	//(*pipp_info).total_input_frames += -(*pipp_info).start_frame + 1;
	(*pipp_info).total_frames = (*pipp_info).total_input_frames + (*pipp_info).total_discarded_frames;
/*	if (((*pipp_info).winjpos_time >= 0) && ((*pipp_info).winjpos_time <= 2)) {
		int start_org_filename_index = 0;

		// 0001-01-01-0000_0-jupiter_ch4_1_pipp
		// 0   45 78 01 3 567
		while (start_org_filename_index < pipp_short_filename.length() - 17 - 3) {
			if ((isNumeric(pipp_short_filename.substr(start_org_filename_index, 4)))
				&& (pipp_short_filename.substr(start_org_filename_index + 4, 1) == "-")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 5, 2)))
				&& (pipp_short_filename.substr(start_org_filename_index + 7, 1) == "-")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 8, 2)))
				&& (pipp_short_filename.substr(start_org_filename_index + 10, 1) == "-")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 11, 2)))
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 13, 2)))
				&& (pipp_short_filename.substr(start_org_filename_index + 15, 1) == "_")
				&& (isNumeric(pipp_short_filename.substr(start_org_filename_index + 16, 1)))
				&& (pipp_short_filename.substr(start_org_filename_index + 17, 1) == "-")) {
				int y = stoi(pipp_short_filename.substr(start_org_filename_index + 0, 4));
				int m = stoi(pipp_short_filename.substr(start_org_filename_index + 5, 2));
				int d = stoi(pipp_short_filename.substr(start_org_filename_index + 8, 2));
				int hour = stoi(pipp_short_filename.substr(start_org_filename_index + 11, 2));
				int min = stoi(pipp_short_filename.substr(start_org_filename_index + 13, 2));
				double sec = stoi(pipp_short_filename.substr(start_org_filename_index + 16, 1)) * 6.0;
				double jd = gregorian_calendar_to_jd(y, m, d, hour, min, sec);
				switch ((*pipp_info).winjpos_time) {
				case 0:
					(*pipp_info).mid_time = jd;
					break;
				case 1:
					(*pipp_info).start_time = jd;
					break;
				case 2:
					(*pipp_info).start_time = jd;
					break;
				}
			}
			start_org_filename_index++;
		}
	}*/

//Checks integrity if quality estimated
	if ((*pipp_info).quality) {
		if ((*pipp_info).qreorder) {
			(*pmessage) << "Ignoring PIPP file " << pipp_short_filename.c_str() << ", no integrity (frames reordered according to their quality)\n";
			return FALSE; // No intefrity because frames reordered
		}
		else if ((*pipp_info).qlimit) {
			(*pmessage) << "Ignoring PIPP file " << pipp_short_filename.c_str() << ", no integrity (frames filtered according to their quality)\n";
			return FALSE; // No intefrity because frames filtered
		}
	}
//if ((*pipp_info).total_output_frames != (*pipp_info).total_input_frames) {
	if (((*pipp_info).input_frames_dropped + (*pipp_info).output_frames_dropped)  > 0) {
		(*pmessage) << "Ignoring PIPP file " << pipp_short_filename.c_str() << ", no integrity (" << ((*pipp_info).input_frames_dropped + (*pipp_info).output_frames_dropped) << " frame";
		if (((*pipp_info).input_frames_dropped + (*pipp_info).output_frames_dropped) > 1) (*pmessage) << "s";
		(*pmessage) << " dropped)\n";
		return FALSE; // No intefrity because frames dropped from input or output
	}
/*	if ((*pipp_info).total_output_frames != (*pipp_info).total_frames) {
		(*pmessage) << "PIPP " << pipp_short_filename.c_str() << " date to be calculated, file has been cropped by " << (*pipp_info).total_frames << " frame";
		if ((*pipp_info).total_frames > 1) (*pmessage) << "s";
		(*pmessage) << "\n";
		return TRUE; // Cropped movie, time to be recalculated, check wjpos in case of PIPP renamming
	}*/
	return TRUE;
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
					lowercase_string(&line);
					std::string filename_acquisition_local = filename_acquisition;
					lowercase_string(&filename_acquisition_local);
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

