#pragma once


#include "stdafx.h"
#include "dtc.h"
#include <vector>
//#include <string> // not needed
#include "opencv2/core/ocl.hpp"

#define FILEACCESS_WAIT_MS 50

#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental\filesystem>
namespace filesys = std::experimental::filesystem;

//using namespace std;
//template <typename T>

extern std::string full_version;
//extern "C" OPTS opts;
extern OPTS opts;

char						*CString2char(const CString source, char *destination);
CString						char2CString(const char *source, CString *destination);
std::string					CString2string(const CString source);
std::string					wstring2string(const std::wstring& wstr);
bool						starts_with(const std::string& s1, const std::string& s2);
bool						replace(std::string& str, const std::string& from, const std::string& to);
void						lowercase_string(std::string* source);
void						trim_string(std::string& line);
bool						duplicate_txtfile(const CString InputFileName, const CString OutputFileName);
CString						GetLine(HANDLE QueueFileHandle);
std::string					StringPlural(const int number);

void						StreamDeTeCtOSversions(std::wstringstream *ss);
void						GetOSversion(std::string *pos_version);

std::vector<std::string>	read_txt(std::string path);
std::string					dirfilename(std::string directoryname_from_path, std::string filename_from_path);
bool						rmdir_force(const char* directory_name);
int							NbWaitedUnlockedFile(CString filename, const int delay);
int64						filesize(const char* filename);