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
#include <cassert>
#include <stdint.h>

#include "serfmt.h"
#include "datation.h"
#include "dtc.h"

unsigned char*	serQueryTimeStamp(SerCapture* sc);
size_t 			serTimeStampRead(unsigned char* pTimeStamp, const size_t size, const size_t num, FILE* f);
double 			serDateTime_JD(unsigned char* headerfield);
void 			serPrintStr(char* p, int sz);

size_t			serFrameRead(SerCapture* sc);


/*****************MAIN FUNCTION to get header and parameters of SER capture***************************/
SerCapture *serCaptureFromFile(const char *fname)
{
	SerCapture *sc;
	char buffer[SER_HEADER_SIZE];
	char *pread;
	char ptmp[SER_MAX_FIELD_SIZE];
	//int nChannels, depth;
	int depth;
	if (debug_mode) { fprintf(stdout, "serCaptureFromFile: creation\n"); }
	sc = (SerCapture*)calloc(sizeof(SerCapture), 1);
	if (sc == NULL) {
		assert(sc != NULL);
	} else {
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile:  Created sercapture %p\n", sc); }
		if (!(sc->fh = fopen(fname, "rb")))
		{
			char msgtext[MAX_STRING] = { 0 };
			snprintf(msgtext,MAX_STRING, "cannot open %s ser file\n", fname);
			Warning(WARNING_MESSAGE_BOX, "cannot open ser file", "serCaptureFromFile()", msgtext);
			//exit(EXIT_FAILURE);
			return NULL;
		}
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile:  Created fh %p\n", sc->fh); }

		//_fseeki64(sc->fh, 0L, SEEK_SET);
		_fseeki64(sc->fh, 0L, SEEK_END);
		__int64 actual_file_size = _ftelli64(sc->fh);

		_fseeki64(sc->fh, 0L, SEEK_SET);

		if (fread(buffer, sizeof(char), SER_HEADER_SIZE, sc->fh) != SER_HEADER_SIZE)
		{
			char msgtext[MAX_STRING] = { 0 };
			snprintf(msgtext,MAX_STRING, "cannot read %s ser header\n", fname);
			Warning(WARNING_MESSAGE_BOX, "cannot read ser header", "serCaptureFromFile()", msgtext);
			fclose(sc->fh);
			//exit(EXIT_FAILURE);
			return NULL;
		}

		size_t frames_size = actual_file_size - SER_HEADER_SIZE;

		pread = buffer;

		//1_FileID
		//Format: String Length : 14 Byte(14 ASCII characters)
		//Content : "LUCAM-RECORDER"
		memcpy(sc->header.FileID, pread, SER_FILEID_SIZE);
		pread += SER_FILEID_SIZE;

		//2_LuID
		//Format: Integer_32(little - endian
		//Length : 4 Byte
		//Content : Lumenera camera series ID(currently unused; default = 0)
		memcpy(ptmp, pread, SER_LUID_SIZE);
		sc->header.LuID = *((int *)pread);
		pread += SER_LUID_SIZE;

		//3_ColorID
		//	Format : Integer_32(little - endian)
		//	Length : 4 Byte
		//	Content : MONO = 0
		//	BAYER_RGGB = 8
		//	BAYER_GRBG = 9
		//	BAYER_GBRG = 10
		//	BAYER_BGGR = 11
		//	BAYER_CYYM = 16
		//	BAYER_YCMY = 17
		//	BAYER_YMCY = 18
		//	BAYER_MYYC = 19
		//	RGB = 100
		//	BGR = 101
		memcpy(ptmp, pread, SER_COLORID_SIZE);
		sc->header.ColorID = *((int *)pread);
		pread += SER_COLORID_SIZE;

		//4_LittleEndian
		//Format: Integer_32(little - endian)
		//Length : 4 Byte
		//Content :	0 (FALSE) for big - endian byte order in 16 bit image data
		//			1 (TRUE) for little - endian byte order in 16 bit image data
		memcpy(ptmp, pread, SER_LITTLEENDIAN_SIZE);
		sc->header.LittleEndian = *((int *)pread);
		pread += SER_LITTLEENDIAN_SIZE;

		//5_ImageWidth
		//Format: Integer_32(little - endian)
		//Length : 4 Byte
		//Content : Width of every image in pixel
		memcpy(ptmp, pread, SER_IMAGEWIDTH_SIZE);
		sc->header.ImageWidth = *((int *)pread);
		pread += SER_IMAGEWIDTH_SIZE;

		//6_ImageHeight
		//Format: Integer_32(little - endian)
		//Length : 4 Byte
		//Content : Height of every image in pixel
		memcpy(ptmp, pread, SER_IMAGEHEIGHT_SIZE);
		sc->header.ImageHeight = *((int *)pread);
		pread += SER_IMAGEHEIGHT_SIZE;

		/*7_PixelDepthPerPlane
			Format : Integer_32(little - endian)
			Length : 4 Byte
			Content : True bit depth per pixel per plane
				3_ColorID			NumberOfPlanes
				MONO … BAYER_MYYC		1
				RGB, BGR				3
			7_PixelDepthPerPlane	BytesPerPixel
			1..8					1 * NumberOfPlanes
			9..16					2 * NumberOfPlanes*/
		memcpy(ptmp, pread, SER_PIXELDEPTH_SIZE);
		sc->header.PixelDepth = *((int *)pread);
		pread += SER_PIXELDEPTH_SIZE;

		/*8_FrameCount
		Format: Integer_32 (little-endian)
		Length: 4 Byte
		Content: Number of image frames in SER file*/
		memcpy(ptmp, pread, SER_FRAMECOUNT_SIZE);
		sc->header.FrameCount = *((int *)pread);
		pread += SER_FRAMECOUNT_SIZE;

		/*9_Observer
		Format: String
		Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
		Content: Name of observer*/
		memcpy(sc->header.Observer, pread, SER_OBSERVER_SIZE);
		pread += SER_OBSERVER_SIZE;

		/*10_Instrument
		Format: String
		Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
		Content: Name of used camera*/
		memcpy(sc->header.Instrument, pread, SER_INSTRUMENT_SIZE);
		pread += SER_INSTRUMENT_SIZE;

		/*11_Telescope
		Format: String
		Length: 40 Byte (40 ASCII characters {32…126 dec.}, fill unused characters with 0 dec.)
		Content: Name of used telescope*/
		memcpy(sc->header.Telescope, pread, SER_TELESCOPE_SIZE);
		pread += SER_TELESCOPE_SIZE;
			//fps=113.64gain=85exp=8.00
			//0     6
		double fps = 0.0;
		char fps_search[SER_TELESCOPE_SIZE];
		strcpy(fps_search,"fps=");
		if (InStr(sc->header.Telescope, fps_search) >= 0) {
			size_t i = strlen(fps_search);
			do  {
				if ((isdigit(sc->header.Telescope[i]) > 0) || (sc->header.Telescope[i] == '.')) i++;
				else break;
			} while (i < SER_TELESCOPE_SIZE);
			char tmpstr[SER_TELESCOPE_SIZE];
			fps = atof(mid(sc->header.Telescope, strlen(fps_search), (i - strlen(fps_search)), tmpstr));
		}
		/*12_DateTime
		Format : Date / Integer_64(little - endian)
		Length : 8 Byte
		Content : Start time of image stream(local time)
				If 12_DateTime <= 0 then 12_DateTime is invalid and the SER file does not contain a Time stamp trailer.*/
		memcpy(sc->header.DateTime, pread, SER_DATETIME_SIZE);
		pread += SER_DATETIME_SIZE;

		/*13_DateTime_UTC
		Format: Date / Integer_64 (little-endian)
		Length: 8 Byte
		Content: Start time of image stream in UTC*/
		memcpy(sc->header.DateTimeUTC, pread, SER_DATETIMEUTC_SIZE);
		pread += SER_DATETIMEUTC_SIZE;

		/*Image Data
			Image data starts at File start offset decimal 178
			Size of every image frame in byte is: 5_ImageWidth x 6_ImageHeigth x BytePerPixel
		Trailer in detail
			Trailer starts at byte offset: 178 + 8_FrameCount x 5_ImageWidth x 6_ImageHeigth x BytePerPixel.
			Trailer contains Date / Integer_64 (little-endian) time stamps in UTC for every image frame.*/

		depth = sc->header.PixelDepth > 8 ? IPL_DEPTH_16U : IPL_DEPTH_8U;

		switch (sc->header.ColorID) {
		case SER_RGB: case SER_BGR:
			sc->nChannels = 3;
			break;
		default:
			sc->nChannels = 1;
			break;
		}

		sc->byte_depth = sc->header.PixelDepth > 8 ? 2 : 1;
		sc->BytesPerPixel = sc->byte_depth * sc->nChannels;
		sc->ImageBytes = sc->header.ImageWidth * sc->header.ImageHeight * sc->BytesPerPixel;

		if (debug_mode) {
			fprintf(stdout, "serCaptureFromFile: BytesPerPixel = %zd ; depth = %d\n", sc->BytesPerPixel, depth);
		}

		/*	sc->image = cvCreateImageHeader(cvSize(sc->header.ImageWidth, sc->header.ImageHeight),
		depth, nChannels);
		assert(sc->image != NULL);
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile: Created image %d\n",sc->image); }
		sc->image->imageData = calloc(sizeof (char), sc->ImageBytes);
		assert(sc->image->imageData != NULL);
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile: Created image data %d\n",sc->image->imageData); }*/
		if (debug_mode) {
			fprintf(stdout, "serCaptureFromFile: Width, Height, depth, nchannels %zd,%zd,%d,%d\n",
				sc->header.ImageWidth, sc->header.ImageHeight, depth, sc->nChannels);
		}
		sc->image = cvCreateImage(cvSize((int)sc->header.ImageWidth, (int)sc->header.ImageHeight), depth, sc->nChannels);
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile: Created Image %p\n", sc->image); }
		/*	sc->TimeStamp = calloc(sizeof (char), SER_DATETIME_SIZE);
		assert(sc->TimeStamp != NULL);
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile: Created TimeStamp %d\n",sc->TimeStamp); }*/

		sc->image->widthStep = (int) (sc->header.ImageWidth * sc->BytesPerPixel * sc->nChannels);
		sc->image->imageDataOrigin = sc->image->imageData;
		sc->frame = 0;
		sc->TimeStamp_frame = 0;
		sc->TimeStampExists = 0;
		sc->ValidFrameCount = sc->header.FrameCount;
		sc->FrameCount = sc->header.FrameCount;
		double duration = 0.0;
		if (fps > 0) duration = (sc->FrameCount / fps);

		if (strncmp(sc->header.FileID, "PlxCapture", strlen("PlxCapture")) == 0) {
			sc->EndTime_JD =			serDateTime_JD(sc->header.DateTime);
			sc->EndTimeUTC_JD =			serDateTime_JD(sc->header.DateTimeUTC);
			if (duration > 0) {
				sc->StartTime_JD =		sc->EndTime_JD - duration / ONE_DAY_SEC;
				sc->StartTimeUTC_JD =	sc->EndTimeUTC_JD - duration / ONE_DAY_SEC;
			}
			else {
				sc->StartTime_JD =		gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
				sc->StartTimeUTC_JD =	gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
			}
		}
		else {
			sc->StartTime_JD =			serDateTime_JD(sc->header.DateTime);
			sc->StartTimeUTC_JD =		serDateTime_JD(sc->header.DateTimeUTC);
			if (duration > 0) {
				sc->EndTime_JD =		sc->StartTime_JD + duration / ONE_DAY_SEC;
				sc->EndTimeUTC_JD =		sc->StartTimeUTC_JD + duration / ONE_DAY_SEC;
			}
			else {
				sc->EndTime_JD =		gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
				sc->EndTimeUTC_JD =		gregorian_calendar_to_jd(1, 1, 1, 0, 0, 0);
			}
		}
		if (debug_mode) {
			fprintf(stdout, "serCaptureFromFile: end\n");
			serPrintHeader(sc);
		}
		if (debug_mode) { fprintf(stdout, "serCaptureFromFile: Created ser %p\n", sc); }
		size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);
		sc->frame_data = calloc(im_size, sc->ImageBytes);
		sc->mat_type = sc->byte_depth == 2 ?
			sc->nChannels == 3 ? CV_16UC3 : CV_16UC1 :
			sc->nChannels == 3 ? CV_8UC3 : CV_8UC1;
		sc->current_frame = 0;
		long file_size = (long) (SER_HEADER_SIZE + sc->FrameCount * (sc->ImageBytes + 8));
		if (actual_file_size < file_size)
			sc->FrameCount = sc->header.FrameCount = frames_size / (sc->ImageBytes + 8);
		sc->big_endian_proc = (*(uint16_t *)"\0\xff" < 0x100);
		// little_endian: 0 for little endian, 1 for big endian
		//sc->data_proc_same_endianness = sc->big_endian_proc == sc->header.LittleEndian;
		sc->data_proc_same_endianness = (unsigned int)(sc->big_endian_proc) == sc->header.LittleEndian;
		/*if (fclose(sc->fh) != 0) {
//			fprintf(stderr, "ERROR in serCaptureFromFile closing capture file\n");
			exit(EXIT_FAILURE);
		}*/
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
					if (debug_mode) { fprintf(stdout, "serReinitCaptureRead: %s file rewinded\n", fname); }
	if (fread(buffer, sizeof (char), SER_HEADER_SIZE, sc->fh) != SER_HEADER_SIZE) {
		char msgtext[MAX_STRING] = { 0 };									
		snprintf(msgtext, MAX_STRING, "wrong header size in %s", fname);
		ErrorExit(TRUE, "wrong header size", "serReinitCaptureRead()", msgtext);
		//exit(EXIT_FAILURE);		
	}
/*	if (!(fsetpos(sc->fh, (fpos_t) (SER_HEADER_SIZE))))
	{
//		fprintf(stderr, "ERROR in serReinitCaptureRead reinitializing %s file\n", fname);
		exit(EXIT_FAILURE);
	}*/
/*	if (!(fseek(sc->fh, SER_HEADER_SIZE, SEEK_CUR)))
	{
//		fprintf(stderr, "ERROR in serReinitCaptureRead reinitializing %s file\n", fname);
		exit(EXIT_FAILURE);
	}*/
}

