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

/*****************MAIN FUNCTION to get header and parameters of SER capture***************************/
SerCapture *serCaptureFromFile(const char *fname)
{
	SerCapture *sc;
	char buffer[SER_HEADER_SIZE];
	char *pread;
	char ptmp[SER_MAX_FIELD_SIZE];
	//int nChannels, depth;
	int depth;
	if (opts.debug) { fprintf(stderr, "serCaptureFromFile: creation\n"); }
	sc = (SerCapture*)calloc(sizeof(SerCapture), 1);
	if (sc == NULL) {
		assert(sc != NULL);
	} else {
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile:  Created sercapture %p\n", sc); }
		if (!(sc->fh = fopen(fname, "rb")))
		{
			fprintf(stderr, "ERROR in serCaptureFromFile opening %s file\n", fname);
			exit(EXIT_FAILURE);
		}
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile:  Created fh %p\n", sc->fh); }

		_fseeki64(sc->fh, 0L, SEEK_END);
		long actual_file_size = _ftelli64(sc->fh);

		_fseeki64(sc->fh, 0L, SEEK_SET);

		if (fread(buffer, sizeof(char), SER_HEADER_SIZE, sc->fh) != SER_HEADER_SIZE)
		{
			fprintf(stderr, "ERROR in serCaptureFromFile reading %s ser header\n", fname);
			exit(EXIT_FAILURE);
		}

		size_t frames_size = actual_file_size - SER_HEADER_SIZE;

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

		if (opts.debug) {
			fprintf(stderr, "serCaptureFromFile: BytesPerPixel = %zd ; depth = %d\n", sc->BytesPerPixel, depth);
		}

		/*	sc->image = cvCreateImageHeader(cvSize(sc->header.ImageWidth, sc->header.ImageHeight),
		depth, nChannels);
		assert(sc->image != NULL);
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created image %d\n",sc->image); }
		sc->image->imageData = calloc(sizeof (char), sc->ImageBytes);
		assert(sc->image->imageData != NULL);
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created image data %d\n",sc->image->imageData); }*/
		if (opts.debug) {
			fprintf(stderr, "serCaptureFromFile: Width, Height, depth, nchannels %zd,%zd,%d,%d\n",
				sc->header.ImageWidth, sc->header.ImageHeight, depth, sc->nChannels);
		}
		sc->image = cvCreateImage(cvSize(sc->header.ImageWidth, sc->header.ImageHeight), depth, sc->nChannels);
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created Image %p\n", sc->image); }
		/*	sc->TimeStamp = calloc(sizeof (char), SER_DATETIME_SIZE);
		assert(sc->TimeStamp != NULL);
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created TimeStamp %d\n",sc->TimeStamp); }*/

		sc->image->widthStep = sc->header.ImageWidth * sc->BytesPerPixel * sc->nChannels;
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
		if (opts.debug) { fprintf(stderr, "serCaptureFromFile: Created ser %p\n", sc); }
		size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);
		sc->frame_data = calloc(im_size, sc->ImageBytes);
		sc->mat_type = sc->byte_depth == 2 ?
			sc->nChannels == 3 ? CV_16UC3 : CV_16UC1 :
			sc->nChannels == 3 ? CV_8UC3 : CV_8UC1;
		sc->current_frame = 0;
		long file_size = SER_HEADER_SIZE + sc->FrameCount * (sc->ImageBytes + 8);
		if (actual_file_size < file_size)
			sc->FrameCount = sc->header.FrameCount = frames_size / (sc->ImageBytes + 8);
		sc->big_endian_proc = (*(uint16_t *)"\0\xff" < 0x100);
		// little_endian: 0 for little endian, 1 for big endian
		sc->data_proc_same_endianness = sc->big_endian_proc == sc->header.LittleEndian;
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
											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing fh %p\n",sc->fh); }
	if (!(fclose(sc->fh)==0)) {
		fprintf(stderr, "ERROR in serReleaseCapture closing capture file\n");
		exit(EXIT_FAILURE);
	}
/*											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image %d size data %d\n", (int) (sc->image), sizeof(*sc->image->imageData)); }
free(sc->image->imageData);*/
											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image %p size image %zd\n", sc->image, sizeof(*sc->image)); }
	cvReleaseImage(&sc->image);
	sc->image=NULL;
/*											if (opts.debug) { fprintf(stderr, "serReleaseCapture: Releasing image header %d\n", (int) (sc->image)); }
	cvReleaseImageHeader(&sc->image);
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Releasing TimeStamp %d\n", (int) (sc->TimeStamp)); }
	free(sc->TimeStamp);
	sc->TimeStamp=NULL;*/
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Releasing sc %p\n", sc); }
	free(sc);
	sc=NULL;
											if (opts.debug) { 	fprintf(stderr, "serReleaseCapture: Released sc\n"); }
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
			//fprintf(stderr,"%2c ", *p++);
			output = (char*)calloc(sizeof(char), 10);
			sprintf_s(output, 10, "%c", *p++);
		}
		else {
			//fprintf(stderr,"%02X ", *p++);
			output = (char*)calloc(sizeof(char), 10);
			sprintf_s(output, 10, "%X", *p++);
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
		//fprintf(stderr,"%03d ", *p++);
		sprintf_s(output, 5, "%03d", *p++);
		OutputDebugStringA(output);
	}

}

