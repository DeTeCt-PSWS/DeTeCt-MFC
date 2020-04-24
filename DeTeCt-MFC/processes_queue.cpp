
#include "processes_queue.h"

#include "auxfunc.h"

//#include <windows.h>
//#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>

//#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>


/**********************************************************************************************
***********************************************************************************************

	INSTANCES FUNCTIONS

***********************************************************************************************
**********************************************************************************************/

/**********************************************************************************************
*
* @fn	AutostakkertInstancesNumber()
*
* @brief	Get number of Autostakkert processes running
*
* @author	Marc
* @date		2020-04-15
*
* @return	number of Autostakkert processes running
**************************************************************************************************/

int AutostakkertInstancesNumber()
{
	return ProcessRunningInstancesNumber(AUTOSTAKKERTFILENAME);
}

/**********************************************************************************************
*
* @fn	DectectInstancesNumber()
*
* @brief	Get number of DeTeCt processes running
*
* @author	Marc
* @date		2020-04-15
*
* @return	number of DeTeCt processes running
**************************************************************************************************/

int DectectInstancesNumber()
{
	char DeTeCtFileNameChar[MAX_PATH];

	return ProcessRunningInstancesNumber(DeTeCtFileName(DeTeCtFileNameChar));
}

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
			//TCHAR Buffer[MAX_PATH];
			//if (GetModuleFileNameEx(hndl, 0, Buffer, MAX_PATH))
			//wprintf(L"%8u, %s\n", process.th32ProcessID, process.szExeFile);
			sprintf(RunningProcessName, "%ws", process.szExeFile);
			if (strcmp(RunningProcessName, ProcessFilename) == 0) ProcessesQuantity++;
			//DBOUT("Process " << process.szExeFile << "\n");
		} while (Process32Next(hndl, &process));
		CloseHandle(hndl);
	}
	return ProcessesQuantity;
}

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
* @fn	CString  DeTeCt_additional_filename(const CString foldername, const CString suffix)
*
* @brief	returns detect additional filename from executable filename (ie L"folder\\Detect_suffix")
*
* @author	Marc
* @date		2020-04-15
*
* @param	foldername [in]	foldername to use as prefix to DeTeCt root name
* @param	suffix [in]		suffix to use as suffix to DeTeCt root name
*
* @return	DeTeCt_additional_filename
**************************************************************************************************/

CString  DeTeCt_additional_filename(const CString foldername, const CString suffix)
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
* @fn	DeTeCt_additional_filename_exe_folder(const CString suffix)
*
* @brief	returns detect filename in detect executable folder
*			ie L"G:\\Work\\Impact\\DeTeCt-PSWS\\DeTeCt-MFC\\x64\\Release\\DeTeCt_suffix
*
* @author	Marc
* @date		2020-04-15
*
* @param	suffix [in]		suffix to use as suffix to DeTeCt root name
*
* @return	DeTeCt_additional_filename
**************************************************************************************************/

