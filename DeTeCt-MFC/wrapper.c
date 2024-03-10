/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    WRAPPER: Different acquisitions format routing functions					*/
/*                                                                              */
/********************************************************************************/
//#include "common.h"

///#include <stdio.h>
//#include <ctype.h>

#include "wrapper.h"
//#include "serfmt.h"
//#include "dtc.h"
//#include "datation.h"

/**********************************************************************************************/
/******************************* Internal functions *******************************************/
/**********************************************************************************************/

//cv::VideoCapture* VideoCaptureFromFile(const char* filename); not used anymore

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

DtcCapture* dtcCaptureFromFile2(const char *fname, int* pframecount)
{
	char ext[EXT_MAX];
	DtcCapture* capt;

	get_fileextension(fname, ext, EXT_MAX);
	lcase(ext, ext);
	capt = (DtcCapture*)malloc(sizeof(DtcCapture));
	capt->framecount = 0;
	if (capt == NULL) {
		assert(capt != NULL);
	}
	else {
		/*capt->pCaptureInfo = (DtcCaptureInfo*)malloc(sizeof(DtcCaptureInfo));

		if (capt->pCaptureInfo == NULL) {
			assert(capt->pCaptureInfo != NULL);
		}*/
		//if (opts.debug) { fprintf(stderr, "!Debug info: dtcCaptureFromFile: capt @ %p\n", capt); }
		if (!strcmp(ext, "ser")) {
			capt->type = CAPTURE_SER;
			capt->u.sercapture = NULL;
			if (!(capt->u.sercapture = serCaptureFromFile(fname)))
			{
				free(capt);
				capt = NULL;
				OutputDebugString(L"ERROR in dtcCaptureFromFile processing ser file\n");
				//fprintf(stderr, "ERROR in dtcCaptureFromFile processing ser file %s\n", fname);
				//exit(EXIT_FAILURE);
				return NULL;
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
				char msgtext[MAX_STRING] = { 0 };
				snprintf(msgtext, MAX_STRING, "cannot read fits file %s\n", fname);
				Warning(WARNING_MESSAGE_BOX, "cannot read fits file", __func__, msgtext);
				//exit(EXIT_FAILURE);
				return NULL;
			}
			capt->framecount = (int)capt->u.filecapture->FrameCount;
		}
		else if ((!strcmp(ext, "bmp")) || (!strcmp(ext, "jpg")) || (!strcmp(ext, "jpeg")) || (!strcmp(ext, "jp2")) || (!strcmp(ext, "dib")) || (!strcmp(ext, "png")) || (!strcmp(ext, "pbm")) || (!strcmp(ext, "pgm")) || (!strcmp(ext, "ppm")) || (!strcmp(ext, "sr")) || (!strcmp(ext, "ras")) || (!strcmp(ext, "tif")) || (!strcmp(ext, "tiff"))) {
			capt->type = CAPTURE_FILES;
			capt->u.filecapture = NULL;
			if (!(capt->u.filecapture = FileCaptureFromFile(fname, pframecount, capt->type)))
			{
				free(capt);
				capt = NULL;
				char msgtext[MAX_STRING] = { 0 };
				char exttext[MAX_STRING] = { 0 };
				snprintf(msgtext, MAX_STRING, "cannot read %s file %s\n", ext, fname);
				snprintf(exttext, MAX_STRING, "cannot read %s file\n", ext);
				Warning(WARNING_MESSAGE_BOX, exttext, __func__, msgtext);
				//exit(EXIT_FAILURE);
				return NULL;
			}
			capt->framecount = (int)capt->u.filecapture->FrameCount;
		}
		else {
			capt->type = CAPTURE_CV;
			capt->u.videocapture = NULL;
			//capt->u.capture = NULL;
			//if (!(capt->u.capture = cvCreateFileCapture(fname, CV_LOAD_IMAGE_ANYCOLOR))) {
			/*capt->u.capture = cvCaptureFromFile(fname);
			if (capt->u.capture == NULL) {*/
			//if ((capt->u.capture = cvCaptureFromFile(fname)) != 0) {
//			if (!(capt->u.capture = cvCaptureFromFile(fname))) {
/*			if (!(capt->u.capture = cvCreateFileCapture(fname))) {
*/
				//if (!(capt->u.videocapture = cv::makePtr<cv::VideoCapture>(cv::VideoCapture(fname)))) {
				//free(capt->u.capture);
				// capt->u.capture=NULL;
				//capt->u.videocapture = cv::makePtr<cv::VideoCapture>(cv::VideoCapture(fname));
				//cv::VideoCapture NewVideo = cv::VideoCapture(fname);
				//capt->u.videocapture = VideoCaptureFromFile(fname);
			capt->u.videocapture = new cv::VideoCapture(fname, cv::CAP_ANY, {cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_ANY}); // cv::CAP_FFMPEG or cv::CAP_DSHOW or cv::CAP_ANY or cv::CAP_IMAGES
				//capt->u.videocapture->open(fname, cv::CAP_ANY);
				if (capt->u.videocapture->isOpened()) capt->framecount = (int)(dtcGetCaptureProperty(capt, cv::CAP_PROP_FRAME_COUNT));
				if ((!capt->u.videocapture->isOpened()) || (capt->framecount < 0)) {
					delete capt->u.videocapture;
					free(capt);
					capt = NULL;
					char msgtext[MAX_STRING] = { 0 };
					snprintf(msgtext, MAX_STRING, "cannot read capture file %s\n", fname);
					Warning(WARNING_MESSAGE_BOX, "cannot read capture file", __func__, msgtext);
					//exit(EXIT_FAILURE);
					return NULL;
				}
			//capt->u.capture = cvCreateFileCapture(fname);
		}
		(*pframecount) = capt->framecount;
		//initDtcCaptureInfo(capt->pCaptureInfo);
		initDtcCaptureInfo(&capt->CaptureInfo);
	}
	return capt;
}

