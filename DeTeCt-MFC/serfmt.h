/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Luis Calderon, Marc Delcroix (delcroix.marc@free.fr) 2012			*/
/*                                                                              */
/*    SERFMT: SER format functions                                              */
/*                                                                              */
/********************************************************************************/

#ifndef __SERFMT_H__
#define __SERFMT_H__


#include "common.h"
//#include <stdint.h>
#include <stdbool.h>
//#include <opencv2\highgui\highgui.hpp>
//#include <opencv2\imgproc\imgproc.hpp>
//#include <opencv2\core\core.hpp>

// SER format FileID
#define SER_FILEID		"LUCAM-RECORDER"

// SER format ColorID
#define SER_MONO				0
#define SER_BAYER_RGGB			8
#define SER_BAYER_GRBG			9
#define SER_BAYER_GBRG			10
#define SER_BAYER_BGGR			11
#define SER_BAYER_CYYM			16
#define SER_BAYER_YCMY			17
#define SER_BAYER_YMCY			18
#define SER_BAYER_MYYC			19
#define SER_RGB					100
#define SER_BGR					101

// SER format byte order i 16 bit pixel format
#define SER_BIG_ENDIAN			0
#define SER_LITTLE_ENDIAN		1

#define SER_FILEID_SIZE			14	// 	0
#define SER_LUID_SIZE			4	//	14
#define SER_COLORID_SIZE		4	//	18
#define SER_LITTLEENDIAN_SIZE	4	//	22
#define SER_IMAGEWIDTH_SIZE		4	//	26
#define SER_IMAGEHEIGHT_SIZE	4	//	30
#define SER_PIXELDEPTH_SIZE		4	//	34	
#define SER_FRAMECOUNT_SIZE		4	//	38
#define SER_OBSERVER_SIZE		40	//	42
#define SER_INSTRUMENT_SIZE		40	//	82
#define SER_TELESCOPE_SIZE		40	//	122
#define SER_DATETIME_SIZE		8	//	160
#define SER_DATETIMEUTC_SIZE	8	//	168
									//	176

#define SER_HEADER_SIZE	SER_FILEID_SIZE+SER_LUID_SIZE+SER_COLORID_SIZE+\
						SER_LITTLEENDIAN_SIZE+SER_IMAGEWIDTH_SIZE+     \
						SER_IMAGEHEIGHT_SIZE+SER_PIXELDEPTH_SIZE+      \
						SER_FRAMECOUNT_SIZE+SER_OBSERVER_SIZE+         \
						SER_INSTRUMENT_SIZE+SER_TELESCOPE_SIZE+        \
						SER_DATETIME_SIZE+SER_DATETIMEUTC_SIZE

#define SER_MAX_FIELD_SIZE		40

struct _SerHeader
{
	char FileID[SER_FILEID_SIZE];	// LUCAM-RECORDER
	unsigned int LuID;	// Lumenera camera series ID
	unsigned int ColorID;
	unsigned int LittleEndian;
	size_t ImageWidth;  // pixels
	size_t ImageHeight; // pixels
	size_t PixelDepth; // <= 8 (BytePerPixel==1), >8 (BytePerPixel==2)
	size_t FrameCount;
	char Observer[SER_OBSERVER_SIZE];
	char Instrument[SER_INSTRUMENT_SIZE];
	char Telescope[SER_TELESCOPE_SIZE];
	unsigned char DateTime[SER_DATETIME_SIZE];
	unsigned char DateTimeUTC[SER_DATETIMEUTC_SIZE];
};
typedef struct _SerHeader SerHeader;

struct _SerCapture
{
	FILE *fh;
	size_t frame;
	size_t TimeStamp_frame;
	IplImage *image;
	unsigned char TimeStamp[SER_DATETIME_SIZE];
	size_t ImageBytes;
	size_t BytesPerPixel;
	SerHeader header;
	int	TimeStampExists;
	size_t FrameCount;
	size_t ValidFrameCount;
	double StartTime_JD;
	double StartTimeUTC_JD;
	double EndTime_JD;
	double EndTimeUTC_JD;
	int nChannels;
	int mat_type;
	int byte_depth;
	void* frame_data;
	int current_frame;
	bool big_endian_proc;
	bool data_proc_same_endianness;
};
typedef struct _SerCapture SerCapture;

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/
	
SerCapture 		*serCaptureFromFile(const char *fname);
void 			serReinitCaptureRead(SerCapture *sc,const char *fname);
void 			serReadTimeStamps(SerCapture *sc);
unsigned char 	*serQueryTimeStamp(SerCapture *sc);
void 			serReleaseCapture(SerCapture *sc);

size_t 			serTimeStampRead(unsigned char *pTimeStamp, const size_t size, const size_t num, FILE *f);

double 			serDateTime_JD(unsigned char *headerfield);

void 			serPrintStr(char *p, int sz);
void 			serPrintHeader(SerCapture *sc);

/*
*  Additions by Jon -- works with SER v3 as of January 2018
*  serFrameRead replaces serImageRead
*  serQueryFrameData replaces serQueryFrame
*  The matrix is now created in wrapper2.cpp
*/
size_t				serFrameRead(SerCapture* sc);
void				*serQueryFrameData(SerCapture *sc, const int ignore, int *perror);
void				serFixPixelDepth(SerCapture *sc, int frame_number);

#endif /* __SERFMT_H__ */