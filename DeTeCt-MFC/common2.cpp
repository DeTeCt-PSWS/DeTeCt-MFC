#include "common2.h"

//#include "dtcgui.hpp"
#include <opencv2\highgui\highgui.hpp>
#include <codecvt>
//#include "versionhelpers.h"	//for IsWindowsxxx

extern "C" {
#include "common.h"
}

#include <fstream>
#include <sstream>
#include "dirent.h"
#include <direct.h>


// ************************************************************
// ***************** String functions *************************
// ************************************************************

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

std::string wstring2string(const std::wstring& wstr)
{
	using convert_typeX = std::codecvt_utf8<wchar_t>;
	std::wstring_convert<convert_typeX, wchar_t> converterX;

	return converterX.to_bytes(wstr);
}

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
	file.close();
	return lines;
}

/*std::wstring wstring_tolower(std::wstring& wstr) {
	std::for_each(wstr.begin(), wstr.end(), [](char& c) {
		c = (char) ::tolower(c);
	});
}*/


// ************************************************************
// ******************* OS functions ***************************
// ************************************************************

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
	
	/*if		(IsWindowsServer())				pos_version->append("WinServer");
	else if (IsWindows10OrGreater())		pos_version->append("Win10");
	else if (IsWindows8Point1OrGreater())	pos_version->append("Win8.1");
	else if (IsWindows8Point1OrGreater())	pos_version->append("Win8");
	else if (IsWindows7SP1OrGreater())		pos_version->append("Win7SP1");
	else if (IsWindows7OrGreater())			pos_version->append("Win7");
	else if (IsWindowsVistaSP2OrGreater())	pos_version->append("WinVistaSP2");
	else if (IsWindowsVistaSP1OrGreater())	pos_version->append("WinVistaSP1");
	else if (IsWindowsVistaOrGreater())		pos_version->append("WinVista");
	else if (IsWindowsXPSP3OrGreater())		pos_version->append("WinXPSP3");
	else if (IsWindowsXPSP2OrGreater())		pos_version->append("WinXPSP2");
	else if (IsWindowsXPSP1OrGreater())		pos_version->append("WinXPSP1");
	else if (IsWindowsXPOrGreater())		pos_version->append("WinXP");*/

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
// **************** File functions ****************************
// ************************************************************


/**** return if file with filename exists ***/
bool file_exists(std::string filename) {
	std::ifstream filetest(filename);
	if (!filetest) {
		filetest.close();
		return false;
	}
	filetest.close();
	return true;
}

/**** return full filename constructed from first file directory and second file short filename ***/
std::string dirfilename(std::string directoryname_from_path, std::string filename_from_path) {
	return directoryname_from_path.substr(0, directoryname_from_path.find_last_of("\\") + 1) + filename_from_path.substr(filename_from_path.find_last_of("\\") + 1, filename_from_path.length());
}

bool duplicate_txtfile(const CString InputFileName, const CString OutputFileName) {
	std::ifstream InputFile(InputFileName, std::ifstream::in);
	std::ofstream OutputFile(OutputFileName, std::ifstream::out);

	if (InputFile && OutputFile) {
		std::vector<std::string> lines;
		std::string line;

		while (std::getline(InputFile, line)) OutputFile << line << "\n";
		InputFile.close();
		OutputFile.close();

		return TRUE;
	}
	else {
		InputFile.close();
		OutputFile.close();
		return FALSE;
	}
}

bool rmdir_force(const char *directory_name) {
	DIR*	directory;
	struct dirent* entry;
	bool	status = TRUE;
	int		status_int = 0;
	bool	return_value = TRUE;
	char	filename[MAX_STRING];
	char	dirname[MAX_STRING];

	if (!(directory = opendir(directory_name))) {
		closedir(directory);
		return FALSE;
	}
	if (!(entry = readdir(directory))) {
		closedir(directory);
		return FALSE;
	}
	do {
		if (!(strcmp(entry->d_name, ".") == 0) && !(strcmp(entry->d_name, "..") == 0)) {
			if (entry->d_type == DT_DIR) {				//directory
//				status = rmdir_force(entry->d_name);
				strcpy(dirname, directory_name);
				strcat(dirname, "\\");
				strcat(dirname, entry->d_name);
				strcat(dirname, "\0");
				status_int = rmdir_force(dirname);
				if (!status) return_value = FALSE;
			}
			else {										//file
//				status_int = remove(entry->d_name);
				strcpy(filename, directory_name);
				strcat(filename, "\\");
				strcat(filename, entry->d_name);
				strcat(filename, "\0");
				status_int = remove(filename);
				if (status_int <0) return_value = FALSE;
			}
		}
	} while (entry = readdir(directory));
	status_int = _rmdir(directory_name);
	if (status_int<0) return_value = FALSE;

	return return_value;
}