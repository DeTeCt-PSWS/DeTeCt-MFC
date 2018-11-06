/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012-			*/
/*                                                                              */
/*    IMG: Frame images analysis and handling functions							*/
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdio.h>

#include "img.h"
#include "cmdline.h"
#include "wrapper.h"

static void dtcWriteFrame(CvVideoWriter *writer, IplImage *img);
int doublecmp(const void *a, const void *b);
void printtbuf(double *uc, size_t s);

void dtcGetROI(IplImage *img, int *xorig, int *yorig, int *width, int *height)
{
	if (img->roi) {
		*xorig  = img->roi->xOffset;
		*yorig  = img->roi->yOffset;
		*width  = img->roi->width;
		*height = img->roi->height;
	} else {
		*xorig  = 0;
		*yorig  = 0;
		*width  = img->width;
		*height = img->height;
	}
}

CvPoint dtcGetCM(IplImage *img)
{
	double xcm = 0, ycm = 0, Y = 0, l;
	uchar r, g, b;
	
	int xorig  = 0;
	int yorig  = 0;
	int width  = img->width;
	int height = img->height;
	int x, y;
	
	for (y = yorig; y < height; y++) {
		uchar *ptr = (uchar*) (img->imageData + y * img->widthStep);
		for (x = xorig; x < width; x++) {
			r = *ptr++;
			g = *ptr++;
			b = *ptr++;
			if (img->nChannels == 4) ptr++;
			Y += (l = KR * r + KG * g + KB * b);
			xcm += x * l;
			ycm += y * l;
		}
	}

	return cvPoint((int) floor(xcm / Y), (int) floor(ycm / Y));
}

CvPoint dtcGetGrayCM(IplImage *img)
{
	double xcm = 0, ycm = 0, Y = 0;
	
	int xorig  = 0;
	int yorig  = 0;
	int width  = img->width;
	int height = img->height;
	int x, y;
	
	for (y = yorig; y < height; y++) {
		uchar *ptr = (uchar*) (img->imageData + y * img->widthStep);
		for (x = xorig; x < width; x++) {
			xcm += x * (*ptr);
			ycm += y * (*ptr);
			Y += *ptr++;
		}
	}

	return cvPoint((int) floor(xcm / Y), (int) floor(ycm / Y));
}

CvPoint dtcGetGrayMatCM(CvMat *mat)
{
	double xcm = 0.0;
	double ycm = 0.0;
	double   Y = 0.0;
	float *ptr;
	
	int xorig  = 0;
	int yorig  = 0;
	int width  = mat->cols;
	int height = mat->rows;
	int x, y;
	int step;
	
	step  = mat->step/sizeof(float);

	for (y = yorig; y < height; y++)
	{
		ptr = (mat->data.fl + y*step);
		for (ptr += (x = xorig); x < width; x++)
		{
			xcm += x * (*ptr);
			ycm += y * (*ptr);
			Y += *ptr++;
		}
	}

	return cvPoint((int) floor(xcm / Y), (int) floor(ycm / Y));
}

CvRect dtcGetImageROIcCM(IplImage *img, CvPoint cm, int medsize, double fact, double secfact)
{
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf	= NULL;
	double *mbuf	= NULL;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	const double kr = 0.299;
	const double kg = 0.587;
	const double kb = 0.114;

	int xorig  = 0;
	int yorig  = 0;
	int width  = img->width;
	int height = img->height;

	if ((tbuf = (double*) calloc(medsize, sizeof (double))) == NULL ||
	   (mbuf = (double*) calloc(MAX(width, height), sizeof (double))) == NULL) {
			perror("dtcGetImageROI");
			abort();
	}
	assert(tbuf != NULL);
	assert(mbuf != NULL);

	posmed = (int) floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;
	
	// Horizontal line
	for (y = cm.y,
	     src = (uchar *) img->imageData + y * img->widthStep
	                                    + xorig * img->nChannels,
	     x = xorig;
	     x < width - medsize;
	     x++, src += img->nChannels)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc += img->nChannels) {
				tbuf[i] = kr * tsrc[0] + kg * tsrc[1] + kb * tsrc[2];
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[x] = tbuf[posmed];
			if (mbuf[x] < mbuf[xmin]) xmin = x;
			if (mbuf[x] > mbuf[xmax]) xmax = x;
	}

	val = mbuf[xmax] - fact*(mbuf[xmax]-mbuf[xmin]);
	for (i = 0; mbuf[i] < val; i++)
		;
	for (j = width-1; mbuf[j] < val; j--)
		;
	hwd = (int) floor((j - i) * secfact);

	// Vertical line
	for (x = cm.x, y = yorig,
	     src = (uchar *) img->imageData + x * img->nChannels
	                                    + y * img->widthStep; 
	     y < height - medsize;
	     y++, src += img->widthStep)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc += img->widthStep) {
				tbuf[i] = kr * tsrc[0] + kg * tsrc[1] + kb * tsrc[2];
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[y] = tbuf[posmed]; 
			if (mbuf[y] < mbuf[ymin]) ymin = y;
			if (mbuf[y] > mbuf[ymax]) ymax = y;

	}
	val = mbuf[ymax] - fact*(mbuf[ymax]-mbuf[ymin]);
	for (i = 0; mbuf[i] < val; i++)
		;
	for (j = height-1; mbuf[j] < val; j--)
		;
	hht = (int) floor((j - i) * secfact);

	free(tbuf);
	tbuf=NULL;
	free(mbuf);
	mbuf=NULL;
	
	return cvRect(cm.x-hwd/2, cm.y-hht/2, hwd, hht);
}

