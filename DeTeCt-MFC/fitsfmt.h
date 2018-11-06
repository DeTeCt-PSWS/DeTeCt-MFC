#ifndef __FITSFMT_H__
#define __FITSFMT_H__
#include "common.h"

#include <opencv/highgui.h>
#include "dirent.h"
#include "filefmt.h"

/****************************************************************************************************/
/*									Procedures and functions										*/
/****************************************************************************************************/
	
size_t	fitsImageRead(void *image, const size_t size, const size_t num, FILE *f);
void 	fitsGet_info(FileCapture *fc, const char *fname, double *date);
double 	fitsJD_date(char *buffer);

#endif /* __FITSFMT_H__ */
