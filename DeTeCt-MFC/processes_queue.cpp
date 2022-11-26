#include "processes_queue.hpp"
#include "common.h"
#include "common2.h"
//#include "auxfunc.h"

#include <tlhelp32.h>
#include <psapi.h>

//#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>


#ifndef _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#define _SILENCE_EXPERIMENTAL_FILESYSTEM_DEPRECATION_WARNING
#endif
#include <experimental\filesystem>
namespace filesys = std::experimental::filesystem;

/*** internal functions ***/

BOOL	OpenRQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);
BOOL	OpenRWQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);
BOOL	OpenWAppendQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);
BOOL	OpenWEraseQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);
void	ReleaseHandle(HANDLE* pFileHandle);

BOOL	OpenQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle, const DWORD dwDesiredAccess, const DWORD dwShareMode, const DWORD dwCreationDisposition);		//internal
BOOL	IsItemAlreadyQueued(const CString objectname, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);			//internal	R--
BOOL	PopFileFromQueue(CString* objectname, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);										//internal	--D

int		DetectInstancesNumber();
int		ProcessChildren(BOOL kills);
int		ParentProcessChildren(const DWORD parent_PID, const BOOL kills);

/**********************************************************************************************
***********************************************************************************************

	INSTANCES FUNCTIONS

***********************************************************************************************
**********************************************************************************************/


/**********************************************************************************************
*
* @fn	ProcessRunningInstancesNumber(const char *ProcessFilename)
*
* @brief	Get number of specific processes running
*
* @author	Marc
* @date		2020-04-15
*
* @param	ProcessFilename [in]   	Process filename to be checked
*
* @return	number of ProcessFilename processes running
**************************************************************************************************/


/**********************************************************************************************
***********************************************************************************************

	DETECT exe file/folder name FUNCTIONS

***********************************************************************************************
**********************************************************************************************/


/**********************************************************************************************
*
* @fn	char *DeTeCtFileName(char *DeTeCtFileNameChar)
*
* @brief	Gets current executable name (ie "DeTeCt.exe")
*
* @author	Marc
* @date		2020-04-15
*
* @param	DeTeCtFileNameChar [out]	current executable name
*
* @return	current executable name
**************************************************************************************************/

char *DeTeCtFileName(char *DeTeCtFileNameChar)
{
	LPWSTR DeTeCtFullPathName = new TCHAR[MAX_PATH];
	//	char DeTeCtFileNameChar[MAX_PATH];

		/*** Get current executable name ***/
	GetModuleFileName(NULL, DeTeCtFullPathName, MAX_PATH);
	LPWSTR DeTeCtFileName = PathFindFileName(DeTeCtFullPathName);
	wcstombs(DeTeCtFileNameChar, DeTeCtFileName, MAX_PATH);

	return DeTeCtFileNameChar;
}

/**********************************************************************************************
*
* @fn	CString  DeTeCt_additional_filename_from_folder(const CString foldername, const CString suffix)
*
* @brief	returns detect additional filename from executable filename (ie L"folder\\Detect_suffix")
*
* @author	Marc
* @date		2020-04-15
*
* @param	foldername [in]	foldername to use as prefix to DeTeCt root name
* @param	suffix [in]		suffix to use as suffix to DeTeCt root name
*
* @return	DeTeCt_additional_filename_from_folder
**************************************************************************************************/

CString  DeTeCt_additional_filename_from_folder(const CString foldername, const CString suffix)
{
	CString folder_return = foldername;
	char DeTeCtFileNameChar[MAX_PATH];
	DeTeCtFileName(DeTeCtFileNameChar);
	CString DeTeCtFileNameString(DeTeCtFileNameChar);

	if (foldername != "") folder_return.Append(L"\\");
	folder_return.Append(DeTeCtFileNameString.Left(DeTeCtFileNameString.ReverseFind(_T('.'))));
	folder_return.Append(suffix);

	return folder_return;
}