CvRect dtcGetGrayImageROIcCM(IplImage *img, CvPoint cm, int medsize, double fact, double secfact)
{
	uchar *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig  = 0;
	int yorig  = 0;
	int width  = img->width;
	int height = img->height;

	if ((tbuf = (double*) calloc(medsize, sizeof (double))) == NULL ||
	   (mbuf = (double*) calloc(MAX(width, height), sizeof (double))) == NULL) {
			perror("ERROR in dtcGetGrayImageROI allocating memory");
			exit(EXIT_FAILURE);
	} else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int) floor(medsize / 2);
	xmin = ymin = xmax = ymax = 0;
	
	// Horizontal
	for (y = cm.y,
	     src = (uchar *) img->imageData + y * img->widthStep
	                                    + xorig * img->nChannels,
	     x = xorig;
	     x < (width - medsize);
	     x++, src += img->nChannels)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc += img->nChannels) {
				tbuf[i] = tsrc[0];
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[x] = tbuf[posmed];
			if (mbuf[x] < mbuf[xmin]) xmin = x;
			if (mbuf[x] > mbuf[xmax]) xmax = x;
	}
	val = mbuf[xmax] - fact*(mbuf[xmax]-mbuf[xmin]);
	for (i = 0; i < (width - medsize) && mbuf[i] < val; i++)
		;
	for (j = (width-1); j >= 0 && mbuf[j] < val; j--)
		;
	hwd = (int) floor((j - i) * secfact);

	// Vertical
	for (x = cm.x, y = yorig,
	     src = (uchar *) img->imageData + x * img->nChannels
	                                    + y * img->widthStep; 
	     y < (height - medsize);
	     y++, src += img->widthStep)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc += img->widthStep)
			{
				tbuf[i] = tsrc[0];
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[y] = tbuf[posmed]; 
			if (mbuf[y] < mbuf[ymin]) ymin = y;
			if (mbuf[y] > mbuf[ymax]) ymax = y;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax]-mbuf[ymin]);

	for (i = 0; i < (height - medsize) && mbuf[i] < val; i++)
		;
	for (j = height-1; j >= 0 && mbuf[j] < val; j--)
		;
	hht = (int) floor((j - i) * secfact);

	free(tbuf);
	tbuf=NULL;
	free(mbuf);
	mbuf=NULL;
	
	return cvRect(cm.x-hwd/2, cm.y-hht/2, hwd, hht);
}