/*****************Reading of all timestamps in a row (at the end of file)***************************/
void serReadTimeStamps(SerCapture *sc)
{
	size_t TimeStamp_nframe=0;
	int timezone=0;
	double starttime = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	double endtime = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	double JD_min = gregorian_calendar_to_jd(1980, 1, 1, 0, 0, 0);
	double JD_max = gregorian_calendar_to_jd(2080, 1, 1, 0, 0, 0);
	
// Save positions
	__int64 offset = _ftelli64(sc->fh);
	int frame_old = (int)sc->frame;

							if (debug_mode) {
												fprintf(stdout,"serReadTimeStamps: StartTime  ");
												fprint_jd(stdout,sc->StartTime_JD);
												fprintf(stdout,"\n");
												fprintf(stdout,"serReadTimeStamps: StartTimeUT");
												fprint_jd(stdout,sc->StartTimeUTC_JD);
												fprintf(stdout,"\n\n");
											}
	
//Position to beginning of TimeStamps zone
	//Trailer starts at byte offset : 178 + 8_FrameCount x 5_ImageWidth x 6_ImageHeigth x BytePerPixel.

	_fseeki64(sc->fh, SER_HEADER_SIZE + sc->header.FrameCount*sc->ImageBytes, SEEK_SET);
	sc->frame = sc->header.FrameCount;
	while ((TimeStamp_nframe<sc->header.FrameCount) && (serQueryTimeStamp(sc))) {
		if (TimeStamp_nframe==0) {
			starttime=serDateTime_JD(sc->TimeStamp);
			if ((starttime > JD_min) && (starttime < JD_max)) sc->StartTime_JD = starttime;
			else sc->TimeStampExists = 0;
											if (debug_mode) {
												fprintf(stdout,"serReadTimeStamps: FirstFrame ");
												fprint_jd(stdout,serDateTime_JD(sc->TimeStamp));
												fprintf(stdout,"\n");
											}
		}
		TimeStamp_nframe++;
	}
	if (sc->TimeStampExists) {
			endtime=serDateTime_JD(sc->TimeStamp);
			if ((endtime > JD_min) && (endtime < JD_max))	sc->EndTime_JD = endtime;
			else sc->TimeStampExists = 0;
			if (debug_mode) {
												fprintf(stdout,"serReadTimeStamps: LastFrame  ");
												fprint_jd(stdout,serDateTime_JD(sc->TimeStamp));
												fprintf(stdout,"\n\n");
												fprintf(stdout,"serReadTimeStamps: StartSerUT ");
												fprint_jd(stdout,sc->StartTimeUTC_JD);
												fprintf(stdout,"\n");
												fprintf(stdout,"serReadTimeStamps: EndSerUT   ");
												fprint_jd(stdout,sc->EndTimeUTC_JD);
												fprintf(stdout,"\n");
											}
	}
	if (sc->TimeStampExists) {
		if ((sc->StartTimeUTC_JD > JD_min) && (sc->StartTimeUTC_JD < JD_max)) timezone = (int)floor(0.5 + (starttime - sc->StartTimeUTC_JD) * 24);
		else if ((sc->EndTimeUTC_JD > JD_min) && (sc->EndTimeUTC_JD < JD_max)) timezone = (int)floor(0.5 + (endtime - sc->EndTimeUTC_JD) * 24);
		sc->StartTimeUTC_JD = starttime - timezone / 24.0;
		sc->EndTimeUTC_JD = endtime - timezone / 24.0;
	}

// Restore positions
	sc->frame = frame_old;
	_fseeki64(sc->fh, offset, SEEK_SET);
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
	if (sc != NULL) {
		/* fprintf(stdout, "serReleaseCapture: Releasing imageData %d\n", sc->image->imageData);
		free(sc->image->imageData);*/
		if (debug_mode) { fprintf(stdout, "serReleaseCapture: Releasing fh %p\n", sc->fh); }
		if (!(fclose(sc->fh) == 0)) {
			char msgtext[MAX_STRING] = { 0 };
			snprintf(msgtext,MAX_STRING, "cannot close capture file\n");
			Warning(WARNING_MESSAGE_BOX, "cannot close capture file", "serReleaseCapture()", msgtext);
			//exit(EXIT_FAILURE);
		}
		/*											if (debug_mode) { fprintf(stdout, "serReleaseCapture: Releasing image %d size data %d\n", (int) (sc->image), sizeof(*sc->image->imageData)); }
		free(sc->image->imageData);*/
		if (debug_mode) { fprintf(stdout, "serReleaseCapture: Releasing image %p size image %zd\n", sc->image, sizeof(*sc->image)); }
		cvReleaseImage(&sc->image);
		sc->image = NULL;
		/*											if (debug_mode) { fprintf(stdout, "serReleaseCapture: Releasing image header %d\n", (int) (sc->image)); }
			cvReleaseImageHeader(&sc->image);
													if (debug_mode) { 	fprintf(stdout, "serReleaseCapture: Releasing TimeStamp %d\n", (int) (sc->TimeStamp)); }
			free(sc->TimeStamp);
			sc->TimeStamp=NULL;*/
		if (debug_mode) { fprintf(stdout, "serReleaseCapture: Releasing sc %p\n", sc); }
		free(sc);
		sc = NULL;
		if (debug_mode) { fprintf(stdout, "serReleaseCapture: Released sc\n"); }
	}
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
/*	fprintf(stdout,"serDateTime_JD: p %d pand %d\n", (*p), ((*p)& 63));*/
	DateTime=((*p--)& 63);
	for (i = SER_DATETIME_SIZE-1; i > 0; i--) {
/*		fprintf(stdout,"serDateTime_JD: p %d\n", (*p));*/
		DateTime=DateTime*256+(*p--);
	}
	DateTime=DateTime/10000000/ONE_DAY_SEC;
	
	return DateTime+gregorian_calendar_to_jd(1,1,1,0,0,0);
}