/**********************************************************************************************
*
* @fn	DeTeCt_additional_filename_exe_fullpath(const CString suffix)
*
* @brief	returns detect filename in detect executable folder
*			ie L"G:\\Work\\Impact\\DeTeCt-PSWS\\DeTeCt-MFC\\x64\\Release\\DeTeCt_suffix
*
* @author	Marc
* @date		2020-04-15
*
* @param	suffix [in]		suffix to use as suffix to DeTeCt root name
*
* @return	DeTeCt_additional_filename_from_folder
**************************************************************************************************/

CString  DeTeCt_additional_filename_exe_fullpath(const CString suffix)
{
	return DeTeCt_additional_filename_from_folder(DeTeCt_exe_folder(), suffix);
}

/**********************************************************************************************
*
* @fn	DeTeCt_exe_folder()
*
* @brief	returns detect executable foldername
*			ie L"G:\\Work\\Impact\\DeTeCt-PSWS\\DeTeCt-MFC\\x64\\Release"
*
* @author	Marc
* @date		2020-04-15
*
* @return	DeTeCt executable folder name
**************************************************************************************************/

CString DeTeCt_exe_folder()
{
	wchar_t exepath[1000];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CString folder = exepath;
	folder = folder.Left(folder.ReverseFind(_T('\\')));

	return folder;
}



/**********************************************************************************************
***********************************************************************************************

	Queue of acquisition files to be processed FUNCTIONS

***********************************************************************************************
**********************************************************************************************/

/**********************************************************************************************
*
* @fn	PushItemToQueue(const CString line, CString tag, CString QueueFilename)
*
* @brief	pushes tag: Item in QueueFilename (last position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	line [in]						object filename
* @param	tag [in]						tag
* @param	QueueFilename [in,out]	queue filename
**************************************************************************************************/

