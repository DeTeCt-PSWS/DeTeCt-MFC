#include "common2.h"

//#include "dtcgui.hpp"
#include <opencv2\highgui\highgui.hpp>

extern "C" {
#include "common.h"
}

#include <fstream>
#include <sstream>

char *CString2char(const CString source, char *destination) {
	CT2A tmp(source);
	strcpy(destination, tmp);
	destination[strlen(tmp)] = '\0';
	return destination;
}

CString char2CString(const char *source, CString *destination) {
	CString tmp(source);
	(*destination) = tmp;
	return (*destination);
}

std::string CString2string(const CString source) {
	// Convert a TCHAR string to a LPCSTR
	CT2CA ConvertedAnsiString(source);
	// construct a std::string using the LPCSTR input
	std::string CString_string(ConvertedAnsiString);

	return CString_string;
}

void StreamDeTeCtOSversions(std::wstringstream *ss)
{
	std::string os_version = "";

	(*ss) << full_version.c_str();
	(*ss) << " running on ";
	GetOSversion(&os_version);
	(*ss) << os_version.c_str() << " OS";
}

void GetOSversion(std::string *pos_version)
{
	OSVERSIONINFOEX os_info;
	ZeroMemory(&os_info, sizeof(OSVERSIONINFOEX));
	os_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&os_info)) {
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 0)
			pos_version->append("Win2000");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 1)
			pos_version->append("WinXP");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) == 0)
			pos_version->append("WinServer2003");
		if (os_info.dwMajorVersion == 5 && os_info.dwMinorVersion == 2 && GetSystemMetrics(SM_SERVERR2) != 0)
			pos_version->append("WinServer2003R3");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 0 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("WinVista");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 0 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2008");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 1 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win7");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 1 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2008R2");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 2 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win8(or_above)");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 2 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2012(or_above)");
		// Following works only for applications that have been manifested for Windows 8.1 or Windows 10
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 3 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win8.1");
		if (os_info.dwMajorVersion == 6 && os_info.dwMinorVersion == 3 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2012R2");
		if (os_info.dwMajorVersion == 10 && os_info.dwMinorVersion == 0 && os_info.wProductType == VER_NT_WORKSTATION)
			pos_version->append("Win10");
		if (os_info.dwMajorVersion == 10 && os_info.dwMinorVersion == 0 && os_info.wProductType != VER_NT_WORKSTATION)
			pos_version->append("WinServer2016");
	}
	else {
		pos_version->append("Unknown");
	}
#if _WIN64
	pos_version->append("_64b");
#elif _WIN32
	BOOL isWow64 = FALSE;
	typedef BOOL(WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);
	LPFN_ISWOW64PROCESS fnIsWow64Process = (LPFN_ISWOW64PROCESS)
		GetProcAddress(GetModuleHandle(TEXT("kernel32")), "IsWow64Process");
	if ((fnIsWow64Process) && (fnIsWow64Process(GetCurrentProcess(), &isWow64)))
		if (isWow64)
			pos_version->append("_64b");
		else
			pos_version->append("_32b");
#endif
}

// ************************************************************
// ************** General functions ***************************
// ************************************************************

bool starts_with(const std::string& s1, const std::string& s2) {
	return s2.size() <= s1.size() && s1.compare(0, s2.size(), s2) == 0;
}

bool replace(std::string& str, const std::string& from, const std::string& to) {
	size_t start_pos = str.find(from);
	if (start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}


std::vector<std::string> read_txt(std::string path) {
	std::ifstream file(path);
	std::string line;
	std::vector<std::string> lines;
	while (std::getline(file, line)) {
		lines.push_back(line);
	}
	return lines;
}

/**** return if file with filename exists ***/
bool file_exists(std::string filename) {
	std::ifstream filetest(filename);
	if (!filetest) return false;
	filetest.close();
	return true;
}

/**** return full filename constructed from first file directory and second file short filename ***/
std::string dirfilename(std::string directoryname_from_path, std::string filename_from_path) {
	return directoryname_from_path.substr(0, directoryname_from_path.find_last_of("\\") + 1) + filename_from_path.substr(filename_from_path.find_last_of("\\") + 1, filename_from_path.length());
}