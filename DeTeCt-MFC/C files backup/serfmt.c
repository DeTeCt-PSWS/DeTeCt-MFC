/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012			*/
/*                                                                              */
/*    SERFMT: SER format functions                                              */
/*                                                                              */
/********************************************************************************/
#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <assert.h>

#include "serfmt.h"
#include "datation.h"
#include "dtc.h"

/*****************MAIN FUNCTION to get header and parameters of SER capture***************************/
SerCapture *serCaptureFromFile(const char *fname)
{
	SerCapture *sc;
	char buffer[SER_HEADER_SIZE];
	char *pread;
	char ptmp[SER_MAX_FIELD_SIZE];
	int nChannels, depth;


															if (opts.debug) { fprintf(stderr, "serCaptureFromFile: creation\n"); }
	sc = calloc(sizeof (SerCapture), 1);
	if (sc == NULL) {
		assert(sc != NULL);
	}
	else {
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile:  Created sercapture %d\n", (int)(sc)); }
		if (!(sc->fh = fopen(fname, "rb")))
		{
			fprintf(stderr, "ERROR in serCaptureFromFile opening %s file\n", fname);
			exit(EXIT_FAILURE);
		}
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile:  Created fh %d\n", (int)(sc->fh)); }

		if (fread(buffer, sizeof(char), SER_HEADER_SIZE, sc->fh) != SER_HEADER_SIZE)
		{
			fprintf(stderr, "ERROR in serCaptureFromFile reading %s ser header\n", fname);
			exit(EXIT_FAILURE);
		}

		pread = buffer;

		memcpy(sc->header.FileID, pread, SER_FILEID_SIZE);
		//printf("--- FileID pread: [%p]\n", pread);
		//serPrintStr(pread, 60); printf("\n");
		pread += SER_FILEID_SIZE;

		memcpy(ptmp, pread, SER_LUID_SIZE);
		//printf("--- LuID pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_LUID_SIZE); printf("\n");
		sc->header.LuID = *((int *)pread);
		pread += SER_LUID_SIZE;

		memcpy(ptmp, pread, SER_COLORID_SIZE);
		//printf("--- ColorID pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_COLORID_SIZE); printf("\n");
		sc->header.ColorID = *((int *)pread);
		pread += SER_COLORID_SIZE;

		memcpy(ptmp, pread, SER_LITTLEENDIAN_SIZE);
		//printf("--- LittleEndian pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_LITTLEENDIAN_SIZE); printf("\n");	
		sc->header.LittleEndian = *((int *)pread);
		pread += SER_LITTLEENDIAN_SIZE;

		memcpy(ptmp, pread, SER_IMAGEWIDTH_SIZE);
		//printf("--- ImageWidth pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_IMAGEWIDTH_SIZE); printf("\n");
		sc->header.ImageWidth = *((int *)pread);
		pread += SER_IMAGEWIDTH_SIZE;

		memcpy(ptmp, pread, SER_IMAGEHEIGHT_SIZE);
		//printf("--- ImageHeight pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_IMAGEHEIGHT_SIZE); printf("\n");
		sc->header.ImageHeight = *((int *)pread);
		pread += SER_IMAGEHEIGHT_SIZE;

		memcpy(ptmp, pread, SER_PIXELDEPTH_SIZE);
		//printf("--- PixelDepth pread: [%p]\n", pread);
		//serPrintStr(ptmp, SER_PIXELDEPTH_SIZE); printf("\n");
		sc->header.PixelDepth = *((int *)pread);
		pread += SER_PIXELDEPTH_SIZE;

		memcpy(ptmp, pread, SER_FRAMECOUNT_SIZE);
		sc->header.FrameCount = *((int *)pread);
		pread += SER_FRAMECOUNT_SIZE;

		memcpy(sc->header.Observer, pread, SER_OBSERVER_SIZE);
		pread += SER_OBSERVER_SIZE;

		memcpy(sc->header.Instrument, pread, SER_INSTRUMENT_SIZE);
		pread += SER_INSTRUMENT_SIZE;

		memcpy(sc->header.Telescope, pread, SER_TELESCOPE_SIZE);
		pread += SER_TELESCOPE_SIZE;

		memcpy(sc->header.DateTime, pread, SER_DATETIME_SIZE);
		pread += SER_DATETIME_SIZE;

		memcpy(sc->header.DateTimeUTC, pread, SER_DATETIMEUTC_SIZE);
		pread += SER_DATETIMEUTC_SIZE;

		sc->BytesPerPixel = sc->header.PixelDepth > 8 ? 2 : 1;
		sc->ImageBytes = sc->header.ImageWidth * sc->header.ImageHeight * sc->BytesPerPixel;

		switch (sc->header.ColorID) {
		case SER_MONO:
		default:
			nChannels = 1;
			depth = sc->header.PixelDepth > 8 ? IPL_DEPTH_16U : IPL_DEPTH_8U;
		}
		if (opts.debug) {
			fprintf(stderr, "serCaptureFromFile: BytesPerPixel = %d ; depth = %d\n", sc->BytesPerPixel, depth);
		}

		/*	sc->image = cvCreateImageHeader(cvSize(sc->header.ImageWidth, sc->header.ImageHeight),
							   depth, nChannels);
							   assert(sc->image != NULL);
							   if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created image %d\n",sc->image); }
							   sc->image->imageData = calloc(sizeof (char), sc->ImageBytes);
							   assert(sc->image->imageData != NULL);
							   if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created image data %d\n",sc->image->imageData); }*/
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Width, Height, depth, nchannels %d,%d,%d,%d\n", sc->header.ImageWidth, sc->header.ImageHeight, depth, nChannels); }
		sc->image = cvCreateImage(cvSize(sc->header.ImageWidth, sc->header.ImageHeight), depth, nChannels);
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created Image %d\n", (int)(sc->image)); }
		/*	sc->TimeStamp = calloc(sizeof (char), SER_DATETIME_SIZE);
			assert(sc->TimeStamp != NULL);
			if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created TimeStamp %d\n",sc->TimeStamp); }*/

		sc->image->widthStep = sc->header.ImageWidth * sc->BytesPerPixel * nChannels;
		sc->image->imageDataOrigin = sc->image->imageData;
		sc->frame = 0;
		sc->TimeStamp_frame = 0;
		sc->TimeStampExists = 0;
		sc->ValidFrameCount = sc->header.FrameCount;
		sc->FrameCount = sc->header.FrameCount;

		if (strncmp(sc->header.FileID, "PlxCapture", strlen("PlxCapture")) == 0) {
			sc->StartTime_JD = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
			sc->StartTimeUTC_JD = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
			sc->EndTime_JD = serDateTime_JD(sc->header.DateTime);
			sc->EndTimeUTC_JD = serDateTime_JD(sc->header.DateTimeUTC);
		}
		else {
			sc->StartTime_JD = serDateTime_JD(sc->header.DateTime);
			sc->StartTimeUTC_JD = serDateTime_JD(sc->header.DateTimeUTC);
			sc->EndTime_JD = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
			sc->EndTimeUTC_JD = gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
		}
		if (opts.debug) {
			fprintf(stderr, "serCaptureFromFile: end\n");
			serPrintHeader(sc);
		}
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created ser %d\n", (int)(sc)); }
	}
	return sc;
}