BOOL PushItemToQueue(const CString line, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //ok, WA
{
	DWORD	dwBytesWritten = 0;
	CT2A objectnamechar(tag + _T(": ") + line + _T("\n"));

	if (pQueueFileHandle == NULL) {
		HANDLE TempQueueHandle = INVALID_HANDLE_VALUE;
		if (OpenWAppendQueueFile(QueueFilename, &TempQueueHandle)) {
			WriteFile(TempQueueHandle, objectnamechar, tag.GetLength() + 2 + line.GetLength() + 1, &dwBytesWritten, NULL);
			ReleaseHandle(&TempQueueHandle);
			return TRUE;
		} else return FALSE;
	}
	else {
		if (OpenWAppendQueueFile(QueueFilename, pQueueFileHandle)) {
			WriteFile(*pQueueFileHandle, objectnamechar, tag.GetLength() + 2 + line.GetLength() + 1, &dwBytesWritten, NULL);
			if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
			return TRUE;
		} else return FALSE;
	}
}


BOOL GetItemFromQueue(CString* pObject, const CString search_string, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //ok, R
{
	CString line = L"";
	(*pObject) = "";

	if (pQueueFileHandle == NULL) {
		HANDLE TempQueueHandle = INVALID_HANDLE_VALUE;
		if (OpenRQueueFile(QueueFilename, &TempQueueHandle)) {
			do {
				line = GetLine(TempQueueHandle);
				if (line.Find(search_string, 0) == 0) {
					line.Delete(line.Find(search_string), search_string.GetLength());
					(*pObject) = line;
					ReleaseHandle(&TempQueueHandle);
					return TRUE;
				}
			} while (((*pObject) == "") && (line.GetLength() > 1));
			ReleaseHandle(&TempQueueHandle);
		}
	}
	else {
		if (OpenRQueueFile(QueueFilename, pQueueFileHandle)) {
			do {
				line = GetLine(*pQueueFileHandle);
				if (line.Find(search_string, 0) == 0) {
					line.Delete(line.Find(search_string), search_string.GetLength());
					(*pObject) = line;
					if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
					return TRUE;
				}
			} while (((*pObject) == "") && (line.GetLength() > 1));
			if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
		}
	}
	return FALSE;
}


/**********************************************************************************************
*
* @fn	RemoveItemsFromQueue(const CString line, CString tag, CString QueueFilename)
*
* @brief	removes tag: Item in QueueFilename
*
* @author	Marc
* @date		2020-05-05
*
* @param	objectname [in]					object filename
* @param	tag [in]						tag
* @param	QueueFilename [in,out]	queue filename
**************************************************************************************************/
void RemoveItemsFromQueue(const CString objectname, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) { //KO / R+Del+WA, could be R+Del+W - check how to keep new created HANDLE, and what to do with close@end

	BOOL	local_close_handle_at_end = close_handle_at_end;

	if (pQueueFileHandle == NULL) local_close_handle_at_end = TRUE;
	if (!OpenRWQueueFile(QueueFilename, pQueueFileHandle)) return;

	std::vector<CString> cstring_lines;
	BOOL file_to_be_updated = FALSE;
	CString object_string = tag + _T(": ") + objectname;
	CString line = L"";

	SetFilePointer(*pQueueFileHandle, 0, NULL, FILE_BEGIN);
	do {
		line = GetLine(*pQueueFileHandle);
		if (line.GetLength() > 1) {
			if (line.Find(object_string, 0) != 0) cstring_lines.push_back(line);
			else file_to_be_updated = TRUE;
		}
	} while (line.GetLength() > 1);

	if (file_to_be_updated) {
		DWORD	dwBytesWritten = 0;

		SetFilePointerEx(*pQueueFileHandle, { 0 }, NULL, FILE_BEGIN);
		SetEndOfFile(*pQueueFileHandle);
		std::for_each(cstring_lines.begin(), cstring_lines.end(), [&](const CString cstring_line) {
			CT2A line(cstring_line + _T("\n"));
			WriteFile(*pQueueFileHandle, line, cstring_line.GetLength() + 1, &dwBytesWritten, NULL);
			});
		if (local_close_handle_at_end) ReleaseHandle(pQueueFileHandle);
	}
	else if (local_close_handle_at_end) ReleaseHandle(pQueueFileHandle); // Item to be removed not found

}

void SetIntParamToQueue(const int param, const CString tag, const CString QueueFilename) { //KO because of removeitems
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;

	//RemoveItemsFromQueue((CString)"", tag, QueueFilename, &QueueFileHandle, FALSE);
	RemoveItemsFromQueue((CString)"", tag, QueueFilename, &QueueFileHandle, FALSE);
	PushItemToQueue((CString)std::to_string(param).c_str(), tag, QueueFilename, &QueueFileHandle, TRUE);
}

int GetIntParamFromQueue(const CString tag, const CString QueueFilename) { //ok, R
	int value = 0;
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;
	
	CString object;
	GetItemFromQueue(&object, tag + _T(": "), QueueFilename, &QueueFileHandle, TRUE);
	value = StrToInt(object);
	return value;
}

// For Debug

int NbItemFromQueue(const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //ok, R
{
	int		nbitem		= 0;
	CString tag_string	= tag + _T(": ");
	CString line		= L"";

	if (pQueueFileHandle == NULL) {
		HANDLE TempQueueHandle = INVALID_HANDLE_VALUE;
		if (OpenRQueueFile(QueueFilename, &TempQueueHandle)) {
			do {
				line = GetLine(TempQueueHandle);
				if (line.Find(tag_string, 0) == 0) nbitem++;
			} while (line.GetLength() > 1);
			ReleaseHandle(&TempQueueHandle);
		}
	}
	else {
		if (OpenRQueueFile(QueueFilename, pQueueFileHandle)) {
			do {
				line = GetLine(*pQueueFileHandle);
				if (line.Find(tag_string, 0) == 0) nbitem++;
			} while (line.GetLength() > 1);
			if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
		}
	}
	return nbitem;
}

/**********************************************************************************************
*
* @fn	PushFileToQueue(const CString objectname,  CString QueueFilename)
*
* @brief	pushes objectname in QueueFilename (last position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]					object
* @param	QueueFilename [in,out]	queue filename
**************************************************************************************************/

void PushFileToQueue(const CString objectname, const CString QueueFilename) //ok  because of pushitems //KO to link!
{
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;
	
	PushItemToQueue(objectname, L"file", QueueFilename, &QueueFileHandle, TRUE);
}

/**********************************************************************************************
*
* @fn	RemoveFileFromQueue(const CString objectname, CString QueueFilename)
*
* @brief	removes from objectname from QueueFilename
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]						object
* @param	QueueFilename [in,out]		queue filename
**************************************************************************************************/

void RemoveFileFromQueue(const CString objectname, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //KO because of removeitems ; - check how to keep new created HANDLE, and what to do with close@end
{
	if (pQueueFileHandle == NULL) {
		HANDLE TempQueueHandle = INVALID_HANDLE_VALUE;
		RemoveItemsFromQueue(objectname, L"file", QueueFilename, &TempQueueHandle, TRUE);
	}
	else RemoveItemsFromQueue(objectname, L"file", QueueFilename, pQueueFileHandle, close_handle_at_end);
}

/**********************************************************************************************
*
* @fn	IsFileAlreadyQueued(const CString objectname, const CString QueueFilename)
*
* @brief	returns if objectname is already queued in QueueFilename
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]					object
* @param	QueueFilename [in]		queue filename
*
* @return	returns if objectname is already queued in QueueFilename
**************************************************************************************************/

BOOL IsFileAlreadyQueued(const CString objectname, const CString QueueFilename) //ok because of GetItem
{
	CString	processed_line;
	HANDLE	QueueFileHandle = INVALID_HANDLE_VALUE;

	return GetItemFromQueue(&processed_line, L"file: " + objectname, QueueFilename, &QueueFileHandle, TRUE);
}


int  NbFilesFromQueue(const CString QueueFilename) //ok because of NbItem
{
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;

	int		nbitem = 0;
	CString tag_string = L"file";
	CString line = L"";

	if (OpenRQueueFile(QueueFilename, &QueueFileHandle)) {
		do {
			line = GetLine(QueueFileHandle);
			if (line.Find(tag_string, 0) == 0) nbitem++;
		} while (line.GetLength() > 1);
		ReleaseHandle(&QueueFileHandle);
	}
		return nbitem;
}

BOOL GetFileFromQueue(CString* pObjectname, const CString QueueFilename) //ko
{
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;

	if (PopFileFromQueue(pObjectname, QueueFilename, &QueueFileHandle, FALSE)) {
		if (!IsItemAlreadyQueued(*pObjectname, _T("file_processing"), QueueFilename, &QueueFileHandle, FALSE)) {
			PushItemToQueue(*pObjectname, _T("file_processing"), QueueFilename, &QueueFileHandle, FALSE);
			ReleaseHandle(&QueueFileHandle);
			return TRUE;
		}
	}
	ReleaseHandle(&QueueFileHandle);
	return FALSE;
}

void SetFileProcessingFromQueue(const CString objectname, const CString QueueFilename) { //KO
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;

	if (!IsItemAlreadyQueued(objectname, _T("file_processing"), QueueFilename, &QueueFileHandle, FALSE)) {
		RemoveItemsFromQueue(objectname, _T("file"), QueueFilename, &QueueFileHandle, FALSE);
		PushItemToQueue(objectname, _T("file_processing"), QueueFilename, &QueueFileHandle, FALSE);
	}
	ReleaseHandle(&QueueFileHandle);
}

void SetProcessingFileProcessedFromQueue(const CString objectname_cstring, const CString details, const CString tag, const CString QueueFilename) { //KO
	HANDLE QueueFileHandle = INVALID_HANDLE_VALUE;

	PushItemToQueue(objectname_cstring + details, tag, QueueFilename, &QueueFileHandle, FALSE);
	RemoveItemsFromQueue(objectname_cstring, _T("file_processing"), QueueFilename, &QueueFileHandle, FALSE);
	ReleaseHandle(&QueueFileHandle);
}


BOOL GetProcessedFileFromQueue(CString *processed_filename, CString *processed_filename_acquisition, CString *processed_message, Rating_type *processed_rating, double *duration, int *nframe_child, int *fps_int_child, const CString QueueFilename) //KO
{
	if (!filesys::exists(CString2string(QueueFilename))) 
	{
		 char msgtext[MAX_STRING] = { 0 };
		char tmpline[MAX_STRING];
		snprintf(msgtext, MAX_STRING, "cannot find acquisition queue file %s", CString2char(QueueFilename, tmpline));
		ErrorExit(TRUE, "queue file not found", "GetProcessedFileFromQueue()", msgtext);  	// exits DeTeCt if Queuefile does not exists
	}
	CString	processed_line;
	BOOL	status;
	HANDLE	QueueFileHandle		= INVALID_HANDLE_VALUE;

	if (!GetItemFromQueue(&processed_line, _T("file_processed : "), QueueFilename, &QueueFileHandle, FALSE)) {
		ReleaseHandle(&QueueFileHandle);
		return FALSE;
	}

	(*duration)			= 0;
	(*nframe_child)		= 0;
	(*fps_int_child)	= 0;
	std::string tmp_line(CString2string(processed_line));
	status = TRUE;

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*processed_filename) = tmp_line.substr(0, tmp_line.find("|")).c_str();
	else status=FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*processed_filename_acquisition) = tmp_line.substr(0, tmp_line.find("|")).c_str();
	else status = FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*processed_message) = tmp_line.substr(0, tmp_line.find("|")).c_str();
	else status = FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*processed_rating) = (Rating_type)(atoi(tmp_line.substr(0, tmp_line.find("|")).c_str()));
	else status = FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*duration) = atoi(tmp_line.c_str());
	else status = FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*nframe_child) = atoi(tmp_line.c_str());
	else status = FALSE;
	tmp_line.erase(0, tmp_line.find("|") + 1);

	while (tmp_line.substr(tmp_line.size() - 1, 1) == " ") tmp_line.erase(tmp_line.size() - 1, 1);
	if (tmp_line.find("|")) (*fps_int_child) = atoi(tmp_line.c_str());
	else status = FALSE;

	RemoveItemsFromQueue(processed_line,				_T("file_processed "), (CString)QueueFilename, &QueueFileHandle, FALSE);
	if (status)	PushItemToQueue(processed_line,			_T("file_ok        "), (CString)QueueFilename, &QueueFileHandle, FALSE);
	else		PushItemToQueue(processed_line,			_T("file_ko        "), (CString)QueueFilename, &QueueFileHandle, FALSE);

	ReleaseHandle(&QueueFileHandle);
	return status;
}