/*****************Prints SER string***************************/			
void serPrintStr(char *p, int sz)
{
	char* output;
	for (int i = 0; i < sz; i++)
	{
		if (*p >= 32 && *p <= 126) {
			//fprintf(stdout,"%2c ", *p++);
			output = (char*)calloc(sizeof(char), 10);
			if (output != 0) sprintf_s(output, 10, "%c", *p++);  // warning C6387 disabling
		}
		else {
			//fprintf(stdout,"%02X ", *p++);
			output = (char*)calloc(sizeof(char), 10);
			if (output != 0) sprintf_s(output, 10, "%X", *p++); // warning C6387 disabling
		}

		OutputDebugStringA(output);
		free(output);
	}
}

/*****************Prints SER Byte***************************/			
void serPrintByte(unsigned char *p, int sz)
{
	char output[5];
	for (int i = 0; i < sz; i++)
	{
		//fprintf(stdout,"%03d ", *p++);
		sprintf_s(output, 5, "%03d", *p++);
		OutputDebugStringA(output);
	}

}

/*****************Prints SER header***************************/			
void serPrintHeader(SerCapture *sc)
{
	char buffer[MAX_STRING] = { 0 };
	sprintf_s(buffer, MAX_STRING, "FileID       : %s\n", sc->header.FileID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "LuID         : %d\n", (int)sc->header.LuID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "ColorID      : %d\n", (int)sc->header.ColorID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "LittleEndian : %d\n", (int)sc->header.LittleEndian);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "ImageWidth   : %d\n", (int)sc->header.ImageWidth);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "ImageHeight  : %d\n", (int)sc->header.ImageHeight);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "PixelDepth   : %d\n", (int)sc->header.PixelDepth);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "FrameCount   : %d\n", (int)sc->header.FrameCount);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "Observer     : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Observer, SER_OBSERVER_SIZE);
	sprintf_s(buffer, MAX_STRING, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "Instrument   : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Instrument, SER_INSTRUMENT_SIZE);
	sprintf_s(buffer, MAX_STRING, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "Telescope    : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Telescope, SER_TELESCOPE_SIZE);
	sprintf_s(buffer, MAX_STRING, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "DateTime     : ");
	OutputDebugStringA(buffer);
	serPrintByte(sc->header.DateTime, SER_DATETIME_SIZE);
	sprintf_s(buffer, MAX_STRING, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "DateTimeUTC  : ");
	OutputDebugStringA(buffer);
	serPrintByte(sc->header.DateTimeUTC, SER_DATETIMEUTC_SIZE);
	sprintf_s(buffer, MAX_STRING, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "StartTime    : %f\n", sc->StartTime_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "StartTimeUTC : %f\n", sc->StartTimeUTC_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "EndTime      : %f\n", sc->EndTime_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "EndTimeUTC   : %f\n", sc->EndTime_JD);
	OutputDebugStringA(buffer);
}

size_t serFrameRead(SerCapture* sc) {
	uint16_t* frame_data16 = (uint16_t*)sc->frame_data;
	uint8_t* frame_data8 = (uint8_t*)sc->frame_data;

	size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);

	_Post_ _Notnull_ void* temp_buffer = calloc(im_size, sc->ImageBytes);  // warning C6387

	if (temp_buffer != NULL) {
		size_t read_values = fread(temp_buffer, 1, sc->ImageBytes, sc->fh);

		uint16_t* temp_buffer16 = (uint16_t*)temp_buffer;
		uint8_t* temp_buffer8 = (uint8_t*)temp_buffer;

		uint8_t* read_ptr8, * read_ptr8_mono;
		uint16_t* read_ptr, * read_ptr_mono;

		if (read_values == sc->ImageBytes) {

			if (sc->current_frame == 0 && sc->header.PixelDepth > 8) {

			}

			for (int32_t y = (int32_t)(sc->header.ImageHeight) - 1; y >= 0; --y) {

				read_ptr = &temp_buffer16[y * (int32_t)(sc->header.ImageWidth) * 3];
				read_ptr8 = &temp_buffer8[y * (uint32_t)(sc->header.ImageWidth) * 3];
				read_ptr_mono = &temp_buffer16[y * (uint32_t)(sc->header.ImageWidth)];
				read_ptr8_mono = &temp_buffer8[y * (uint32_t)(sc->header.ImageWidth)];

				uint32_t shift_left = 16 - (uint32_t)(sc->header.PixelDepth);
				uint32_t shift_right = (uint32_t)(sc->header.PixelDepth) - shift_left;

				for (int32_t x = 0; x < sc->header.ImageWidth; ++x) {

					uint16_t r, g, b;
					int32_t b_idx, g_idx, r_idx, grey_idx;

					b_idx = 3 * y * (int32_t)(sc->header.ImageWidth) + 3 * x;
					g_idx = 3 * y * (int32_t)(sc->header.ImageWidth) + 3 * x + 1;
					r_idx = 3 * y * (int32_t)(sc->header.ImageWidth) + 3 * x + 2;
					grey_idx = y * (int32_t)(sc->header.ImageWidth) + x;

					if (sc->byte_depth == 2) {
						if (sc->header.ColorID == SER_RGB) {
							if (sc->header.PixelDepth == 16) {
								if (sc->data_proc_same_endianness) {
									r = *read_ptr++;
									g = *read_ptr++;
									b = *read_ptr++;
								}
								else {
									r = (*read_ptr8++) << 8;
									r += *read_ptr8++;
									g = (*read_ptr8++) << 8;
									g += *read_ptr8++;
									b = (*read_ptr8++) << 8;
									b += *read_ptr8;
								}
								frame_data16[b_idx] = b;
								frame_data16[g_idx] = g;
								frame_data16[r_idx] = r;
							}
							else {
								if (sc->data_proc_same_endianness) {
									r = *read_ptr++;
									r = (r << shift_left) + (r >> shift_right);
									g = *read_ptr++;
									g = (g << shift_left) + (g >> shift_right);
									b = *read_ptr++;
									b = (b << shift_left) + (b >> shift_right);
									frame_data16[b_idx] = b;
									frame_data16[g_idx] = g;
									frame_data16[r_idx] = r;
								}
								else {
									r = *read_ptr8 << 8;
									r += *read_ptr8;
									g = *read_ptr8 << 8;
									g += *read_ptr8;
									b = *read_ptr8 << 8;
									b += *read_ptr8;
									frame_data16[b_idx] = (b << shift_left) + (b >> shift_right);
									frame_data16[g_idx] = (g << shift_left) + (g >> shift_right);
									frame_data16[r_idx] = (r << shift_left) + (r >> shift_right);
								}
							}
						}
						else if (sc->header.ColorID == SER_BGR) {
							if (sc->header.PixelDepth == 16) {
								if (sc->data_proc_same_endianness) {
									b = *read_ptr++;
									g = *read_ptr++;
									r = *read_ptr++;
								}
								else {
									b = (*read_ptr8++) << 8;
									b += *read_ptr8++;
									g = (*read_ptr8++) << 8;
									g += *read_ptr8++;
									r = (*read_ptr8++) << 8;
									r += *read_ptr8;
								}
							}
							else {
								if (sc->data_proc_same_endianness) {
									b = *read_ptr++;
									b = (b << shift_left) + (b >> shift_right);
									g = *read_ptr++;
									g = (g << shift_left) + (b >> shift_right);
									r = *read_ptr++;
									r = (r << shift_left) + (r >> shift_right);
								}
								else {
									//								b += *read_ptr8++;
									b = *read_ptr8++;
									b = (b << shift_left) + (b >> shift_right);
									//								g += *read_ptr8++;
									g = *read_ptr8++;
									g = (g << shift_left) + (g >> shift_right);
									//								r += *read_ptr8++;
									r = *read_ptr8++;
									r = (r << shift_left) + (r >> shift_right);
								}
							}
							frame_data16[b_idx] = b;
							frame_data16[g_idx] = g;
							frame_data16[r_idx] = r;
						}
						else {
							uint16_t val;
							if (sc->header.PixelDepth == 16) {
								if (sc->data_proc_same_endianness) {
									val = *read_ptr_mono++;
								}
								else {
									val = (*read_ptr8_mono++) << 8;
									val += *read_ptr8_mono++;
								}
							}
							else {
								if (sc->data_proc_same_endianness) {
									val = *read_ptr_mono++;
									val = (val << shift_left) + (val >> shift_right);
								}
								else {
									val = (*read_ptr8_mono++) << 8;
									val += *read_ptr8_mono++;
									val = (val << shift_left) + (val >> shift_right);
								}
							}
							frame_data16[grey_idx] = val;
						}
					}
					else {
						uint8_t r8, g8, b8;
						if (sc->header.ColorID == SER_RGB) {
							r8 = *read_ptr8++;
							g8 = *read_ptr8++;
							b8 = *read_ptr8++;
							frame_data8[b_idx] = b8;
							frame_data8[g_idx] = g8;
							frame_data8[r_idx] = r8;
						}
						else if (sc->header.ColorID == SER_BGR) {
							b8 = *read_ptr8++;
							g8 = *read_ptr8++;
							r8 = *read_ptr8++;
							frame_data8[b_idx] = b8;
							frame_data8[g_idx] = g8;
							frame_data8[r_idx] = r8;
						}
						else {
							frame_data8[grey_idx] = *read_ptr8_mono++;
						}
					}
				}
			}
		}
		else {
			read_values = 0;
		}
		free(temp_buffer);
		return read_values;
	} else return 0;
}