CvRect dtcGetGrayMatROIcCM(CvMat *img, CvPoint cm, int medsize, double fact, double secfact)
{
	float *src, *tsrc;
	int x, y, i, j;
	double *tbuf;
	double *mbuf;
	int posmed;
	int xmin, xmax, ymin, ymax;
	double val;
	int hwd, hht;

	int xorig  = 0;
	int yorig  = 0;
	int width  = img->cols;
	int height = img->rows;
	int step   = img->step/sizeof (float);

	if ((tbuf = (double *) calloc(medsize, sizeof (double))) == NULL ||
	   (mbuf = (double *) calloc(MAX(width, height), sizeof (double))) == NULL) {
			perror("EROOR in dtcGetGrayMatImageROI allocating memory");
			exit(EXIT_FAILURE);
	} else {
		assert(tbuf != NULL);
		assert(mbuf != NULL);
	}

	posmed = (int) floor(medsize/2);
	xmin = ymin = xmax = ymax = 0;
	
	// Horizontal
	y   = cm.y; 
	src = (float *) img->data.fl + y*step + xorig;
	for (x = xorig; x < width-medsize; x++)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc++)
			{
				tbuf[i] = *tsrc;
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[x] = tbuf[posmed];
			if (mbuf[x] < mbuf[xmin]) xmin = x;
			if (mbuf[x] > mbuf[xmax]) xmax = x;
			src++;
	}

	val = mbuf[xmax] - fact*(mbuf[xmax]-mbuf[xmin]);
	for (i = 0; i < width-medsize && mbuf[i] < val; i++)
		;
	for (j = width-medsize-1; j >= 0 && mbuf[j] < val; j--)
		;
	hwd = (int) floor((j - i) * secfact);

	// Vertical
	x   = cm.x;
	src = (float *) img->data.fl + x + yorig*step; 	     
	for (y = yorig; y < height-medsize; y++)
	{
			for (i = 0, tsrc = src; i < medsize; i++, tsrc += step)
			{
				tbuf[i] = *tsrc;
			}

			qsort(tbuf, medsize, sizeof (double), doublecmp);
			mbuf[y] = tbuf[posmed]; 
			if (mbuf[y] < mbuf[ymin]) ymin = y;
			if (mbuf[y] > mbuf[ymax]) ymax = y;
			src += step;
	}
	val = mbuf[ymax] - fact*(mbuf[ymax]-mbuf[ymin]);

	for (i = 0; i < height-medsize && mbuf[i] < val; i++)
		;
	for (j = height-medsize-1; j >= 0 && mbuf[j] < val; j--)
		;
	hht = (int) floor((j - i) * secfact);

	free(tbuf);
	tbuf=NULL;
	free(mbuf);
	mbuf=NULL;
	
	return cvRect(cm.x-hwd/2, cm.y-hht/2, hwd, hht);
}

IplImage *dtcLumThreshold_ToZero(IplImage *imgsrc, IplImage *imgdst, double threshold)
{
	int xorig, yorig, width, height;
	int i, j;
	uchar *src;
	uchar *srcx;
	
	dtcGetROI(imgsrc, &xorig, &yorig, &width, &height);

	src = (uchar *) (imgsrc->imageData + xorig * imgsrc->nChannels + yorig * imgsrc->widthStep);
	for (i = 0; i < height; i++, src += imgsrc->widthStep)
	{
		srcx = src;
		for (j = 0; j < width; j++, srcx += imgsrc->nChannels)
		{
			if (threshold > KR * srcx[0] + KG * srcx[1] + KB * srcx[2])
			{
				uchar *dst = (uchar *) imgdst->imageData + (srcx - (uchar *) imgsrc->imageData);
				dst[0] = dst[1] = dst[2] = 0;
			}
		}
	}

	return imgsrc;
}

double dtcGetImageLum(IplImage *img)
{
    int xorig, yorig, width, height;
	double lum = 0.0;
	double Y;
	int i, j;
    uchar *src;
	uchar *srcx;

	if (img->roi) {
		xorig  = img->roi->xOffset;
		yorig  = img->roi->yOffset;
		width  = img->roi->width;
		height = img->roi->height;
	} else {
		xorig  = 0;
		yorig  = 0;
		width  = img->width;
		height = img->height;
	}

	src = (uchar *) (img->imageData + xorig * img->nChannels + yorig * img->widthStep);
	for (i = 0; i < height; i++, src += img->widthStep) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img->nChannels) {
			Y = KR * srcx[0] + KG * srcx[1] + KB * srcx[2];
			if (Y < opts.threshold) Y = 0.0;
			lum += Y;
		}
	}

	return lum / (height * width);
}