/**********************************************************************************************
***********************************************************************************************

	RUNNING PROCESSES FUNCTIONS

From https://stackoverflow.com/questions/29939893/get-parent-process-name-windows

***********************************************************************************************
**********************************************************************************************/

/**********************************************************************************************
*
* @fn	getParentPID(const DWORD pid)
*
* @brief	gets parent process ID from child process id
* From https://stackoverflow.com/questions/29939893/get-parent-process-name-windows
*
* @author	Marc
* @date		2020-04-15
*
* @param	pid [in]	child process ID
*
* @return	Parent Process ID
**************************************************************************************************/

DWORD getParentPID(const DWORD pid)
{
	HANDLE h = NULL;
	PROCESSENTRY32 pe = { 0 };
	DWORD ppid = 0;
	pe.dwSize = sizeof(PROCESSENTRY32);
	h = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (Process32First(h, &pe))
	{
		do
		{
			if (pe.th32ProcessID == pid)
			{
				ppid = pe.th32ParentProcessID;
				break;
			}
		} while (Process32Next(h, &pe));
	}
	CloseHandle(h);
	return (ppid);
}

/**********************************************************************************************
*
* @fn	getProcessName(const DWORD pid, wchar_t *wfname, DWORD sz)
*
* @brief	gets parent process name in wfname from its process ID
* From https://stackoverflow.com/questions/29939893/get-parent-process-name-windows
*
* @author	Marc
* @date		2020-04-15
*
* @param	pid [in]		child process ID
* @param	wfname [out]	process name
*
* @return	Parent Process ID
**************************************************************************************************/

