/*
Source File : AutoUpdate.cpp
// https://www.codeproject.com/Articles/1205548/An-efficient-way-for-automatic-updating
Created for the purpose of demonstration for http://www.codeproject.com

Copyright 2017 Michael Haephrati, Secured Globe Inc.
See also: https://www.codeproject.com/script/Membership/View.aspx?mid=5956881

Secured Globe, Inc.
http://www.securedglobe.com

MODIFIED by MARC DELCROIX 05/2021

*/

#include "stdafx.h"
#include "AutoUpdate.h"
#include <urlmon.h>    
#include <iostream>

#pragma comment (lib, "urlmon.lib")
#pragma comment(lib, "Version.lib")

#include "dtcgui.hpp"
#include <string>
#include <fstream>
#include "common2.h"

//#pragma warning(disable:4477 4313 4840 4189)

// Utilities
BOOL SG_Run(LPWSTR FileName, std::vector<CString>* log_cstring_lines)
{
	CString commandLineArgument = GetCommandLine();
	
	LPWSTR* szArglist;
	int nArgs;
	int idx;
	std::wstring Parameters(L"");

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		wprintf(L"CommandLineToArgvW failed\n");
	}
	else for (idx = 1; idx < nArgs; idx++) {
		Parameters = std::wstring(Parameters) + std::wstring(szArglist[idx]) + std::wstring(L" ");
	}

	//wprintf(L"Called SG_Run '%s'\n", FileName);
	(*log_cstring_lines).push_back((CString)"Called SG_Run " + FileName);
	PROCESS_INFORMATION ProcessInfo; //This is what we get as an [out] parameter

	STARTUPINFO StartupInfo; //This is an [in] parameter

	ZeroMemory(&StartupInfo, sizeof(StartupInfo));
	StartupInfo.cb = sizeof StartupInfo; //Only compulsory field

	if (CreateProcess(FileName, (LPWSTR)Parameters.c_str(),
		NULL, NULL, FALSE, 0, NULL,
		NULL, &StartupInfo, &ProcessInfo))
	{
		//WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
		//wprintf(L"Success\n");
		(*log_cstring_lines).push_back((CString)"Success");
		return TRUE;
	}
	else
	{
		//wprintf(L"Failed\n");
		(*log_cstring_lines).push_back((CString)"Failed");
		return FALSE;
	}

}

CString GetFileNameFromPath(CString Path)
{
	int slash = Path.ReverseFind(L'\\');
	return(Path.Mid(slash + 1));
}

// AutoUpdate class

AutoUpdate::AutoUpdate(std::vector<CString>* log_cstring_lines)
{
	TCHAR szPath[MAX_PATH];

	if (!GetModuleFileName(NULL, szPath, MAX_PATH))
	{
		//wprintf(L"Can't find module file name (%s)\n", (long) GetLastError());
		(*log_cstring_lines).push_back((CString)"Can't find module file name (" + (CString)std::to_string(GetLastError()).c_str() + _T(")"));
		return;
	}
	SetSelfFullPath(szPath);
	SetSelfFileName(GetFileNameFromPath(szPath));
	m_VersionsFileName.Append(m_SelfFileName.Left(m_SelfFileName.ReverseFind(_T('.'))));
	m_VersionsFileName.Append(VERSIONSEXT);
	m_VersionsFullPath.Append(m_SelfFullPath.Left(m_SelfFullPath.ReverseFind(_T('\\'))+1));
	m_VersionsFullPath.Append(UPDATEPREFIX + m_VersionsFileName);

	// original
	SG_Version ver;
	if (SG_GetVersion(szPath, &ver, log_cstring_lines))	{
		CString ModifiedFileName = szPath;
		AddNextVersionToFileName(ModifiedFileName, ver);
	}
	ReplaceTempVersion(log_cstring_lines);
}

void AutoUpdate::AddNextVersionToFileName(CString& ExeFile, SG_Version ver)
{
	CString strVer;
	ver.SubRevision += 1;	// For the time being we just promote the subrevision in one but of course
							// we should build a mechanism to promote the major, minor and revision
	ExeFile = GetSelfFileName();
	ExeFile = ExeFile.Left(ExeFile.GetLength() - 4);
	strVer.Format(L"%d.%d.%d.%d", ver.Major, ver.Minor, ver.Revision, ver.SubRevision);
	ExeFile += L"."+strVer;
	ExeFile += L".exe";
	m_NextVersion = ExeFile;
}