void* serQueryFrameData(SerCapture *sc, const int ignore, int *perror)
{
	size_t bytesR;
	(*perror) = 0;

	if ((sc->frame >= sc->header.FrameCount) || ((ignore) && (sc->frame >= sc->ValidFrameCount))) {
		return NULL;
	}
	sc->frame++;
	if (debug_mode) { fprintf(stdout, "serQueryFrame: Reading frame #%zd\n", sc->frame); }
	if (!(bytesR = serFrameRead(sc)))
	{
		sc->ValidFrameCount = sc->frame - 1;
		if (!ignore) {
			 char msgtext[MAX_STRING] = { 0 };										
			snprintf(msgtext, MAX_STRING, "cannot read ser frame %zd", sc->frame);
			ErrorExit(TRUE, "cannot read ser frame", "serQueryFrame()", msgtext);
		} else {
			(*perror) = 1;
			 char msgtext[MAX_STRING] = { 0 };	
			snprintf(msgtext, MAX_STRING, "cannot read ser frame #%zd and above (%zd missing till frame #%zd)", sc->frame, sc->header.FrameCount - sc->ValidFrameCount, sc->header.FrameCount);
			Warning(WARNING_MESSAGE_BOX, "cannot read ser frame", "serQueryFrame()", msgtext);
		}
	}
	sc->current_frame++;
	return sc->frame_data;
}

void serFixPixelDepth(SerCapture *sc, int frame_number) {
	_fseeki64(sc->fh, SER_HEADER_SIZE + frame_number * sc->ImageBytes, SEEK_SET);

	size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);
	_Post_ _Notnull_ void* temp_buffer = calloc(im_size, sc->ImageBytes);  // warning C6387
	if (temp_buffer != 0) { // warning C6387
		size_t read_values = fread(temp_buffer, 1, sc->ImageBytes, sc->fh);
		uint16_t* temp_buffer16 = (uint16_t*)temp_buffer;

		uint16_t max_pixel = 0;
		if (read_values == sc->ImageBytes) {
			for (int x = 0; x < sc->ImageBytes; x++) {
				max_pixel |= *temp_buffer16++;
			}

			int32_t pixel_depth = 8;
			for (int x = 15; x >= 8; x--) {
				if (max_pixel >= (1 << x)) {
					pixel_depth = x + 1;
					break;
				}
			}

			sc->header.PixelDepth = pixel_depth;
			if (sc->header.PixelDepth > 8)
				sc->byte_depth = 2;
		}

		free(temp_buffer);

		_fseeki64(sc->fh, SER_HEADER_SIZE, SEEK_SET);
	}
}