/*****************Reinitializing file read in sercapture ***************************/
void serReinitCaptureRead(SerCapture *sc,const char *fname)
{
	char buffer[SER_HEADER_SIZE];

	sc->frame = 0;
	sc->TimeStamp_frame = 0;
	rewind(sc->fh);
					if (opts.debug) { fprintf(stderr, "serReinitCaptureRead: %s file rewinded\n", fname); }
	if (fread(buffer, sizeof (char), SER_HEADER_SIZE, sc->fh) != SER_HEADER_SIZE) {
		fprintf(stderr, "ERROR in serReinitCaptureRead reading %s ser header\n", fname);
		exit(EXIT_FAILURE);		
	}
/*	if (!(fsetpos(sc->fh, (fpos_t) (SER_HEADER_SIZE))))
	{
		fprintf(stderr, "ERROR in serReinitCaptureRead reinitializing %s file\n", fname);
		exit(EXIT_FAILURE);
	}*/
/*	if (!(fseek(sc->fh, SER_HEADER_SIZE, SEEK_CUR)))
	{
		fprintf(stderr, "ERROR in serReinitCaptureRead reinitializing %s file\n", fname);
		exit(EXIT_FAILURE);
	}*/
}

/*****************Reading of all timestamps in a row (at the end of file)***************************/
void serReadTimeStamps(SerCapture *sc)
{
	size_t TimeStamp_nframe=0;
	int timezone=0;
	double starttime;
	double endtime;

											if (opts.debug) {
												fprintf(stderr,"serReadTimeStamps: StartTime  ");
												fprint_jd(stderr,sc->StartTime_JD);
												fprintf(stderr,"\n");
												fprintf(stderr,"serReadTimeStamps: StartTimeUT");
												fprint_jd(stderr,sc->StartTimeUTC_JD);
												fprintf(stderr,"\n\n");
											}
	while ((TimeStamp_nframe<sc->header.FrameCount) && (serQueryTimeStamp(sc))) {
		if (TimeStamp_nframe==0) {
			starttime=serDateTime_JD(sc->TimeStamp);
			timezone=(int) floor(0.5+(starttime-sc->StartTimeUTC_JD)*24);
			sc->StartTimeUTC_JD=starttime-timezone/24.0;
			sc->StartTime_JD=starttime;
											if (opts.debug) {
												fprintf(stderr,"serReadTimeStamps: FirstFrame ");
												fprint_jd(stderr,serDateTime_JD(sc->TimeStamp));
												fprintf(stderr,"\n");
											}
		}
		TimeStamp_nframe++;
	}
	if (sc->TimeStampExists) {
			endtime=serDateTime_JD(sc->TimeStamp);
			sc->EndTimeUTC_JD=endtime-timezone/24.0;
			sc->EndTime_JD=endtime;
											if (opts.debug) {
												fprintf(stderr,"serReadTimeStamps: LastFrame  ");
												fprint_jd(stderr,serDateTime_JD(sc->TimeStamp));
												fprintf(stderr,"\n\n");
												fprintf(stderr,"serReadTimeStamps: StartSerUT ");
												fprint_jd(stderr,sc->StartTimeUTC_JD);
												fprintf(stderr,"\n");
												fprintf(stderr,"serReadTimeStamps: EndSerUT   ");
												fprint_jd(stderr,sc->EndTimeUTC_JD);
												fprintf(stderr,"\n");
											}
	}
}