BOOL AutoUpdate::SG_GetVersion_from_ConfigFile(SG_Version* ver, std::vector<CString>* log_cstring_lines) //added by MD
{
	MyCallback pCallback;

	CString URL = m_DownloadLink + m_VersionsFileName;
//	CString VerName = UPDATEPREFIX + m_VersionsFileName;
	CString VerName = m_VersionsFullPath;

	/**** downloads version file ***/
	DeleteUrlCacheEntry(URL);
	HRESULT hr = 0;
	hr = URLDownloadToFile(
		NULL,   // A pointer to the controlling IUnknown interface (not needed here)
		URL,
		VerName, 0,		      // Reserved. Must be set to 0.
		&pCallback); // status callback interface (not needed for basic use)
	if (SUCCEEDED(hr)) {
		std::ifstream ConfigFile(m_VersionsFullPath, std::ifstream::in);
		CT2A tmp(m_SelfFileName + ";");
		std::string tag_string(tmp);
		CString object;

		if (ConfigFile) {
			std::string line;
			object = "";
			while ((object == "") && (std::getline(ConfigFile, line))) {
				if (line.find(tag_string) != std::string::npos) {
					line.erase(line.find(tag_string), tag_string.size());
					object = line.c_str();
					if (line.find(".") != std::string::npos) {
						ver->Major = stoi(line.substr(0, line.find(".")));
						line.erase(0, line.find(".") + 1);
						if (line.find(".") != std::string::npos) {
							ver->Minor = stoi(line.substr(0, line.find(".")));
							line.erase(0, line.find(".") + 1);
							if (line.find(".") != std::string::npos) {
								ver->Revision = stoi(line.substr(0, line.find(".")));
								line.erase(0, line.find(".") + 1);
								if ((line.size() > 0) && (line.find(".") == std::string::npos)) {
									ver->SubRevision = stoi(line);
									if (opts.debug) (*log_cstring_lines).push_back((CString)"Info: Found current version in config file '" + m_VersionsFullPath + "' : " + (CString)std::to_string(ver->Major).c_str() + _T(".") + (CString)std::to_string(ver->Minor).c_str() + _T(".") + (CString)std::to_string(ver->Revision).c_str() + _T(".") + (CString)std::to_string(ver->SubRevision).c_str() + _T("."));
									ConfigFile.close();
									DeleteFile(m_VersionsFullPath);
									return TRUE;
								}
							}
						}
					}
					(*log_cstring_lines).push_back((CString)"Error: incorrect syntax for '" + m_SelfFileName + "' version (" + object + ") in configuration file " + m_VersionsFullPath);
					ConfigFile.close();
					DeleteFile(m_VersionsFullPath);
					return FALSE;
				}
			}
			(*log_cstring_lines).push_back((CString)"Error: cannot find '" + m_SelfFileName + "' in configuration file " + m_VersionsFullPath);
			ConfigFile.close();
			DeleteFile(m_VersionsFullPath);
			return FALSE;
		}
		else {
			(*log_cstring_lines).push_back((CString)"Info: cannot open configuration file " + m_VersionsFullPath);
			ConfigFile.close();
			return FALSE;
		}
	}
	else {
		(*log_cstring_lines).push_back((CString)"Info: cannot download configuration file " + m_VersionsFullPath);
		return FALSE;
	}
}

BOOL AutoUpdate::SG_GetVersion(LPWSTR ExeFile, SG_Version *ver, std::vector<CString>* log_cstring_lines)
{
	BOOL result = FALSE;
	DWORD dwDummy;
	DWORD dwFVISize = GetFileVersionInfoSize(ExeFile, &dwDummy);
	LPBYTE lpVersionInfo = new BYTE[dwFVISize];
	GetFileVersionInfo(ExeFile, 0, dwFVISize, lpVersionInfo);
	UINT uLen;
	VS_FIXEDFILEINFO *lpFfi;
	VerQueryValue(lpVersionInfo, _T("\\"), (LPVOID *)&lpFfi, &uLen);
	if (lpFfi && uLen)
	{
		DWORD dwFileVersionMS = lpFfi->dwFileVersionMS;
		DWORD dwFileVersionLS = lpFfi->dwFileVersionLS;
		delete[] lpVersionInfo;
		ver->Major = HIWORD(dwFileVersionMS);
		ver->Minor = LOWORD(dwFileVersionMS);
		ver->Revision = HIWORD(dwFileVersionLS);
		ver->SubRevision = LOWORD(dwFileVersionLS);
		if (opts.debug) {
			//wprintf(L"Info: found current version: %d.%d.%d.%d\n", ver->Major, ver->Minor, ver->Revision, ver->SubRevision);
			(*log_cstring_lines).push_back((CString)"Info: found current version: " + version_CString((*ver)) + _T("."));
		}
		result = TRUE;
	}
	else {
		//wprintf(L"Error: can't detect current version\n");
		(*log_cstring_lines).push_back((CString)"Error: can't detect current version");
	}
	return result;
}