DtcImageVals dtcGetImageVals(IplImage *img)
{
    int xorig, yorig, width, height;
    double lum;
    DtcImageVals vals = { 0.0, 256.0, 0.0 };
	int i, j;
	uchar *srcx;
	uchar *src;

	if (img->roi) {
		xorig  = img->roi->xOffset;
		yorig  = img->roi->yOffset;
		width  = img->roi->width;
		height = img->roi->height;
	} else {
		xorig  = 0;
		yorig  = 0;
		width  = img->width;
		height = img->height;
	}

	src = (uchar *) (img->imageData + xorig * img->nChannels + yorig * img->widthStep);
	for (i = 0; i < height; i++, src += img->widthStep) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img->nChannels) {
			lum = KR * srcx[0] + KG * srcx[1] + KB * srcx[2];
			if (vals.maxlum < lum) vals.maxlum = lum;
			if (vals.minlum > lum) vals.minlum = lum;
			vals.lum += lum;
		}
	}

	vals.lum /= width * height;
	
	return vals;
}

DtcImageVals dtcGetGrayImageVals(IplImage *img)
{
    int xorig, yorig, width, height;
    double lum;
    DtcImageVals vals = { 0.0, 256.0, 0.0 };
	int i, j;
	uchar *src;
	uchar *srcx;

	if (img->roi) {
		xorig  = img->roi->xOffset;
		yorig  = img->roi->yOffset;
		width  = img->roi->width;
		height = img->roi->height;
	} else {
		xorig  = 0;
		yorig  = 0;
		width  = img->width;
		height = img->height;
	}

	src = (uchar *) (img->imageData + xorig * img->nChannels + yorig * img->widthStep);
	for (i = 0; i < height; i++, src += img->widthStep) {
		srcx = src;
		for (j = 0; j < width; j++, srcx += img->nChannels) {
			lum = srcx[0];
			if (vals.maxlum < lum) vals.maxlum = lum;
			if (vals.minlum > lum) vals.minlum = lum;
			vals.lum += lum;
		}
	}

	vals.lum /= width * height;
	
	return vals;
}

CvRect dtcMaxRect(CvRect one, CvRect two)
{
	int x, y, w, h;
	
	x = one.width  > two.width  ? one.x      : two.x;
	y = one.height > two.height ? one.y      : two.y;
	w = one.width  > two.width  ? one.width  : two.width;
	h = one.height > two.height ? one.height : two.height;
	
	return cvRect(x, y, w, h);
}

IplImage *dtcGetGray(IplImage *frame)
{
	IplImage *gray = NULL;
	
	if (!frame) return NULL;

	if (frame->colorModel[0] == 'R' && frame->colorModel[1] == 'G' && frame->colorModel[2] == 'B')
	{
		gray = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 1);
		cvCvtColor(frame, gray, CV_BGR2GRAY);
    	return gray;
	}
	else if (frame->colorModel[0] == 'G' && frame->colorModel[1] == 'R' && frame->colorModel[2] == 'A' && frame->colorModel[3] == 'Y')
	{
    	if (opts.bayer>=0) {
			IplImage *debayered = NULL;

			debayered = cvCreateImage(cvGetSize(frame), IPL_DEPTH_8U, 3);
			cvCvtColor(frame, debayered, opts.bayer);
			cvCvtColor(debayered, gray, CV_BGR2GRAY);
    		cvReleaseImage(&debayered);
		} else {
			gray = cvCloneImage(frame);
		}
		return gray;
	}
	else
	{
		printf("ERROR in dtcGetGray: Format %c%c%c%c not supported.\n", frame->colorModel[0], frame->colorModel[1],
		                                           frame->colorModel[2], frame->colorModel[3]);
		exit(EXIT_FAILURE);
	}
}