/*****************Reads current frame***************************/			
IplImage *serQueryFrame(SerCapture *sc, const int ignore, int *perror)
{
	size_t bytesR;
	(*perror)=0;

	if ((sc->frame >= sc->header.FrameCount) || ((ignore) && (sc->frame >= sc->ValidFrameCount))) {
		return NULL;
	}
	sc->frame++;
				if (opts.debug) { fprintf(stderr, "serQueryFrame: Reading frame #%d\n", sc->frame); }
	if (!(bytesR = serImageRead(sc->image->imageData, sizeof (char), sc->ImageBytes, sc->fh)))
	{
		sc->ValidFrameCount=sc->frame-1;
		if (!ignore) {
			fprintf(stderr, "ERROR in serQueryFrame reading frame #%d\n", sc->frame);
			exit(EXIT_FAILURE);
		} else {
			(*perror)=1;
			fprintf(stderr, "WARNING in serQueryFrame: ignoring error reading frame #%d and above (%d missing till frame #%d)\n", sc->frame, sc->header.FrameCount-sc->ValidFrameCount, sc->header.FrameCount);
		}
	}
	return sc->image;
}

/*****************Reads current timestamp***************************/			
unsigned char *serQueryTimeStamp(SerCapture *sc)
{
	if (sc->frame != sc->header.FrameCount)
		return NULL;

	if (sc->TimeStamp_frame == sc->header.FrameCount)
		return NULL;

	if (!(serTimeStampRead(sc->TimeStamp, sizeof (char), SER_DATETIME_SIZE, sc->fh)))
	{
		sc->TimeStampExists=0;
		return NULL;
	} else {
		sc->TimeStampExists=1;
		sc->TimeStamp_frame++;
	}
	return sc->TimeStamp;
}

/*****************Cleans capture***************************/			
void serReleaseCapture(SerCapture *sc)
{
	/* fprintf(stderr, "serReleaseCapture: Releasing imageData %d\n", sc->image->imageData);
	free(sc->image->imageData);*/
											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing fh %d\n",(int) (sc->fh)); }
	if (!(fclose(sc->fh)==0)) {
		fprintf(stderr, "ERROR in serReleaseCapture closing capture file\n");
		exit(EXIT_FAILURE);
	}
/*											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image %d size data %d\n", (int) (sc->image), sizeof(*sc->image->imageData)); }
free(sc->image->imageData);*/
											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image %d size image %d\n", (int) (sc->image), sizeof(*sc->image)); }
	cvReleaseImage(&sc->image);
	sc->image=NULL;
/*											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image header %d\n", (int) (sc->image)); }
	cvReleaseImageHeader(&sc->image);
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Releasing TimeStamp %d\n", (int) (sc->TimeStamp)); }
	free(sc->TimeStamp);
	sc->TimeStamp=NULL;*/
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Releasing sc %d\n", (int) (sc)); }
	free(sc);
	sc=NULL;
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Released sc\n"); }
}