/*****************Prints SER header***************************/			
void serPrintHeader(SerCapture *sc)
{
	char* buffer = (char*)malloc(1000);
	sprintf_s(buffer, 1000, "FileID       : %s\n", sc->header.FileID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "LuID         : %d\n", (int)sc->header.LuID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "ColorID      : %d\n", (int)sc->header.ColorID);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "LittleEndian : %d\n", (int)sc->header.LittleEndian);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "ImageWidth   : %d\n", (int)sc->header.ImageWidth);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "ImageHeight  : %d\n", (int)sc->header.ImageHeight);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "PixelDepth   : %d\n", (int)sc->header.PixelDepth);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "FrameCount   : %d\n", (int)sc->header.FrameCount);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "Observer     : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Observer, SER_OBSERVER_SIZE);
	sprintf_s(buffer, 1000, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "Instrument   : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Instrument, SER_INSTRUMENT_SIZE);
	sprintf_s(buffer, 1000, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "Telescope    : ");
	OutputDebugStringA(buffer);
	serPrintStr(sc->header.Telescope, SER_TELESCOPE_SIZE);
	sprintf_s(buffer, 1000, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "DateTime     : ");
	OutputDebugStringA(buffer);
	serPrintByte(sc->header.DateTime, SER_DATETIME_SIZE);
	sprintf_s(buffer, 1000, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "DateTimeUTC  : ");
	OutputDebugStringA(buffer);
	serPrintByte(sc->header.DateTimeUTC, SER_DATETIMEUTC_SIZE);
	sprintf_s(buffer, 1000, "\n");
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "StartTime    : %f\n", sc->StartTime_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "StartTimeUTC : %f\n", sc->StartTimeUTC_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "EndTime      : %f\n", sc->EndTime_JD);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, 1000, "EndTimeUTC   : %f\n", sc->EndTime_JD);
	OutputDebugStringA(buffer);

	free(buffer);
}

size_t serFrameRead(SerCapture* sc) {
	uint16_t* frame_data16 = (uint16_t*)sc->frame_data;
	uint8_t* frame_data8 = (uint8_t*)sc->frame_data;

	size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);

	void* temp_buffer = calloc(im_size, sc->ImageBytes);

	size_t read_values = fread(temp_buffer, 1, sc->ImageBytes, sc->fh);

	uint16_t* temp_buffer16 = (uint16_t*)temp_buffer;
	uint8_t* temp_buffer8 = (uint8_t*)temp_buffer;

	uint8_t* read_ptr8, *read_ptr8_mono;
	uint16_t* read_ptr, *read_ptr_mono;

	if (read_values == sc->ImageBytes) {

		if (sc->current_frame == 0 && sc->header.PixelDepth > 8) {
			
		}

		for (int32_t y = sc->header.ImageHeight - 1; y >= 0; --y) {

			read_ptr = &temp_buffer16[y * sc->header.ImageWidth * 3];
			read_ptr8 = &temp_buffer8[y * sc->header.ImageWidth * 3];
			read_ptr_mono = &temp_buffer16[y * sc->header.ImageWidth];
			read_ptr8_mono = &temp_buffer8[y * sc->header.ImageWidth];

			uint32_t shift_left = 16 - sc->header.PixelDepth;
			uint32_t shift_right = sc->header.PixelDepth - shift_left;

			for (int32_t x = 0; x < sc->header.ImageWidth; ++x) {

				uint16_t r, g, b;
				int32_t b_idx, g_idx, r_idx, grey_idx;

				b_idx = 3 * y * sc->header.ImageWidth + 3 * x;
				g_idx = 3 * y * sc->header.ImageWidth + 3 * x + 1;
				r_idx = 3 * y * sc->header.ImageWidth + 3 * x + 2;
				grey_idx = y * sc->header.ImageWidth + x;

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
								r += *read_ptr8;;
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
								b += *read_ptr8++;
								b = (b << shift_left) + (b >> shift_right);
								g += *read_ptr8++;
								g = (g << shift_left) + (g >> shift_right);
								r += *read_ptr8++;
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
					uint8_t r, g, b;
					if (sc->header.ColorID == SER_RGB) {
						r = *read_ptr8++;
						g = *read_ptr8++;
						b = *read_ptr8++;
						frame_data8[b_idx] = b;
						frame_data8[g_idx] = g;
						frame_data8[r_idx] = r;
					}
					else if (sc->header.ColorID == SER_BGR) {
						b = *read_ptr8++;
						g = *read_ptr8++;
						r = *read_ptr8++;
						frame_data8[b_idx] = b;
						frame_data8[g_idx] = g;
						frame_data8[r_idx] = r;
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
}

void* serQueryFrameData(SerCapture *sc, const int ignore, int *perror)
{
	size_t bytesR;
	(*perror) = 0;

	if ((sc->frame >= sc->header.FrameCount) || ((ignore) && (sc->frame >= sc->ValidFrameCount))) {
		return NULL;
	}
	sc->frame++;
	if (opts.debug) { fprintf(stderr, "serQueryFrame: Reading frame #%zd\n", sc->frame); }
	if (!(bytesR = serFrameRead(sc)))
	{
		sc->ValidFrameCount = sc->frame - 1;
		if (!ignore) {
			fprintf(stderr, "ERROR in serQueryFrame reading frame #%zd\n", sc->frame);
			exit(EXIT_FAILURE);
		} else {
			(*perror) = 1;
			fprintf(stderr, "WARNING in serQueryFrame: ignoring error reading frame #%zd and above (%zd missing till frame #%zd)\n", sc->frame, sc->header.FrameCount - sc->ValidFrameCount, sc->header.FrameCount);
		}
	}
	sc->current_frame++;
	return sc->frame_data;
}

void serFixPixelDepth(SerCapture *sc, int frame_number) {
	_fseeki64(sc->fh, SER_HEADER_SIZE + frame_number * sc->ImageBytes, SEEK_SET);

	size_t im_size = sc->byte_depth == 2 ? sizeof(uint16_t) : sizeof(uint8_t);
	void* temp_buffer = calloc(im_size, sc->ImageBytes);
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
