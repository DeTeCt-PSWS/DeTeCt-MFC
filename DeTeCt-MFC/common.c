/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    COMMON: Common functions													*/
/*                                                                              */
/********************************************************************************/
#include "common.h"
//#include <stdio.h>
#include <ctype.h>

int debug_mode;

/*****************Math functions***************************/
//int		round(const float num)
//{
//	return (int)(num<0 ? (num-0.5) : (num+0.5));
//}

/*****************String extended functions***************************/
char *mid(const char *src, size_t start, size_t length, char *dst)
{
/*    size_t len = min( dstlen - 1, length);*/
 
	if (start>=MAX_STRING) {
		fprintf(stderr,"ERROR in mid: incorrect start %zd\n",start);
		exit(EXIT_FAILURE);
	}
	if ((start+length>=MAX_STRING)) {
		fprintf(stderr,"ERROR in mid: incorrect length %zd\n",start+length);
		exit(EXIT_FAILURE);
	}
    strncpy(dst, src + start, length);
/* zero terminate because strncpy() didn't ?  */
/*    if(len < length) {*/
        dst[length] = '\0';
/*	}*/
	return dst;
}

char *left(const char *src, size_t length, char *dst)
{
	if (length>=MAX_STRING) {
		fprintf(stderr,"ERROR in left: incorrect length %zd\n",length);
		exit(EXIT_FAILURE);
	}
	if (length<strlen(src)) {
		mid(src, 0, length,dst);
	} else {
		strcpy(dst, src);
	}
	
	return dst;
}

char *right(const char *src, size_t length, char *dst)
{
	if (length>=MAX_STRING) {
		fprintf(stderr,"ERROR in right: incorrect length %zd\n",length);
		exit(EXIT_FAILURE);
	}
	if (length<strlen(src)) {
		mid(src, strlen(src)-length, length,dst);
	} else {
		strcpy(dst, src);
	}
	
	return dst;
}

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[MAX_STRING];
  char *p;

  if(!(p = strstr(str, orig)))  // Is 'orig' even in 'str'?
    return str;

  strncpy(buffer, str, p-str); // Copy characters from 'str' start to 'orig' st$

  sprintf(buffer+(p-str), "%s%s%c", rep, p+strlen(orig),'\0');
  
  return buffer;
}

int InStr(const char *str, const char *search)
{
	if (strstr(str,search)==NULL) {
		return -1;
	} else {
		return (int) (strstr(str,search)-str);
	}
}

char* strrstr(const char *haystack, const char *needle)
{
	char *r = NULL;

	if (!needle[0])
		return (char*)haystack + strlen(haystack);
	while (1) {
		char *p = strstr(haystack, needle);
		if (!p)
			return r;
		r = p;
		haystack = p + 1;
	}
}

int InRstr(const char *str, const char *search)
{
	if (strrstr(str,search)==NULL) {
		return -1;
	} else {
		return (int) (strrstr(str,search)-str);
	}
}

void init_string(char *variable)
{
#pragma loop(hint_parallel(0))
	for (int i=0; i<MAX_STRING; i++) {
		variable[i]='\0';
	}
} 	 	

char *lcase(char *src, char *dst)
{
	int len;

	len = strlen(src);

	if (len>=MAX_STRING) {
		fprintf(stderr,"ERROR in lcase: incorrect length %zd\n",strlen(src));
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < len; i++) {
		dst[i] = (char) tolower(src[i]);
	}	
	return dst;
}

char *ucase(char *src, char *dst)
{
	int len;
	len = strlen(src);
	
	if (len>=MAX_STRING) {
		fprintf(stderr,"ERROR in ucase: incorrect length %zd\n",strlen(src));
		exit(EXIT_FAILURE);
	}
	for (int i = 0; i < len; i++) {
		dst[i] = (char) toupper(src[i]);
	}	
	return dst;
}

/*****************File extended functions***************************/

char *getline_ux_win(FILE *file)
{
	static char buffer[MAX_STRING];
	char carac;
	int nb_carac;
/* \r char(13) \n char(10) */
	char CR=13;
	char LF=10;

	strcpy(buffer,"");
	nb_carac=0;
	carac=(char) fgetc(file);
	while ((carac!=LF) && (!feof(file))) {
		if ((carac>0) && (carac!=CR)) {
			buffer[nb_carac]=carac;
			nb_carac++;
		}
		carac=(char) fgetc(file);
	}
	for (int i=nb_carac; i<MAX_STRING; i++) {
		buffer[i]='\0';
	}

	if (nb_carac>0) {
		return buffer;
	} else {
		return "";
	}
}

void get_fileextension(const char *src, char *dst, int max)
{
	char *ext;
	
	ext = strrchr(src, '.');
	
	if (!ext) {
		*dst = '\0';
	} else {
		strncpy(dst, ++ext, max - 1);
		dst[max-1] = '\0';
	}
}

void get_folder(const char *src, char *dst)
{
	char *dir;
	
	dir = strrchr(src, '\\');
	
	if (!dir) {
		*dst = '.';
	} else {
		strncpy(dst, src, strlen(src)-strlen(dir));
		dst[strlen(src)-strlen(dir)+1] = '\0';
	}
}