CvMat *dtcGetGrayMat(IplImage *frame)
{
	CvMat *gray = NULL;
	IplImage *gryi = NULL;

	if (!frame) return NULL;
							if (opts.debug) { fprintf(stderr, "dtcGetGrayMat:\n"); }	
	gray = cvCreateMat(frame->height, frame->width, CV_32FC1);
							if (opts.debug) { fprintf(stderr, "dtcGetGrayMat:\n"); }	
	
	if (frame->colorModel[0] == 'R' && frame->colorModel[1] == 'G' && frame->colorModel[2] == 'B')	{
							if (opts.debug) { fprintf(stderr, "dtcGetGrayMat:\n"); }	
		gryi = cvCreateImage(cvGetSize(frame), frame->depth, 1);
		cvCvtColor(frame, gryi, CV_BGR2GRAY);
		cvConvertScale(gryi, gray, 1, 0);
	   	cvReleaseImage(&gryi);
		gryi=NULL;
	}	else if (frame->colorModel[0] == 'G' && frame->colorModel[1] == 'R' && frame->colorModel[2] == 'A' && frame->colorModel[3] == 'Y')	{
							if (opts.debug) { fprintf(stderr, "dtcGetGrayMat: B&W, %d\n",opts.bayer); }	
    	if (opts.bayer>=0) {
			IplImage *debayered = NULL;
	
			debayered = cvCreateImage(cvGetSize(frame), frame->depth, 3);
			cvCvtColor(frame, debayered, opts.bayer);
			gryi = cvCreateImage(cvGetSize(frame), frame->depth, 1);
			cvCvtColor(debayered, gryi, CV_BGR2GRAY);
			cvConvertScale(gryi, gray, 1, 0);
			cvReleaseImage(&gryi);
			gryi=NULL;
		   	cvReleaseImage(&debayered);
			gryi=NULL;
		} else {
			cvConvertScale(frame, gray, 1, 0);
		}
							if (opts.debug) { fprintf(stderr, "dtcGetGrayMat: B&W\n"); }	
	}	else	{
		printf("ERROR in dtcGetGrayMat: Format %c%c%c%c not supported\n", frame->colorModel[0], frame->colorModel[1],
		                                           frame->colorModel[2], frame->colorModel[3]);
		cvReleaseMat(&gray);
		gray=NULL;
		
		exit(EXIT_FAILURE);
	}
   	
	return gray;
}

IplImage *dtcReduceToROI(IplImage **src, CvRect roi)
{
	IplImage *dst;
	
	dst = cvCreateImage(cvSize(roi.width, roi.height), (*src)->depth, (*src)->nChannels);
	cvSetImageROI(*src, roi);
	cvCopy(*src, dst, NULL);
	cvResetImageROI(*src);
	cvReleaseImage(src);
	
	return *src = dst;
}

CvMat *dtcReduceMatToROI(CvMat **src, CvRect roi)
{
	CvMat *dst;
	float *psrc, *pdst;
	int xorig = roi.x;
	int yorig = roi.y;
	int width = xorig + roi.width;
	int height = yorig + roi.height;
	int x1, x2, x3;
	int sstep;
	int roiwidth;
	int minheight;
	int maxheight;

	int debug = 0;

	roiwidth = roi.width;
	dst = cvCreateMat(roi.height, roi.width, CV_32FC1);
	cvSet(dst, cvScalarAll(0.0), NULL);

	sstep = (*src)->step / sizeof(float);
	pdst = dst->data.fl;
	if (debug) {
		fprintf(stderr, "dtcReduceMatToROI: InitMatROI %4d %4d %4d %4d\n", xorig, yorig, roi.height, roi.width);
		fprintf(stderr, "dtcReduceMatToROI: CopyROIptr sstep=%4d CV_32FC1=%16d h=%4d w=%4d\n", sstep, CV_32FC1, height, width);
		/*								fprintf(stderr, "dtcReduceMatToROI: (psrc,delta_start,pdst) %16d %16d %16d\n", (*src)->data.fl, yorig*sstep+xorig, pdst); */
	}
	psrc = (*src)->data.fl;

	x1 = max(xorig, 0);
	x2 = min(width, (*src)->cols);
	x3 = max(width, (*src)->cols);
	for (int y = yorig; y < 0; y++) {
		for (int x = 0; x < roiwidth; x++) {
			*pdst++ = 0;
		}
	}
	minheight = min(height, (*src)->rows);
	for (int y = max(yorig, 0); y < minheight; y++) {
		psrc = (*src)->data.fl + y*sstep;
		for (int x = xorig; x < 0; x++) {
			*pdst++ = 0;
		}
		for (int x = 0; x < x1; x++) {
			(void)*psrc++;
		}
		for (int x = x1; x < x2; x++) {
			*pdst++ = *psrc++;
		}
		for (int x = (*src)->cols; x < x3; x++) {
			*pdst++ = 0;
		}
	}
	maxheight = max(height, (*src)->rows);
	for (int y = (*src)->rows; y < maxheight; y++) {
		for (int x = 0; x < roiwidth; x++) {
			*pdst++ = 0;
		}
	}

	cvReleaseMat(src);

	return *src = dst;
	
	/*	CvMat *dst;
	float *psrc, *pdst;
	int xorig = roi.x;
	int yorig = roi.y;
	int width = xorig + roi.width;
	int height = yorig + roi.height;
	int x, y;
	int sstep, dstep;

	dst = cvCreateMat(roi.height, roi.width, CV_32FC1);
	cvSet(dst, cvScalarAll(0.0), NULL);

	sstep = (*src)->step / sizeof(float);
	pdst = dst->data.fl;
	for (y = yorig; y < height; y++)
	{
		psrc = (*src)->data.fl + y*sstep;
		for (psrc += (x = xorig); x < width; x++)
		{
			*pdst++ = *psrc++;
		}
		psrc += dst->step;
	}

	cvReleaseMat(src);

	return *src = dst; */
}

