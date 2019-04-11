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