AutoUpdate::~AutoUpdate()
{
}


CString AutoUpdate::GetSelfFullPath()
{
	return m_SelfFullPath;
}

void AutoUpdate::SetSelfFullPath(CString Path)
{
	m_SelfFullPath = Path;
}
CString AutoUpdate::GetSelfFileName()
{
	return m_SelfFileName;
}

void AutoUpdate::SetSelfFileName(CString FileName)
{
	m_SelfFileName = FileName;
}


BOOL AutoUpdate::ReplaceTempVersion(std::vector<CString>* log_cstring_lines)
{
	int tries = 5;
	if (m_SelfFileName.Left((int) (wcslen(UPDATEPREFIX))) == UPDATEPREFIX)
	{
		tempversion = true;
		//wprintf(L"Info: running a temp version\n");
		(*log_cstring_lines).push_back((CString)"Info: running a temp version");
	retry:;
		BOOL result = DeleteFile(m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))));
		if (result)
		{
			if (opts.debug) {
				//wprintf(L"Info: file '%s' deleted\n", m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))));
				(*log_cstring_lines).push_back((CString)"Info: file '" + m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))) + _T("' deleted"));
			}
			BOOL result2 = CopyFile(m_SelfFileName, m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))), FALSE);
			if (result2)
			{
				//wprintf(L"Info: file '%s' copied to '%s'\n", m_SelfFileName, m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))));
				(*log_cstring_lines).push_back((CString)"Info: file '" + m_SelfFileName + _T("' copied to '") + m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))) + _T("'"));
				if (SG_Run(m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))).GetBuffer(), log_cstring_lines))
				{
					//wprintf(L"Info: terminated %s\n",m_SelfFileName);
					(*log_cstring_lines).push_back((CString)"Info: terminated " + m_SelfFileName);
					_exit(0);
				}
			}
		}
		else
		{
			if (--tries) goto retry;
			//wprintf(L"Error: 'original version' ('%s') can't be deleted or doesn't exists\n", m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))));
			(*log_cstring_lines).push_back((CString)"Error: 'original version' ('" + m_SelfFileName.Mid((int) (wcslen(UPDATEPREFIX))) + _T("') can't be deleted or doesn't exists"));
		}
	}
	else
	{
		tempversion = false;
		if (opts.debug) {
			//wprintf(L"Info: running the normal version\n");
			(*log_cstring_lines).push_back((CString)"Info: running the normal version");
		}
	retry2:;
		BOOL result = DeleteFile(UPDATEPREFIX+ m_SelfFileName);
		if (result)
		{
			if (opts.debug) {
				//wprintf(L"Info: temp File '%s' deleted\n", UPDATEPREFIX + m_SelfFileName);
				(*log_cstring_lines).push_back((CString)"Info : temp File '" + UPDATEPREFIX + m_SelfFileName + _T("' deleted"));
			}
		}
		else
		{
			if (--tries) goto retry2;
			if (opts.debug) {
				//wprintf(L"Info: temp File '%s' can't be deleted or doesn't exist\n", UPDATEPREFIX + m_SelfFileName);
				(*log_cstring_lines).push_back((CString)"Info: temp File '" + UPDATEPREFIX + m_SelfFileName + _T("' can't be deleted or doesn't exist"));
			}
		}
	}
	return TRUE;
}