int getProcessName(const DWORD pid, wchar_t *wfname, DWORD sz)
{
	HANDLE h = NULL;
	int e = 0;
	h = OpenProcess
	(
		PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
//		PROCESS_QUERY_INFORMATION,
		FALSE,
		pid
	);
	if (h)
	{
		if (GetModuleFileNameEx(h, NULL, wfname, sz) == 0)
			e = GetLastError();
		CloseHandle(h);
	}
	else
	{
		e = GetLastError();
	}
	return (e);
}

int ProcessRunningInstancesNumber(const char *ProcessFilename)
{
	int ProcessesQuantity = 0;
	char RunningProcessName[MAX_PATH];

	/*** Check current running processes ***/
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0);
	if (hndl)
	{
		PROCESSENTRY32  process = { sizeof(PROCESSENTRY32) };
		Process32First(hndl, &process);
		do
		{
			sprintf(RunningProcessName, "%ws", process.szExeFile);
			if (strcmp(RunningProcessName, ProcessFilename) == 0) ProcessesQuantity++;
		} while (Process32Next(hndl, &process));
		CloseHandle(hndl);
	}
	return ProcessesQuantity;
}

BOOL IsProcessRunning(const DWORD pid)
{
	HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, pid);
	DWORD ret = WaitForSingleObject(process, 0);
	CloseHandle(process);

	return ret == WAIT_TIMEOUT;
}

