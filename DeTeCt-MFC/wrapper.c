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

DtcCapture *dtcCaptureFromFile(const char *fname, int *pframecount)
{
	char ext[EXT_MAX];
	DtcCapture *capt;

	get_fileextension(fname, ext, EXT_MAX);
	lcase(ext,ext);
	capt = (DtcCapture *) malloc(sizeof (DtcCapture));
	if (capt == NULL) {
		assert(capt != NULL);
	} else {
		if (opts.debug) { fprintf(stderr, "dtcCaptureFromFile: capt @ %d\n", (int)(capt)); }

		if (!strcmp(ext, "ser")) {
			capt->type = CAPTURE_SER;
			if (!(capt->u.sercapture = serCaptureFromFile(fname)))
			{
				free(capt);
				capt = NULL;
				fprintf(stderr, "ERROR in dtcCaptureFromFile processing ser file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			capt->framecount = capt->u.sercapture->header.FrameCount;
		}
		else if ((!strcmp(ext, "fit")) || (!strcmp(ext, "fits")))	{
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
//			if (!(capt->u.capture = cvCaptureFromFile(fname, CV_CAP_OPENNI_GRAY_IMAGE))) {
			if (!(capt->u.capture = cvCaptureFromFile(fname))) {
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

void dtcReinitCaptureRead(DtcCapture **pcapture,const char *fname)
{
	int framecount;
	
	switch ((*pcapture)->type)
	{
		case CAPTURE_SER:
			serReinitCaptureRead((*pcapture)->u.sercapture,fname);
			break;
		case CAPTURE_FITS:
		case CAPTURE_FILES:
			fileReinitCaptureRead((*pcapture)->u.filecapture,fname);
			break;
		default: // CAPTURE_CV			
			dtcReleaseCapture(*pcapture);
			if (!(*pcapture = dtcCaptureFromFile(fname, &framecount))) {
				fprintf(stderr, "ERROR in dtcReinitCaptureRead opening file %s\n", fname);
				exit(EXIT_FAILURE);
			}
			break;
	}
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
}

IplImage *dtcQueryFrame(DtcCapture *capture, const int ignore, int *perror)
{
	(*perror)=0;
	switch (capture->type)
	{
		case CAPTURE_SER:
			return serQueryFrame(capture->u.sercapture, ignore, perror);
			break;
		case CAPTURE_FITS:
		case CAPTURE_FILES:
			return fileQueryFrame(capture->u.filecapture, ignore, perror);
			break;
		default: // CAPTURE_CV
			return cvQueryFrame(capture->u.capture);
	}	
}

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