BOOL AutoUpdate::ReplaceExeVersion(const CString ExeName, const CString NewExeName, std::vector<CString>* log_cstring_lines)
{
	CString OldExeName;
	OldExeName = ExeName + OLDSUFFIX;
	char OldExeNameChar[MAX_STRING];
	char ExeNameChar[MAX_STRING];
	char NewExeNameChar[MAX_STRING];
	CString2char(ExeName, ExeNameChar);
	CString2char(OldExeName, OldExeNameChar);
	CString2char(NewExeName, NewExeNameChar);
	DeleteFile(OldExeName);
	//CString2char(_T("DeTeCt.exe_old"), NewExeNameChar);
	if (rename(ExeNameChar, OldExeNameChar) == 0) {
//	if (DeleteFile(ExeName)) {
		(*log_cstring_lines).push_back((CString)"Info: deleted old version of file " + ExeName);
		if (rename(NewExeNameChar, ExeNameChar) == 0) {
			(*log_cstring_lines).push_back((CString)"Info: updated new version of file " + ExeName);
			return TRUE;
		}
		else {
			(*log_cstring_lines).push_back((CString)"Error: cannot update new version of file " + ExeName);
			return FALSE;
		}
	}
	else {
		(*log_cstring_lines).push_back((CString)"Error: cannot delete old version of file " + ExeName);
		return FALSE;
	}
}

BOOL AutoUpdate::SG_VersionStringMatch(CString ExeFile, SG_Version *ver)
{
	BOOL result = false;
	int	major		= 0;
	int minor		= 0;
	int revision	= 0;
	int subrevision = 0;;
	//int last_char = ExeFile.GetLength();
	int i1, i2, i3, i4,i5;
	CString curString = ExeFile;
	i1 = curString.ReverseFind(L'.');
	if (i1 > -1)
	{
		CString strStart = ExeFile.Mid(0, i1);
		i2 = strStart.ReverseFind(L'.');
		curString = curString.Left(i2);
		if (i2 > -1)
		{
			CString StrSubrevision = ExeFile.Mid(i2+1, i1 - i2-1);
			subrevision = _wtoi(StrSubrevision.GetBuffer());
			i3 = curString.ReverseFind(L'.');
			curString = curString.Left(i3);
			if (i3 > -1)
			{
				CString StrRevision = ExeFile.Mid(i3+1, i2 - i3-1);
				revision = _wtoi(StrRevision.GetBuffer());
				i4 = curString.ReverseFind(L'.');
				curString = curString.Left(i4);
				if (i4 > -1)
				{
					CString StrMinor = ExeFile.Mid(i4+1, i3 - i4-1);
					minor = _wtoi(StrMinor.GetBuffer());
					i5 = curString.ReverseFind(L'.');
					curString = curString.Left(i5);
					if (i5 > -1)
					{
						CString StrMajor = ExeFile.Mid(i5 + 1, i4 - i5 - 1);
						major = _wtoi(StrMajor.GetBuffer());
					}
					if (major == ver->Major &&
						minor == ver->Minor &&
						revision == ver->Revision &&
						subrevision == ver->SubRevision)
							result = TRUE;
				}

			}

		}
	}
	return result;
}

BOOL	AutoUpdate::SG_VersionsCompare(const SG_Version ver1, const SG_Version ver2)
{
	if ((ver1.Major == ver2.Major) && (ver1.Minor == ver2.Minor) && (ver1.Revision == ver2.Revision) && (ver1.SubRevision == ver2.SubRevision)) return TRUE;
	return FALSE;
}


