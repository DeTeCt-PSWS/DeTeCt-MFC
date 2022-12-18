/********************************************************************************/
/*                                                                              */
/*	DTC	(c) Marc Delcroix (delcroix.marc@free.fr) 2012-							*/
/*                                                                              */
/*    COMMON: Common functions													*/
/*                                                                              */
/********************************************************************************/
//#include "common.h"
#include "dtc.h"
#include <windows.h>
#include <ctype.h>

int debug_mode;

OPTS opts;

/*****************Math functions***************************/
//int		round(const float num)
//{
//	return (int)(num<0 ? (num-0.5) : (num+0.5));
//}

/*****************String extended functions***************************/
char *mid(const char *src, size_t start, size_t length, char *dst)
{
    size_t len_valid = length;

	if (start>=MAX_STRING) {
		char msgtext[MAX_STRING] = { 0 };
		sprintf(msgtext,"incorrect start %zd for %s\n",start, src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string start", __func__, msgtext);
		strcpy(dst,"");
		return dst;
	}
	if ((start+len_valid>=MAX_STRING)) {
		char msgtext[MAX_STRING] = { 0 };
		sprintf(msgtext,"incorrect length %zd for %s, truncating it\n",start+len_valid, src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string length", __func__, msgtext);
		len_valid = MAX_STRING - start -1;
	}
    strncpy(dst, src + start, len_valid);
/* zero terminate because strncpy() didn't ?  */
/*    if(len < length) {*/
        dst[length] = '\0';
/*	}*/
	return dst;
}

char *left(const char *src, size_t length, char *dst)
{
    size_t len_valid = length;

	if (len_valid>=MAX_STRING) {
		char msgtext[MAX_STRING] = { 0 };
		sprintf(msgtext,"incorrect length %zd for %s, truncating it\n",len_valid,src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string length", __func__, msgtext);
		len_valid = MAX_STRING - 1;
	}
	if (length<strlen(src)) {
		mid(src, 0, len_valid,dst);
	} else {
		strcpy(dst, src);
	}
	
	return dst;
}

char *right(const char *src, size_t length, char *dst)
{
    size_t len_valid = length;

	if (len_valid>=MAX_STRING) {
		char msgtext[MAX_STRING] = { 0 };
		sprintf(msgtext,"incorrect length %zd for %s, truncating it\n",len_valid, src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string length", __func__, msgtext);
		len_valid = MAX_STRING - 1;
	}
	if (length<strlen(src)) {
		mid(src, strlen(src)-len_valid, len_valid,dst);
	} else {
		strcpy(dst, src);
	}
	
	return dst;
}

char *trim(const char* src, char *dst)
{
	// Initialize considering no trimming required.
	int atstart = 0;
	int atend = (int) (strlen(src) - 1);
	// Spaces at start
	for (int i = 0; i < strlen(src); i++) {
		if (src[i] != ' ') {
			atstart = i;
			break;
		}
	}
	// Spaced from end
	for (int i = (int)(strlen(src) - 1); i >= 0; i--) {
		if (src[i] != ' ') {
			atend = i;
			break;
		}
	}
	mid(src, atstart, atend - atstart, dst);
	/* zero terminate because mid() didn't ?  */
	//dst[atend - atstart + 1] = '\0'; 

	return dst;
}

char *replace_str(char *str, char *orig, char *rep)
{
  static char buffer[MAX_STRING] = { 0 };
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

char *lcase(const char *src, char *dst)
{
	size_t len_valid = strlen(src);

	if (len_valid>=MAX_STRING) {
		char msgtext[MAX_STRING] = { 0 };
		sprintf(msgtext,"incorrect length %zi for %s, truncating it\n",len_valid, src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string length", __func__, msgtext);
		len_valid = MAX_STRING - 1;
	}
	for (int i = 0; i < len_valid; i++) {
		dst[i] = (char) tolower(src[i]);
	}	
	return dst;
}

char *ucase(const char *src, char *dst)
{
	size_t len_valid = strlen(src);
	
	if (len_valid>=MAX_STRING) {
		char msgtext[MAX_STRING] = { 0 };
		snprintf(msgtext,MAX_STRING, "incorrect length %zi for %s, truncating it\n",len_valid, src);
		Warning(WARNING_MESSAGE_BOX, "incorrect string length", __func__, msgtext);
		len_valid = MAX_STRING - 1;
	}
	for (int i = 0; i < len_valid; i++) {
		dst[i] = (char) toupper(src[i]);
	}	
	return dst;
}

char* str_trail_fill(const char* src, const char *character, const int size, char* dst)
{
	size_t len_valid = strlen(src);

	init_string(dst);
	strcpy(dst, src);
	if (size > len_valid) {
		for (int i = 0; i < (size - len_valid); i++) {
			strcat(dst, character);
		}
	}
	return dst;
}

LPWSTR char2LPWSTR(const char* text, LPWSTR *LPWSTR_text) {
	wchar_t wtext[MAX_STRING];
	mbstowcs(wtext, text, strlen(text) + 1);
	(*LPWSTR_text) = wtext;
	return (*LPWSTR_text);
}


/*****************File extended functions***************************/

char *getline_ux_win(FILE *file)
{
	static char buffer[MAX_STRING] = { 0 };
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

bool file_exists(const char* fname)
{
	FILE* file;
	if ((file = fopen(fname, "r")))
	{
		fclose(file);
		return true;
	}
	return false;
}

void	ErrorExit(const bool display_msgbox, const char *title, const char *function, const char *text) {	
	char fulltext[MAX_STRING];
	char fulltitle[MAX_STRING];
	char buffer[MAX_STRING] = { 0 };

	if (strlen(function)>0) {
		sprintf_s(buffer, MAX_STRING, "Error in %s(): %s\n", function, text);
		fprintf(stderr, "%s", buffer);
		snprintf(fulltext, MAX_STRING, "%s\nWill now exit program", buffer);
	} else {
		sprintf_s(buffer, MAX_STRING, "Error: %s\n", text);
		fprintf(stderr, "%s", buffer);
		sprintf(fulltext, "%s\nWill now exit program", buffer);
	}
	OutputDebugStringA(buffer);
	if (strlen(opts.ErrorsFilename) > 1) {
		FILE* logfile = fopen(opts.ErrorsFilename, "a");
		fprintf(logfile, buffer);
		fclose(logfile);
	}
	snprintf(fulltitle, MAX_STRING, "Error: %s", title);
//	if (display_msgbox && (opts.parent_instance || (opts.maxinstances == 1))) {
	if (display_msgbox) {
		wchar_t wfulltitle[MAX_STRING];
		mbstowcs(wfulltitle, fulltitle, strlen(fulltitle) + 1);
		wchar_t wfulltext[MAX_STRING];
		mbstowcs(wfulltext, fulltext, strlen(fulltext) + 1);
		MessageBox(NULL, wfulltext, wfulltitle, MB_OK + MB_ICONERROR + MB_SETFOREGROUND + MB_TOPMOST);
	}
	exit(EXIT_FAILURE);
}


void	Warning(const bool display_msgbox, const char* title, const char* function, const char* text) {
	char fulltext[MAX_STRING];
	char fulltitle[MAX_STRING];
	char buffer[MAX_STRING] = { 0 };

	if (strlen(function) > 0) {
		sprintf_s(buffer, MAX_STRING, "Warning in %s(): %s\n", function, text);
		fprintf(stderr, "%s", buffer);
		snprintf(fulltext, MAX_STRING, "%s\nWill now continue program", buffer);
	}
	else {
		sprintf_s(buffer, MAX_STRING, "Warning: %s\n", text);
		fprintf(stderr, "%s", buffer);
		snprintf(fulltext, MAX_STRING, "%s\nWill now continue program", buffer);
	}
	OutputDebugStringA(buffer);
	if (strlen(opts.WarningsFilename) > 1) {
		FILE* logfile = fopen(opts.WarningsFilename, "a");
		fprintf(logfile, buffer);
		fclose(logfile);
	}
	snprintf(fulltitle, MAX_STRING, "Warning: %s", title);
	//	if (display_msgbox && (opts.parent_instance || (opts.maxinstances == 1))) {
	if (display_msgbox) {
		wchar_t wfulltitle[MAX_STRING];
		mbstowcs(wfulltitle, fulltitle, strlen(fulltitle) + 1);
		wchar_t wfulltext[MAX_STRING];
		mbstowcs(wfulltext, fulltext, strlen(fulltext) + 1);
		MessageBox(NULL, wfulltext, wfulltitle, MB_OK + MB_ICONWARNING + MB_SETFOREGROUND + MB_TOPMOST);
	}
}

void	Info(const bool display_msgbox, const char* title, const char* function, const char* text) {
	char fulltext[MAX_STRING];
	char fulltitle[MAX_STRING];
	char buffer[MAX_STRING] = { 0 };

	if (strlen(function) > 0) {
		sprintf_s(buffer, MAX_STRING, "Info in %s(): %s\n", function, text);
		fprintf(stderr, "%s", buffer);
		snprintf(fulltext, MAX_STRING, "%s\nWill now continue program", buffer);
	}
	else {
		sprintf_s(buffer, MAX_STRING, "Info: %s\n", text);
		fprintf(stderr, "%s", buffer);
		snprintf(fulltext, MAX_STRING, "%s\nWill now continue program", buffer);
	}
	OutputDebugStringA(buffer);
	if (strlen(opts.WarningsFilename) > 1) {
		FILE* logfile = fopen(opts.WarningsFilename, "a");
		fprintf(logfile, buffer);
		fclose(logfile);
	}
	snprintf(fulltitle, MAX_STRING, "Info: %s", title);
	//	if (display_msgbox && (opts.parent_instance || (opts.maxinstances == 1))) {
	if (display_msgbox) {
		wchar_t wfulltitle[MAX_STRING];
		mbstowcs(wfulltitle, fulltitle, strlen(fulltitle) + 1);
		wchar_t wfulltext[MAX_STRING];
		mbstowcs(wfulltext, fulltext, strlen(fulltext) + 1);
		MessageBox(NULL, wfulltext, wfulltitle, MB_OK + MB_ICONINFORMATION + MB_SETFOREGROUND + MB_TOPMOST);
	}
}
