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

#include "wrapper.h"
#include "serfmt.h"
#include "dtc.h"

double dtcGetCaptureProperty(DtcCapture *capture, int property_id)
{
	double val;

	switch (capture->type)
	{
	case CAPTURE_SER:
		if (property_id == CV_CAP_PROP_FPS)
			val = DEFAULT_FPS;
		else
			val = 0.0;
		break;
	case CAPTURE_FITS:
	case CAPTURE_FILES:
		if (property_id == CV_CAP_PROP_FPS)
			val = DEFAULT_FPS;
		else
			val = 0.0;
		break;
	default: // CAPTURE_CV
		val = cvGetCaptureProperty(capture->u.capture, property_id);
		//capture->u.videocapture->get(property_id);
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
	pCaptureInfo->binning			= NotSet;
	pCaptureInfo->bitdepth			= INT_MIN;
	pCaptureInfo->debayer			= NotSet;
	pCaptureInfo->shutter_ms		= -DBL_MAX;
	pCaptureInfo->gain				= INT_MIN;
	pCaptureInfo->gamma				= INT_MIN;
	pCaptureInfo->autoexposure		= NotSet;
	pCaptureInfo->softwaregain		= INT_MIN;
	pCaptureInfo->autohisto			= INT_MIN;;
	pCaptureInfo->brightness		= INT_MIN;;
	pCaptureInfo->autogain			= NotSet;
	pCaptureInfo->histmin			= INT_MIN;
	pCaptureInfo->histmax			= INT_MIN;
	pCaptureInfo->histavg_pc		= INT_MIN;
	pCaptureInfo->noise				= -DBL_MAX;
	init_string(pCaptureInfo->prefilter);
	pCaptureInfo->temp_C			= -DBL_MAX;
	init_string(pCaptureInfo->target);
}