int KillsChildrenProcesses()
{
	return ProcessChildren(TRUE);
}

int ChildrenProcessesNumber()
{
	return ProcessChildren(FALSE);
}

int ParentChildrenProcessesNumber(const DWORD parent_PID)
{
	return ParentProcessChildren(parent_PID, FALSE);
}

/**********************************************************************************************
***********************************************************************************************

	internal functions

***********************************************************************************************
**********************************************************************************************/

// ************** DeTeCt process queue management **********

BOOL OpenRQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle) //ok
{
	if (((*pQueueFileHandle == INVALID_HANDLE_VALUE) || (*pQueueFileHandle == NULL)) && (!filesys::exists(CString2string(QueueFilename)))) return FALSE; // only because file must be read: Queuefilename must exist already

	if (OpenQueueFile(QueueFilename, pQueueFileHandle, (GENERIC_READ | GENERIC_WRITE), 0, OPEN_ALWAYS)) {
		SetFilePointer(*pQueueFileHandle, 0, NULL, FILE_BEGIN);
		return TRUE;
	} else return FALSE;
}

BOOL OpenRWQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle) //ok
{
	if (OpenQueueFile(QueueFilename, pQueueFileHandle, (GENERIC_READ | GENERIC_WRITE), 0, OPEN_ALWAYS)) {
		SetFilePointer(*pQueueFileHandle, 0, NULL, FILE_BEGIN);
		return TRUE;
	}
	else return FALSE;
}

BOOL OpenWAppendQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle) //ok
{
	if (OpenQueueFile(QueueFilename, pQueueFileHandle, (GENERIC_READ | GENERIC_WRITE), 0, OPEN_ALWAYS)) {
		SetFilePointer(*pQueueFileHandle, 0, NULL, FILE_END);
		return TRUE;
	}
	else return FALSE;
}

