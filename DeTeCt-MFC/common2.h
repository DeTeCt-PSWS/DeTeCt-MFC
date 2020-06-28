#pragma once
#include "stdafx.h"
#include <vector>

extern std::string full_version;

char *CString2char(const CString source, char *destination);
CString char2CString(const char *source, CString *destination);
std::string CString2string(const CString source);

void	StreamDeTeCtOSversions(std::wstringstream *ss);
void	GetOSversion(std::string *pos_version);

bool starts_with(const std::string& s1, const std::string& s2);
bool replace(std::string& str, const std::string& from, const std::string& to);

std::vector<std::string> read_txt(std::string path);
bool file_exists(std::string filename);
std::string dirfilename(std::string directoryname_from_path, std::string filename_from_path);
