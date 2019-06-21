
#include "processes_queue.h"

#include "auxfunc.h"
//#include <windows.h>
//#include <stdio.h>
//#include <psapi.h>
#include <tlhelp32.h>

//#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

int DectectInstancesNumber()
{
	int ProcessesQuantity = 0;
	char RunningProcessName[MAX_PATH];
	char DeTeCtFileNameChar[MAX_PATH];

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
			if (strcmp(RunningProcessName, DeTeCtFileName(DeTeCtFileNameChar)) == 0) ProcessesQuantity++;
			//DBOUT("Process " << process.szExeFile << "\n");
		} while (Process32Next(hndl, &process));
		CloseHandle(hndl);
	}
	return ProcessesQuantity;
}

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


/*** returns detect additional filename from executable filename ***/
CString  DeTeCt_additional_filename(CString folder, CString suffix)
{
	CString folder_return = folder;
	char DeTeCtFileNameChar[MAX_PATH];
	DeTeCtFileName(DeTeCtFileNameChar);
	CString DeTeCtFileNameString(DeTeCtFileNameChar);

	folder_return.Append(L"\\");
	folder_return.Append(DeTeCtFileNameString.Left(DeTeCtFileNameString.ReverseFind(_T('.'))));
	folder_return.Append(suffix);

	return folder_return;
}

/*** returns detect filename in detect executable folder ***/
CString  DeTeCt_additional_filename_exe_folder(CString suffix)
{
	return DeTeCt_additional_filename(DeTeCt_exe_folder(), suffix);
}

/*** returns detect executable foldername ***/
CString DeTeCt_exe_folder()
{
	wchar_t exepath[1000];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CString folder = exepath;
	folder = folder.Left(folder.ReverseFind(_T('\\')));

	return folder;
}

BOOL IsAlreadyQueued(CString objectname, CString DeTeCtQueueFilename)
{
	std::ifstream DeTeCtQueueFile(DeTeCtQueueFilename);

	if (DeTeCtQueueFile) {
		std::string line;
		CT2A tmp(objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1); 
			if (line.compare(objectstring) == 0) {
				DeTeCtQueueFile.close();
				return TRUE;
			}
		}
	}
	return FALSE;
}

void PushToQueue(CString objectname, CString DeTeCtQueueFilename)
{
	std::ofstream output_file(DeTeCtQueueFilename, std::ofstream::app);
	CT2A objectnamechar(objectname);
	output_file << objectnamechar << "\n";
	output_file.close();
}

void RemoveFromQueue(CString objectname, CString DeTeCtQueueFilename) {
	
	std::ifstream DeTeCtQueueInputFile(DeTeCtQueueFilename);

	if (DeTeCtQueueInputFile) {
		std::vector<std::string> lines;
		std::string line;
		int linesnb = 0;

		CT2A tmp(objectname);
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
	}
}

BOOL PopFromQueue(CString *objectname, CString DeTeCtQueueFilename)
{
	std::ifstream DeTeCtQueueFile(DeTeCtQueueFilename);

	if (DeTeCtQueueFile) {
		std::string line;
		//		CT2A tmp(objectname);
		//		std::string objectstring(tmp);

		std::getline(DeTeCtQueueFile, line);
		DeTeCtQueueFile.close();
		//copy in objectname first line
		(*objectname) = line.c_str();
		RemoveFromQueue((*objectname), DeTeCtQueueFilename);
		return TRUE;
	} else return FALSE;
}
