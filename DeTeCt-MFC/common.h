#ifndef __COMMON_H__
#define __COMMON_H__

#include <opencv/highgui.h>
//#include <opencv2/highgui/highgui.hpp>

#include <stdio.h>
#define MAX_STRING	768
#define EXT_MAX		16

#if defined(_MSC_VER)
#ifdef _WIN32
#ifndef STDIN_FILENO
#define STDIN_FILENO	0
#define STDOUT_FILENO	1
#define STDERR_FILENO	2
#endif
#define snprintf	_snprintf
#define strncasecmp	_strnicmp  
#define read 		_read
#define lseek 		_lseek 
#define open		_open
#define close 		_close 
#endif
#pragma warning(disable: 4131)
#pragma warning(disable: 4201)
#pragma warning(disable: 4214)
#pragma warning(disable: 4706)
#pragma warning(disable: 4996)
#endif

//#define min(a,b)  (((a) < (b)) ? (a) : (b))
//#define max(a,b)  (((a) > (b)) ? (a) : (b))

	/****************************************************************************************************/
	/*									Procedures and functions										*/
	/****************************************************************************************************/

#ifdef __cplusplus /* C++ prototypes */
extern "C" {
#endif
	//int		round(const float num);

	char	*mid(const char *src, size_t start, size_t length, char *dst);
	char 	*left(const char *src, size_t length, char *dst);
	char 	*right(const char *src, size_t length, char *dst);
	char 	*replace_str(char *str, char *orig, char *rep);
	int 	InStr(const char *str, const char *search);
	char* 	strrstr(const char *haystack, const char *needle);
	int 	InRstr(const char *str, const char *search);
	char 	*lcase(char *src, char *dst);
	char 	*ucase(char *src, char *dst);

	char 	*getline_ux_win(FILE *file);
	void 	init_string(char *variable);

	void 	get_fileextension(const char *src, char *dst, int max);
	void 	get_folder(const char *src, char *dst);

#ifdef __cplusplus /* C++ prototypes */
}
#endif
#endif