CString  DeTeCt_additional_filename_exe_folder(const CString suffix)
{
	return DeTeCt_additional_filename(DeTeCt_exe_folder(), suffix);
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
* @fn	WriteItemToQueue(const CString line, CString tag, CString DeTeCtQueueFilename)
*
* @brief	pushes tag: Item in DeTeCtQueueFilename (last position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	line [in]						object filename
* @param	tag [in]						tag
* @param	DeTeCtQueueFilename [in,out]	queue filename
**************************************************************************************************/

void WriteItemToQueue(const CString line, const CString tag, CString DeTeCtQueueFilename)
{
	std::ofstream output_file(DeTeCtQueueFilename, std::ofstream::app);
	CT2A objectnamechar(tag + ": " + line);
	output_file << objectnamechar << "\n";
	output_file.close();
}

/**********************************************************************************************
*
* @fn	WriteItemToQueue(const CString line, CString tag, CString DeTeCtQueueFilename)
*
* @brief	pushes tag: Item in DeTeCtQueueFilename (last position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	line [in]						object
* @param	DeTeCtQueueFilename [in,out]	queue filename
*
* @return: TRUE if tag was found, with item in object, FALSE/NULL otherwise
**************************************************************************************************/


BOOL	GetItemFromQueue(CString *object, const CString tag, const CString DeTeCtQueueFilename)
{
	std::ifstream DeTeCtQueueFile(DeTeCtQueueFilename);
	CT2A tmp(tag + ": ");
	std::string tag_string(tmp);


	if (DeTeCtQueueFile) {
		std::string line;
		(*object) = "";
		while (((*object) == "") && (std::getline(DeTeCtQueueFile, line))) {
			if (line.find(tag_string) != std::string::npos) {
				line.erase(line.find(tag_string), tag_string.size());
				(*object) = line.c_str();
				DeTeCtQueueFile.close();
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	return FALSE;
}

/**********************************************************************************************
*
* @fn	IsAlreadyQueued(const CString objectname, const CString DeTeCtQueueFilename)
*
* @brief	returns if objectname is already queued in DeTeCtQueueFilename
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]					object
* @param	DeTeCtQueueFilename [in]		queue filename
*
* @return	returns if objectname is already queued in DeTeCtQueueFilename
**************************************************************************************************/

BOOL IsAlreadyQueued(const CString objectname, const CString DeTeCtQueueFilename)
{
	std::ifstream DeTeCtQueueFile(DeTeCtQueueFilename);

	if (DeTeCtQueueFile) {
		std::string line;
		CT2A tmp(L"file: " + objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1); 
			if (line.compare(objectstring) == 0) {
				DeTeCtQueueFile.close();
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();

	return FALSE;
}

/**********************************************************************************************
*
* @fn	PushToQueue(const CString objectname,  CString DeTeCtQueueFilename)
*
* @brief	pushes objectname in DeTeCtQueueFilename (last position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]					object
* @param	DeTeCtQueueFilename [in,out]	queue filename
**************************************************************************************************/

void PushToQueue(const CString objectname,  CString DeTeCtQueueFilename)
{
	std::ofstream DeTeCtQueueFile(DeTeCtQueueFilename, std::ofstream::app);
	CT2A objectnamechar(L"file: " + objectname);
	DeTeCtQueueFile << objectnamechar << "\n";
	DeTeCtQueueFile.close();
}

/**********************************************************************************************
*
* @fn	PopFromQueue(CString *objectname,  CString DeTeCtQueueFilename)
*
* @brief	pops and removes objectname from DeTeCtQueueFilename (first position)
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [out]				object
* @param	DeTeCtQueueFilename [in]		queue filename
*
* @return	FALSE if Queue does not exist, TRUE otherwise
**************************************************************************************************/

BOOL PopFromQueue(CString *objectname, CString DeTeCtQueueFilename)
{
	std::ifstream DeTeCtQueueFile(DeTeCtQueueFilename);

	if (DeTeCtQueueFile) {
		std::string line;
		std::string tag("file: ");

		(*objectname) = "";
		while (((*objectname) == "") && (std::getline(DeTeCtQueueFile, line))) {
			//copy in objectname first line, removing tag
			if (line.find(tag) != std::string::npos) {
				DeTeCtQueueFile.close();
				line.erase(line.find(tag), tag.size());
				(*objectname) = line.c_str();
				RemoveFromQueue((*objectname), DeTeCtQueueFilename);
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	return FALSE;
}

/**********************************************************************************************
*
* @fn	RemoveFromQueue(const CString objectname, CString DeTeCtQueueFilename)
*
* @brief	removes from objectname from DeTeCtQueueFilename
*
* @author	Marc
* @date		2020-04-15
*
* @param	objectname [in]						object
* @param	DeTeCtQueueFilename [in,out]		queue filename
**************************************************************************************************/

void RemoveFromQueue(const CString objectname, CString DeTeCtQueueFilename) {
	
	std::ifstream DeTeCtQueueInputFile(DeTeCtQueueFilename);

	if (DeTeCtQueueInputFile) {
		std::vector<std::string> lines;
		std::string line;
		int linesnb = 0;

		CT2A tmp(L"file: " + objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueInputFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1);
			if (objectstring.compare(line) != 0) {
				lines.push_back(line);
				linesnb++;
			}
		}
		DeTeCtQueueInputFile.close();

		CT2A DeTeCtQueueFilenameChar(DeTeCtQueueFilename);
		if ((remove(DeTeCtQueueFilenameChar) == 0) && (linesnb>0)) {
			std::ofstream DeTeCtQueueOutputFile(DeTeCtQueueFilename);
			std::ostream_iterator<std::string> output_iterator(DeTeCtQueueOutputFile, "\n");
			std::copy(lines.begin(), lines.end(), output_iterator);
			DeTeCtQueueOutputFile.close();
		}
	} else 	DeTeCtQueueInputFile.close();
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

/**********************************************************************************************
*
* @fn	IsParentAutostakkert() 
*
* @brief	returns if parent of current DeTeCt process is AutoStakkert
*
* @author	Marc
* @date		2020-04-15
*
* @return	TRUE if parent of current DeTeCt process is AutoStakkert
**************************************************************************************************/

BOOL IsParentAutostakkert()
{
	DWORD pid, ppid;
	int e;
	wchar_t wfname[MAX_PATH] = { 0 };

	pid = GetCurrentProcessId();
	ppid = getParentPID(pid);
	e = getProcessName(ppid, wfname, MAX_PATH);
	if (wcsstr(wfname, LAUTOSTAKKERTFILENAME) != NULL) return TRUE;
	return FALSE;
}