CvRect dtcGetFileROIcCM(DtcCapture *pcapture, const int ignore)
{
	int error = 0;
	CvRect win, roi = cvRect(0, 0, 0, 0); 
    CvMat *gray;
	IplImage *frame;

	unsigned long nframe;
	for (nframe = 1; !opts.nframesROI || nframe <= opts.nframesROI; nframe++)
	{
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: frame %d\n", (int) (nframe)); }
		error=0;
        frame = dtcQueryFrame(pcapture, ignore, &error);
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: frame %d\n", (int) (nframe)); }
        if (!frame) break;
		if (error==0) { 
			CvPoint cm;

							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: dtcGetGrayMat frame %d\n", (int) (nframe)); }
			gray = dtcGetGrayMat(frame);
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: dtcGetGrayMatCM frame %d\n", (int) (nframe)); }
			cm = dtcGetGrayMatCM(gray);
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: dtcGetGrayMatROIcCM frame %d\n", (int) (nframe)); }
			win = dtcGetGrayMatROIcCM(gray, cm, (int) opts.medSize, opts.facSize, opts.secSize);
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: dtcMaxRect frame %d\n", (int) (nframe)); }
			roi = dtcMaxRect(win, roi);
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: cvReleaseMat frame %d\n", (int) (nframe)); }
			cvReleaseMat(&gray);
			gray=NULL;
							if (opts.debug) { fprintf(stderr, "dtcGetFileROIcCM: frame %d\n", (int) (nframe)); }
		}
    }
    
    return roi;
}

void dtcDrawCM(IplImage *image, CvPoint cm)
{
	int x = cm.x - (image->roi ? image->roi->xOffset : 0);
	int y = cm.y - (image->roi ? image->roi->yOffset : 0);
	
	cvLine(image, cvPoint(x,y), cvPoint(x,y), CV_RGB(255,0,0), 1, 8, 0);
	cvLine(image, cvPoint(x-25,y), cvPoint(x-5,y), CV_RGB(255,0,0), 1, 8, 0);
	cvLine(image, cvPoint(x+5,y), cvPoint(x+25,y), CV_RGB(255,0,0), 1, 8, 0);
	cvLine(image, cvPoint(x,y-25), cvPoint(x,y-5), CV_RGB(255,0,0), 1, 8, 0);
	cvLine(image, cvPoint(x,y+5), cvPoint(x,y+25), CV_RGB(255,0,0), 1, 8, 0);
}

IplImage *dtcRunningAvg(IplImage *imgsrc, IplImage *imgdst, double lR)
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
	nChannels = imgsrc->nChannels;
	
	src = (uchar *) (imgsrc->imageData + x_srco * imgsrc->nChannels + y_srco * imgsrc->widthStep);
	dst = (uchar *) (imgdst->imageData + x_dsto * imgdst->nChannels + y_dsto * imgdst->widthStep);
	for (int i = 0; i < height; i++, src += imgsrc->widthStep, dst += imgdst->widthStep)
	{
		srcx = src;
		dstx = dst;
		for (int j = 0; j < width; j++)
		{
			for (int k = 0; k < nChannels; k++)
			{
				dstx[k] = (uchar) (lR * srcx[k] + (1.0 - lR) * dstx[k]);
				++srcx;
				++dstx;
			}
		}
	}
		
	return imgdst;
}