/* Not used
cv::VideoCapture* VideoCaptureFromFile(const char* filename) {
	CString filename_cstring;
	char2CString(filename, &filename_cstring);

	cv::VideoCapture* pVideoCapture = new cv::VideoCapture(filename_cstring, cv::CAP_ANY, { cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_ANY });
	
	return pVideoCapture;
}
*/

double dtcGetCaptureProperty(DtcCapture *capture, int property_id)
{
	double val = 0.0;

	if (capture == NULL) return val;

	switch (capture->type)
	{
	case CAPTURE_SER:
		if (property_id == cv::CAP_PROP_FPS)
			val = DEFAULT_FPS;
		else
			val = 0.0;
		break;
	case CAPTURE_FITS:
	case CAPTURE_FILES:
		if (property_id == cv::CAP_PROP_FPS)
			val = DEFAULT_FPS;
		else
			val = 0.0;
		break;
	default: // CAPTURE_CV
		//val = cvGetCaptureProperty(capture->u.capture, property_id);
		
		val = capture->u.videocapture->get(property_id);
		if ((property_id == cv::CAP_PROP_FRAME_COUNT) && (val == 0)) {
			int			total_frames = 0;
			cv::UMat	matrix_frame;					// matrix used to contain frame data //was cv::Mat

			while (capture->u.videocapture->read(matrix_frame)) total_frames++;
			capture->u.videocapture->set(cv::CAP_PROP_POS_FRAMES, 0);
			val = total_frames;
		}
	}
	return val;
}


void		initDtcCaptureInfo(DtcCaptureInfo* pCaptureInfo)
{
	init_string(pCaptureInfo->observer);
	init_string(pCaptureInfo->location);
	init_string(pCaptureInfo->scope);
	init_string(pCaptureInfo->camera);
	init_string(pCaptureInfo->filter);
	init_string(pCaptureInfo->profile);
	pCaptureInfo->diameter_arcsec	= -DBL_MAX;
	pCaptureInfo->magnitude			= -DBL_MAX;
	init_string(pCaptureInfo->centralmeridian);
	pCaptureInfo->focallength_mm	= INT_MIN;
	pCaptureInfo->resolution		= INT_MIN;
	init_string(pCaptureInfo->binning);
	pCaptureInfo->bitdepth			= INT_MIN;
	pCaptureInfo->debayer			= NotSet;
	pCaptureInfo->exposure_ms		= -DBL_MAX;
	pCaptureInfo->gain				= INT_MIN;
	pCaptureInfo->gamma				= INT_MIN;
	pCaptureInfo->autoexposure		= NotSet;
	pCaptureInfo->softwaregain		= INT_MIN;
	pCaptureInfo->autohisto			= INT_MIN;
	pCaptureInfo->brightness		= INT_MIN;
	pCaptureInfo->autogain			= NotSet;
	pCaptureInfo->histmin			= INT_MIN;
	pCaptureInfo->histmax			= INT_MIN;
	pCaptureInfo->histavg_pc		= INT_MIN;
	pCaptureInfo->noise				= -DBL_MAX;
	init_string(pCaptureInfo->prefilter);
	pCaptureInfo->temp_C			= -DBL_MAX;
	init_string(pCaptureInfo->target);
}

void		initPIPPInfo(PIPPInfo* pipp_info) {
	(*pipp_info).isPIPP					= FALSE;
	(*pipp_info).log_exists				= FALSE;
	init_string((*pipp_info).capture_filename);
	(*pipp_info).capture_exists			= FALSE;
	(*pipp_info).centered_frames		= FALSE;
	(*pipp_info).start_frame			= 1;
	(*pipp_info).max_nb_frames			= 0;
	(*pipp_info).input_frames_dropped	= 0;
	(*pipp_info).output_frames_dropped	= 0;
	(*pipp_info).winjpos_time			= -1;	// no PIPP datation
	(*pipp_info).quality				= FALSE;
	(*pipp_info).qlimit					= FALSE;
	(*pipp_info).qlimit_frames			= 0;
	(*pipp_info).qreorder				= TRUE; // PIPP logs no quality reorder option
	(*pipp_info).total_frames			= 0;
	(*pipp_info).total_input_frames		= 0;
	(*pipp_info).total_output_frames	= 0;
	(*pipp_info).total_discarded_frames = 0;
//	(*pipp_info).start_time				= gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
//	(*pipp_info).mid_time				= gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
}