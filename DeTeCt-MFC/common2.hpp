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

class MyCallback : public IBindStatusCallback
{
public:
	MyCallback() {}

	~MyCallback() {}

#pragma warning(disable:4100)
	// This one is called by URLDownloadToFile
	STDMETHOD(OnProgress)(/* [in] */ ULONG ulProgress, /* [in] */ ULONG ulProgressMax, /* [in] */ ULONG ulStatusCode, /* [in] */ LPCWSTR wszStatusText)
	{
		// You can use your own logging function here
		wprintf(L"Downloaded %d of %d. Status code %d\n", ulProgress, ulProgressMax, ulStatusCode);
		return S_OK;
	}

	STDMETHOD(OnStartBinding)(/* [in] */ DWORD dwReserved, /* [in] */ IBinding __RPC_FAR* pib)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetPriority)(/* [out] */ LONG __RPC_FAR* pnPriority)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnLowResource)(/* [in] */ DWORD reserved)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnStopBinding)(/* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(GetBindInfo)(/* [out] */ DWORD __RPC_FAR* grfBINDF, /* [unique][out][in] */ BINDINFO __RPC_FAR* pbindinfo)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnDataAvailable)(/* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC __RPC_FAR* pformatetc, /* [in] */ STGMEDIUM __RPC_FAR* pstgmed)
	{
		return E_NOTIMPL;
	}

	STDMETHOD(OnObjectAvailable)(/* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown __RPC_FAR* punk)
	{
		return E_NOTIMPL;
	}

	// IUnknown stuff
	STDMETHOD_(ULONG, AddRef)()
	{
		return 0;
	}

	STDMETHOD_(ULONG, Release)()
	{
		return 0;
	}

	STDMETHOD(QueryInterface)(/* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR* __RPC_FAR* ppvObject)
	{
		return E_NOTIMPL;
	}
#pragma warning(default:4100)
};

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
bool						URLUncachedDownloadToFile(CString URL, CString DownloadedFilename);

std::vector<std::string>	read_txt(std::string path);
std::string					dirfilename(std::string directoryname_from_path, std::string filename_from_path);
bool						rmdir_force(const char* directory_name);
int							NbWaitedUnlockedFile(CString filename, const int delay);
int64						filesize(const char* filename);

double						round_precision(const double number, const int precision);