IplImage *dtcGetHistogramImage(CvArr *src, int scale, double thr)
{
	CvHistogram *pHis;
	IplImage *pHisImg;
	const int hsize = 256;
	const int vsize = 300;
	int phsize[] = {hsize};
	float range[] = {0 , (float) hsize};
	float *ranges[] = {range};
	int i;
	double bval;
	float max_val = 0.0;
	
	pHisImg = cvCreateImage(cvSize(hsize*scale, vsize*scale), 8, 3);
	pHis = cvCreateHist(1, phsize, CV_HIST_ARRAY, ranges, 1);
	cvCalcHist((IplImage **) &src, pHis, 0, NULL);
	if (thr) {
		cvSet1D(pHis->bins, 0, cvScalarAll(0.0));
	}
	
	cvGetMinMaxHistValue(pHis, 0, &max_val, 0, 0);
    cvSet(pHisImg, cvScalar(255, 255, 255, 0), NULL);

	if (max_val > 0)
	{
		for (i = 0; i < hsize; i++)
    	{
                bval = cvGetReal1D(pHis->bins, i);
                cvRectangle(pHisImg, cvPoint(i*scale,
                            (vsize-cvRound(bval*hsize/max_val))*scale),
                            cvPoint((i+1)*scale - 1, vsize*scale),
                            CV_RGB(0, 0, 0), CV_FILLED, 8, 0);
        	}     
	}
   
	cvReleaseHist(&pHis);
	pHis=NULL;

/*	free(*ranges);*/
	return pHisImg;
}

void dtcWriteFrame(CvVideoWriter *writer, IplImage *img)
{
	IplImage *tmp;
	char *src, *dst;
	
	if (img->width * img->nChannels == img->widthStep)
		cvWriteFrame(writer, img);
	else
	{
		tmp = cvCreateImage(cvGetSize(img), img->depth, img->nChannels);
		tmp->origin = img->origin;
		tmp->align  = img->align;
		
		src = img->imageData;
		dst = tmp->imageData;
		for (int i = 0; i < tmp->height; i++) {
			src = img->imageData + i * img->widthStep;
			for (int j = 0; j < (tmp->width * img->nChannels); j++) {
				*dst++ = *src++;
			}
		}
		cvWriteFrame(writer, tmp);
		cvReleaseImage(&tmp);
		tmp=NULL;
	}
}

CvVideoWriter *dtcWriteVideo(const char *file, CvVideoWriter *writer, DtcCapture *capture, IplImage *img)
{
	IplImage *color;
	double fps;
	CvSize size;
	
	if (!img) return NULL;
	
	if (!writer)
	{
		fps  = dtcGetCaptureProperty(capture, CV_CAP_PROP_FPS);
		size = cvSize(img->width, img->height);
		writer = cvCreateVideoWriter(file,
#if defined(_WIN32)
			CV_FOURCC('D', 'I', 'B', ' '),
#else
			CV_FOURCC('M', 'J', 'P', 'G'),
#endif
			fps, size, CV_LOAD_IMAGE_COLOR);
	}
	
	if (writer)
	{
		if (img->nChannels < 3)
		{
			color = cvCreateImage(cvGetSize(img), IPL_DEPTH_8U, 3);
			cvCvtColor(img, color, CV_GRAY2BGR);
		}
		else
		{
			color = img;
		}

		dtcWriteFrame(writer, color);
		if (img->nChannels < 3)
			cvReleaseImage(&color);
			color=NULL;
	}
	
	return writer;
}

void dtcShowPhotometry(CvMat *pmat, int nframe)
{	CvScalar lum;
	CvPoint minPoint;
	CvPoint maxPoint;
	double minLum;
	double maxLum;
	
	lum = cvSum(pmat);
	cvMinMaxLoc(pmat, &minLum, &maxLum, &minPoint, &maxPoint, NULL);
   	printf("%03d   %10.6f %d ( %3d , %3d )   %3d ( %3d , %3d )\n",
	   	nframe, lum.val[0]/(pmat->cols * pmat->rows),
       	(int) minLum, minPoint.x, minPoint.y, (int) maxLum, maxPoint.x, maxPoint.y);
}

int doublecmp(const void *a, const void *b)
{
	if (*((double *) a) < *((double *) b)) return -1;
	else if (*((double *) a) > *((double *) b)) return 1;
	else return 0; 
}

void printtbuf(double *uc, size_t s)
{
	printf("[ ");
	for (size_t i = 0; i < s; i++) {
		printf("%6.3f ", uc[i]);
	}
	printf("]\n");
}
