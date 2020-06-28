#ifndef __FILEFMT_H__
#define __FILEFMT_H__
//#include "common.h"


//#include <opencv2\core\core.hpp>
//#include <opencv2\highgui\highgui.hpp> 

//#include <opencv/highgui.h>
	#include "dirent.h"

	struct _FileCapture
	{
		int FileType;

		FILE *fh;
		int frame;
		IplImage *image;
		//cv::Mat image;
		size_t ImageBytes;
		size_t BytesPerPixel;
		int ImageWidth;		// pixels
		int ImageHeight;	// pixels
		int PixelDepth;		// <= 8 (BytePerPixel==1), >8 (BytePerPixel==2)
		int MaxBits;	// real maximum number of bits (eg could be 14 bits instead of 16 bits)
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
	typedef struct _FileCapture FileCapture;


	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

	FileCapture	*FileCaptureFromFile(const char *fname, int *pframecount, const int capture_type);
	void 		fileReinitCaptureRead(FileCapture *fc, const char *fname);
	IplImage 	*fileQueryFrame(FileCapture *fc, const int ignore, int *perror);
	void 		fileGet_info(FileCapture *fc, const char *fname, double *date);
	void 		fileReleaseCapture(FileCapture *fc);
	void 		fileGenerate_filename(char *dest, FileCapture *fc, int nb);
	void 		fileGet_filename(char *dest, FileCapture *fc, int nb);
	void 		fileGenerate_number(char *dest, FileCapture *fc, int nb);

#endif /* __FILEFMT_H__ */
