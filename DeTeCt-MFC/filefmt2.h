#pragma once
#ifndef __FILEFMT2_H__
#define __FILEFMT2_H__
//#include "common.h"
//#include "dirent.h"

//#include <opencv2\core\core.hpp>
//#include <opencv2\highgui\highgui.hpp> 

/**********************************************************************************************//**
 * @struct	_FileCapture
 *
 * @brief	A file capture. Same structure as the old version, changed to the new OpenCV API.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/



 // Deactivation Marc 2020.06.02

struct _FileCapture2
{
	int FileType;

	FILE *fh;
	int frame;
	cv::Mat image;
	size_t ImageBytes;
	size_t BytesPerPixel;
	int ImageWidth;  // pixels
	int ImageHeight; // pixels
	int PixelDepth; // <= 8 (BytePerPixel==1), >8 (BytePerPixel==2)
	unsigned int ColorID;
	size_t header_size;

	size_t FrameCount;
	size_t ValidFrameCount;

	double StartTime_JD;
	double StartTimeUTC_JD;
	double EndTime_JD;
	double EndTimeUTC_JD;

	int	NumberPos;
	int LeadingZeros;
	int FirstFileIdx;
	int LastValidFileIdx;
	int LastFileIdx;
	char filename_rac[MAX_STRING];
	char filename_head[MAX_STRING];
	char filename_trail[MAX_STRING];
	char filename_ext[EXT_MAX];
	char filename_folder[MAX_STRING];
};
typedef struct _FileCapture2 FileCapture2;



/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/

//FileCapture	*FileCaptureFromFile(const char *fname, int *pframecount, const int capture_type);
//void 		fileReinitCaptureRead(FileCapture *fc, const char *fname);
//cv::Mat 	fileQueryFrame(FileCapture *fc, const int ignore, int *perror);
//void 		fileGet_info(FileCapture *fc, const char *fname, double *date);
//void 		fileReleaseCapture(FileCapture *fc);
//void 		fileGenerate_filename(char *dest, FileCapture *fc, int nb);

 // Deactivation Marc 2020.06.02

void 		fileGet_filename(char *dest, FileCapture2 *fc, int nb);
void 		fileGenerate_number(char *dest, FileCapture2 *fc, int nb);


#endif /* __FILEFMT_H2__ */