BOOL OpenWEraseQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle) //ok
{
	if (OpenQueueFile(QueueFilename, pQueueFileHandle, (GENERIC_READ | GENERIC_WRITE), 0, OPEN_ALWAYS)) {
		SetFilePointer(*pQueueFileHandle, 0, NULL, FILE_BEGIN);
		SetEndOfFile(*pQueueFileHandle);
		return TRUE;
	}
	else return FALSE;
}

BOOL OpenQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle, const DWORD dwDesiredAccess, const DWORD dwShareMode, const DWORD dwCreationDisposition) //ok
{
	if ((*pQueueFileHandle != INVALID_HANDLE_VALUE) && (*pQueueFileHandle != NULL)) return TRUE;
	
	if (QueueFilename.GetLength() > 1) {
		do {
			(*pQueueFileHandle) = CreateFileW(QueueFilename, dwDesiredAccess,		dwShareMode,		NULL,					dwCreationDisposition,	FILE_ATTRIBUTE_NORMAL, NULL);
			//dwLastErrorCode = GetLastError();
			if ((*pQueueFileHandle == INVALID_HANDLE_VALUE) || (*pQueueFileHandle == NULL)) Sleep(FILEACCESS_WAIT_MS);
			else {
				return TRUE;
			}
		} while (TRUE);
	}
	else return FALSE;
}

void ReleaseHandle(HANDLE* pFileHandle) {
	if (*pFileHandle != INVALID_HANDLE_VALUE) {
		CloseHandle(*pFileHandle);
		(*pFileHandle) = INVALID_HANDLE_VALUE;
	}
}

BOOL IsItemAlreadyQueued(const CString objectname, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //ok
{
	CString	processed_line;
	
	return GetItemFromQueue(&processed_line, tag + _T(": ") + objectname, QueueFilename, pQueueFileHandle, close_handle_at_end);
}

/**********************************************************************************************
*
* @fn	PopFileFromQueue(CString *objectname,  CString QueueFilename)
*
* @brief	pops and removes objectname from QueueFilename (first position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [out]				object
* @param	QueueFilename [in]		queue filename
*
* @return	FALSE if Queue does not exist, TRUE otherwise
**************************************************************************************************/

BOOL PopFileFromQueue(CString* pObjectname, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end) //KO
{
	if (GetItemFromQueue(pObjectname, L"file: ", QueueFilename, pQueueFileHandle, FALSE)) {
		RemoveFileFromQueue((*pObjectname), QueueFilename, pQueueFileHandle, FALSE);
		if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
		return TRUE;
	}
	else {
		if (close_handle_at_end) ReleaseHandle(pQueueFileHandle);
		return FALSE;
	}
}

// ************** Process functions **********

/**********************************************************************************************
*
* @fn	DetectInstancesNumber()
*
* @brief	Get number of DeTeCt processes running
*
* @author	Marc
* @date		2020-04-15
*
* @return	number of DeTeCt processes running
**************************************************************************************************/

int DetectInstancesNumber()
{
	char DeTeCtFileNameChar[MAX_PATH];

	return ProcessRunningInstancesNumber(DeTeCtFileName(DeTeCtFileNameChar));
}

int ProcessChildren(const BOOL kills)
{
	return ParentProcessChildren(GetCurrentProcessId(), kills);
}

int ParentProcessChildren(const DWORD parent_PID, const BOOL kills)
{
	int processed_children = 0;
	/*** Check current running processes ***/
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0);
	if (hndl)
	{
		PROCESSENTRY32  process = { sizeof(PROCESSENTRY32) };
		Process32First(hndl, &process);
		do
		{
			if ((getParentPID(process.th32ProcessID) == parent_PID) && (process.th32ProcessID != parent_PID)) {
				/*CString pid_str;
				pid_str.Format(L"%g", process.th32ProcessID);
				int retval = ::_tsystem(_T("taskkill /F /T /PID ") + pid_str);*/
				if (kills) TerminateProcess(hndl, EXIT_FAILURE);
				processed_children++;
			}
		} while (Process32Next(hndl, &process));
		CloseHandle(hndl);
	}
	return processed_children;
}