BOOL AutoUpdate::CheckForUpdates(std::vector<CString> *log_cstring_lines)
{
	if (tempversion) return TRUE;	// We don't check for updates if we are running a temp version
	
	// original
	TCHAR szPath[MAX_PATH];
	GetModuleFileName(NULL, szPath, MAX_PATH);
	SG_Version ver_current;
	SG_GetVersion(szPath, &ver_current, log_cstring_lines);

	//added by MD
	SG_Version ver_server;
	if (!SG_GetVersion_from_ConfigFile(&ver_server, log_cstring_lines)) return FALSE;

	if (SG_Version_number(ver_server) < SG_Version_number(ver_current)) {
		(*log_cstring_lines).push_back((CString)"Info: current version " + version_CString(ver_current) + (CString)" is more recent than update version " + version_CString(ver_server) + (CString)" !");
		return FALSE;
	}
	else if (SG_Version_number(ver_server) == SG_Version_number(ver_current)) {
		if (DeleteFile(m_SelfFullPath + OLDSUFFIX)) { // exe backup exists so update finished
			(*log_cstring_lines).push_back((CString)"Info: successful update to version " + version_CString(ver_server));
			if (opts.interactive) MessageBox(NULL, m_SelfFileName + _T(" successfully updated to version ") + version_CString(ver_server), _T("DeTeCt update"), MB_OK + MB_ICONINFORMATION + MB_SETFOREGROUND + MB_TOPMOST);
			return FALSE;
		}
		else {
			(*log_cstring_lines).push_back((CString)"Info: current version " + version_CString(ver_current) + (CString)" is already uptodate");
		}
		return FALSE;
	}

	if ((!opts.autostakkert) && (opts.parent_instance)) { // No update if autostakkert mode or child mode !
		(*log_cstring_lines).push_back((CString)"Info: updating to new version " + version_CString(ver_server));
		if (opts.interactive) MessageBox(NULL, (CString)"Updating " + m_SelfFileName + _T(" from version ") + version_CString(ver_current) + _T(" to new version ") + version_CString(ver_server), _T("DeTeCt update"), MB_OK + MB_ICONINFORMATION + MB_SETFOREGROUND);

		MyCallback pCallback;
		CString ExeName = m_SelfFullPath.Left(m_SelfFullPath.ReverseFind(_T('\\')) + 1) + UPDATEPREFIX + m_SelfFileName;

		m_NextVersion = m_SelfFileName;					// added by MD: CHANGE OF NextVersion filename - remove version in filename !!!

		CString URL = m_DownloadLink + m_NextVersion;
		//wprintf(L"Next version will be %s\n", m_NextVersion);
		//(*log_cstring_lines).push_back((CString)"Next version will be " + m_NextVersion);
		if (m_NextVersion == L"") return FALSE;
		//wprintf(L"Looking for updates at %s\n", URL);
		if (opts.debug) (*log_cstring_lines).push_back((CString)"Info: downloading updates ...");
		DeleteUrlCacheEntry(URL);
		HRESULT hr = 0;
		hr = URLDownloadToFile(
			NULL,   // A pointer to the controlling IUnknown interface (not needed here)
			URL,
			ExeName, 0,		      // Reserved. Must be set to 0.
			&pCallback); // status callback interface (not needed for basic use)
		if (SUCCEEDED(hr))
		{
			// Check if the version string matches the file name on the server
			SG_Version ver_update;
			if (SG_GetVersion(ExeName.GetBuffer(), &ver_update, log_cstring_lines))
			{
				if (SG_VersionsCompare(ver_server, ver_update) == FALSE)
				{
					//wprintf(L"Version string doesn't match actual version\n\n");
					(*log_cstring_lines).push_back((CString)"Error: expected server version " + version_CString(ver_server) + (CString)" doesn't match downloaded version " + version_CString(ver_update));
					return FALSE;
				}
			}
			//wprintf(L"Downloaded file '%s' which is a newer version. Result = %u\n", m_NextVersion, hr);
			if (opts.debug) (*log_cstring_lines).push_back((CString)"Info: downloaded file '" + m_NextVersion + (CString)"' which is a newer version.");
			if (ReplaceExeVersion(m_SelfFullPath, ExeName, log_cstring_lines)) {
				(*log_cstring_lines).push_back((CString)"Info: updated new version of file " + m_SelfFileName);
				if (SG_Run(m_SelfFullPath.GetBuffer(), log_cstring_lines))
				{
					//wprintf(L"Successfully started the temp version (%s)\n", ExeName);
					(*log_cstring_lines).push_back((CString)"Info: successfully started the temp version (" + ExeName + _T(")"));
					_exit(0);
				}
				else
				{
					//wprintf(L"Couldn't start the temp version (%s)\n", ExeName);
					(*log_cstring_lines).push_back((CString)"Error: couldn't start the new version (" + ExeName + _T(")"));
				}

			}
			else
			{
				(*log_cstring_lines).push_back((CString)"Error: cannot update new version of file " + m_SelfFileName);
				return FALSE;
			}
		}
		else
			//wprintf(L"No new version (%s) on the server\n", m_NextVersion);
			(*log_cstring_lines).push_back((CString)"Error: no new version (" + ExeName + _T(") on the server"));
		if (hr == 1) return true;
		else return false;
		//return (hr) ? TRUE : FALSE;
	}
	else { // Ask for individual launch for update
		(*log_cstring_lines).push_back((CString)"Info: manually launch " + m_SelfFileName + " separately without any other instance running to update it to new version " + version_CString(ver_server));
		return FALSE;
	}
}


long	AutoUpdate::SG_Version_number(const SG_Version version) {
	return version.SubRevision + 100 * (version.Revision + 100 * (version.Minor + 100 * (version.Major)));
}

CString AutoUpdate::version_CString(const SG_Version version) {
	CString strVer;
	strVer.Format(L"%d.%d.%d.%d", version.Major, version.Minor, version.Revision, version.SubRevision);
	return strVer;
}

//#pragma warning(default:4477 4313 4840 4189)