/*****************Reads current image data***************************/			
size_t serImageRead(void *image, const size_t size, const size_t num, FILE *f)
{
	size_t bytesC;
	size_t bytesR;
	size_t n;
	size_t *ptr;
	
	bytesR = 0;
	n = num;
	ptr = image;
	while (bytesR < num * size) {
		bytesC = fread(ptr, size, num, f);
		if (!bytesC)
			return 0;	
		bytesR += bytesC;
		ptr += bytesR;
		n -= bytesR;
	}
	return bytesR;
}

/*****************Reads current timestamp***************************/			
size_t serTimeStampRead(unsigned char *pTimeStamp, const size_t size, const size_t num, FILE *f)
{
	size_t bytesC;
	size_t bytesR;
	size_t n;
	unsigned char *ptr;
	
	bytesR = 0;
	n = num;
	ptr = pTimeStamp;
	while (bytesR < num * size)
	{
		bytesC = fread(ptr, size, num, f);
		
		if (!bytesC)
			return 0;
		
		bytesR += bytesC;
		ptr += bytesR;
		n -= bytesR;
	}
	return bytesR;
}

/*****************Transforms SER date format to Julian Day***************************/			
double serDateTime_JD(unsigned char *headerfield)
{
	double DateTime =0;
	unsigned char *p;
	int i;
	
	p=headerfield;
	for (i = 2; i <= SER_DATETIME_SIZE; i++) {
		(void) (*p++);
	}
/*	fprintf(stderr,"serDateTime_JD: p %d pand %d\n", (*p), ((*p)& 63));*/
	DateTime=((*p--)& 63);
	for (i = SER_DATETIME_SIZE-1; i > 0; i--) {
/*		fprintf(stderr,"serDateTime_JD: p %d\n", (*p));*/
		DateTime=DateTime*256+(*p--);
	}
	DateTime=DateTime/10000000/60/60/24;
	
	return DateTime+gregorian_calendar_to_jd(1,1,1,0,0,0);
}

/*****************Prints SER string***************************/			
void serPrintStr(char *p, int sz)
{
	for (int i = 0; i < sz; i++)
	{
		if (*p >= 32 && *p <= 126)
			fprintf(stderr,"%2c ", *p++);
		else
			fprintf(stderr,"%02X ", *p++);
	}
}

/*****************Prints SER Byte***************************/			
void serPrintByte(unsigned char *p, int sz)
{
	for (int i = 0; i < sz; i++)
	{
		fprintf(stderr,"%03d ", *p++);
	}
}

/*****************Prints SER header***************************/			
void serPrintHeader(SerCapture *sc)
{
	fprintf(stderr,"FileID       : "); serPrintStr(sc->header.FileID, SER_FILEID_SIZE); fprintf(stderr,"\n");
	fprintf(stderr,"LuID         : %d\n", sc->header.LuID); 	
	fprintf(stderr,"ColorID      : %d\n", sc->header.ColorID);
	fprintf(stderr,"LittleEndian : %d\n", sc->header.LittleEndian); 
	fprintf(stderr,"ImageWidth   : %d\n", sc->header.ImageWidth); 
	fprintf(stderr,"ImageHeight  : %d\n", sc->header.ImageHeight); 
	fprintf(stderr,"PixelDepth   : %d\n", sc->header.PixelDepth); 
	fprintf(stderr,"FrameCount   : %d\n", sc->header.FrameCount);
	fprintf(stderr,"Observer     : "); serPrintStr(sc->header.Observer, SER_OBSERVER_SIZE); fprintf(stderr,"\n"); 
	fprintf(stderr,"Instrument   : "); serPrintStr(sc->header.Instrument, SER_INSTRUMENT_SIZE); fprintf(stderr,"\n");
	fprintf(stderr,"Telescope    : "); serPrintStr(sc->header.Telescope, SER_TELESCOPE_SIZE); fprintf(stderr,"\n");
	fprintf(stderr,"DateTime     : "); serPrintByte(sc->header.DateTime, SER_DATETIME_SIZE); fprintf(stderr,"\n");
	fprintf(stderr,"DateTimeUTC  : "); serPrintByte(sc->header.DateTimeUTC, SER_DATETIMEUTC_SIZE); fprintf(stderr,"\n");
	fprintf(stderr,"StartTime    : %f\n", sc->StartTime_JD);
	fprintf(stderr,"StartTimeUTC : %f\n", sc->StartTimeUTC_JD);
	fprintf(stderr,"EndTime      : %f\n", sc->EndTime_JD);
	fprintf(stderr,"EndTimeUTC   : %f\n", sc->EndTimeUTC_JD);
}