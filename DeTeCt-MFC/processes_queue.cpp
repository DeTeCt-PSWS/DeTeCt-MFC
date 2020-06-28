#include "processes_queue.h"
#include "common2.h"
#include "auxfunc.h"

#include <tlhelp32.h>
#include <psapi.h>

//#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>

#define QUEUE_LOCK_EXT ".lock"
#define QUEUE_UNLOCK_EXT ".unlock"

void WriteLockQueue(const CString text, const CString QueueFilename);

/**********************************************************************************************
***********************************************************************************************

	INSTANCES FUNCTIONS

***********************************************************************************************
**********************************************************************************************/

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

int ProcessChildren(BOOL kills);
int ParentProcessChildren(const DWORD parent_PID, const BOOL kills);

int DetectInstancesNumber()
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

void PushItemToQueue(const CString line, const CString tag, const CString QueueFilename, const BOOL use_lock)
{
	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__ + _T(" ") + line + _T(" ") + tag, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//else WriteLockQueue((CString)__FUNCTION__ + _T("(no lock)"), QueueFilename);

	//std::ofstream output_file(QueueFilename, std::ofstream::app, sharing_flag);
	std::ofstream output_file(QueueFilename, std::ofstream::app);
	CT2A objectnamechar(tag + ": " + line);
	output_file << objectnamechar << "\n";
	output_file.close();
	if (use_lock) UnlockQueue(QueueFilename);
}


BOOL GetItemFromQueue(CString *object, const CString tag, const CString QueueFilename)
{
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + tag, QueueFilename);
	//std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in, _SH_DENYRW);
	std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in);
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
				UnlockQueue(QueueFilename);
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	UnlockQueue(QueueFilename);

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
void RemoveItemsFromQueue(const CString objectname, const CString tag, const CString QueueFilename, const BOOL use_lock) {

	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname + _T(" ") + tag, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//std::ifstream DeTeCtQueueInputFile(QueueFilename, std::ifstream::in, sharing_flag);
	std::ifstream DeTeCtQueueInputFile(QueueFilename, std::ifstream::in);

	if (DeTeCtQueueInputFile) {
		std::vector<std::string> lines;
		std::string line;
		int linesnb = 0;

		CT2A tmp(tag + ": " + objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueInputFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1);
			//if (objectstring.compare(line) != 0) {
			if (line.compare(0, objectstring.size(), objectstring) != 0) {
			lines.push_back(line);
				linesnb++;
			}
		}
		DeTeCtQueueInputFile.close();

		CT2A QueueFilenameChar(QueueFilename);
		//if ((remove(QueueFilenameChar) == 0) && (linesnb > 0)) {
			//std::ofstream DeTeCtQueueOutputFile(QueueFilename, std::ofstream::out, sharing_flag);
		if (linesnb > 0) {
			std::ofstream DeTeCtQueueOutputFile(QueueFilename, std::ofstream::out|std::ofstream::trunc);
			std::ostream_iterator<std::string> output_iterator(DeTeCtQueueOutputFile, "\n");
			std::copy(lines.begin(), lines.end(), output_iterator);
			DeTeCtQueueOutputFile.close();
		}
	}
	else 	DeTeCtQueueInputFile.close();
	if (use_lock) UnlockQueue(QueueFilename);
}

void SetIntParamToQueue(const int param, const CString tag, const CString QueueFilename) {
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + (CString)std::to_string(param).c_str() + _T(" ") + tag, QueueFilename);
	RemoveItemsFromQueue((CString)"", tag, QueueFilename, FALSE);
	//RemoveItemsFromQueue((CString)"", tag, QueueFilename, TRUE);
	PushItemToQueue((CString)std::to_string(param).c_str(), tag, QueueFilename, FALSE);
	//PushItemToQueue((CString)std::to_string(param).c_str(), tag, QueueFilename, TRUE);
	UnlockQueue(QueueFilename);
}

int GetIntParamFromQueue(const CString tag, const CString QueueFilename) {
	int value = 0;
	CString object;
	GetItemFromQueue(&object, tag, QueueFilename);
	value = StrToInt(object);
	return value;
}

BOOL IsItemAlreadyQueued(const CString objectname, const CString tag, const CString QueueFilename, const BOOL use_lock)
{
	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname + _T(" ") + tag, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//else WriteLockQueue((CString)__FUNCTION__ + _T(" ") + objectname + _T(" ") + tag, QueueFilename);

	//std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in, sharing_flag);
	std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in);

	if (DeTeCtQueueFile) {
		std::string line;
		CT2A tmp(tag + objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1);
			if (line.compare(objectstring) == 0) {
				DeTeCtQueueFile.close();
				if (use_lock) UnlockQueue(QueueFilename);
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	if (use_lock) UnlockQueue(QueueFilename);
	return FALSE;
}


void LockQueue(const CString text, const CString QueueFilename)
{
	std::string lock_filename = CString2string((CString)QueueFilename).substr(0, CString2string((CString)QueueFilename).find_last_of(".")) + QUEUE_LOCK_EXT;

	std::ofstream lock_file(lock_filename, std::ofstream::app);
	if (lock_file) {
		//std::string text_string = CString2string(text) + " PID" + std::to_string(GetCurrentProcessId()).c_str(); 	// do not know why, but need to convert to string then back to cstring to have it work
		//lock_file << text_string.c_str() << "\n";
		lock_file.flush();
	}
	lock_file.close();
}

void UnlockQueue(const CString QueueFilename)
{
	std::string lock_filename = CString2string((CString)QueueFilename).substr(0, CString2string((CString)QueueFilename).find_last_of(".")) + QUEUE_LOCK_EXT;

	//WriteLockQueue((CString)__FUNCTION__ + _T(" init (no lock)"), QueueFilename);
	std::ifstream lock_file(lock_filename, std::ifstream::in);
	//std::ifstream lock_file(lock_filename);
	//WriteLockQueue((CString)__FUNCTION__ + _T(" if stream (no lock)"), QueueFilename);
	if (lock_file) {
		//WriteLockQueue((CString)__FUNCTION__ + _T(" close (no lock)"), QueueFilename);
		lock_file.close(); // Needed to release lock on .... lock file!
		do {
			//WriteLockQueue((CString)__FUNCTION__ + _T(" remove (no lock)"), QueueFilename); 
			remove(lock_filename.c_str());
		}  while (file_exists(lock_filename.c_str())); //makes sure lock is removed
	} else lock_file.close();
}

void GetLockQueue(const CString text, const CString QueueFilename) {
	std::string lock_filename = CString2string((CString)QueueFilename).substr(0, CString2string((CString)QueueFilename).find_last_of(".")) + QUEUE_LOCK_EXT;

	do {
		std::ifstream lock_file(lock_filename, std::ifstream::in);
		//std::ifstream lock_file(lock_filename);
		//lock_file.close();
		if (lock_file) {
			lock_file.close(); // Needed to release lock on .... lock file!
			Sleep(100 + rand() % 400);
		}
		else {
			lock_file.close();
			break;
		}
	} while (TRUE);
	LockQueue(text, QueueFilename);
}

// For Debug

void WriteLockQueue(const CString text, const CString QueueFilename)
{
	std::string lock_filename = CString2string((CString)QueueFilename).substr(0, CString2string((CString)QueueFilename).find_last_of(".")) + QUEUE_LOCK_EXT;

	//std::ofstream lock_file(lock_filename, std::ofstream::app);
	std::ofstream lock_file(lock_filename);
	if (lock_file) {
		std::string text_string = CString2string(text); 	// do not know why, but need to convert to string then back to cstring to have it work
		lock_file << text_string.c_str() << "\n";
		lock_file.flush();
	}
	lock_file.close();
}

int NbItemFromQueue(const CString tag, const CString QueueFilename, const BOOL use_lock)
{
	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__ + _T(" ") + tag, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//std::ifstream	DeTeCtQueueFile(QueueFilename, std::ifstream::in, sharing_flag);
	std::ifstream	DeTeCtQueueFile(QueueFilename, std::ifstream::in);
	CT2A tmp(tag + _T(": "));
	std::string		tag_string(tmp);
	int				nbitem = 0;

	if (DeTeCtQueueFile) {
		std::string line;
		while (std::getline(DeTeCtQueueFile, line)) if (line.find(tag_string) != std::string::npos) nbitem++;
	}
	DeTeCtQueueFile.close();
	if (use_lock) UnlockQueue(QueueFilename);
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

void PushFileToQueue(const CString objectname, const CString QueueFilename)
{
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname, QueueFilename);
	//std::ofstream DeTeCtQueueFile(QueueFilename, std::ofstream::app, _SH_DENYRW);
	std::ofstream DeTeCtQueueFile(QueueFilename, std::ofstream::app);
	CT2A objectnamechar(L"file: " + objectname);
	DeTeCtQueueFile << objectnamechar << "\n";
	DeTeCtQueueFile.close();
	UnlockQueue(QueueFilename);
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

BOOL PopFileFromQueue(CString *objectname, const CString QueueFilename, const BOOL use_lock)
{
	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//else WriteLockQueue((CString)__FUNCTION__ + _T("(no lock)"), QueueFilename);

	//std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in, sharing_flag);
	std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in);

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
				if (use_lock) UnlockQueue(QueueFilename);							// Before RemoveFileFromQueue which also looks Queue
				RemoveFileFromQueue((*objectname), QueueFilename, FALSE);
				//RemoveFileFromQueue((*objectname), QueueFilename, TRUE);
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	if (use_lock) UnlockQueue(QueueFilename);
	return FALSE;
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

void RemoveFileFromQueue(const CString objectname, const CString QueueFilename, const BOOL use_lock)
{
	int sharing_flag = _SH_DENYNO;
	if (use_lock) {
		GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname, QueueFilename);
		sharing_flag = _SH_DENYRW;
	}
	//std::ifstream DeTeCtQueueInputFile(QueueFilename, std::ifstream::in, sharing_flag);
	std::ifstream DeTeCtQueueInputFile(QueueFilename, std::ifstream::in);

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

		CT2A QueueFilenameChar(QueueFilename);
		//if ((remove(QueueFilenameChar) == 0) && (linesnb > 0)) {
			//std::ofstream DeTeCtQueueOutputFile(QueueFilename, std::ofstream::out, sharing_flag);
		if (linesnb > 0) {
			//std::ofstream DeTeCtQueueOutputFile(QueueFilename, std::ofstream::out|std::ofstream::trunc, sharing_flag);
			std::ofstream DeTeCtQueueOutputFile(QueueFilename, std::ofstream::out | std::ofstream::trunc);
			std::ostream_iterator<std::string> output_iterator(DeTeCtQueueOutputFile, "\n");
			std::copy(lines.begin(), lines.end(), output_iterator);
			DeTeCtQueueOutputFile.close();
		}
	}
	else 	DeTeCtQueueInputFile.close();
	if (use_lock) UnlockQueue(QueueFilename);
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

BOOL IsFileAlreadyQueued(const CString objectname, const CString QueueFilename)
{
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname, QueueFilename);
	//std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in, _SH_DENYRW);
	std::ifstream DeTeCtQueueFile(QueueFilename, std::ifstream::in);

	if (DeTeCtQueueFile) {
		std::string line;
		CT2A tmp(L"file: " + objectname);
		std::string objectstring(tmp);

		while (std::getline(DeTeCtQueueFile, line)) {
			while (line.substr(line.size() - 1, 1) == " ") line.erase(line.size() - 1, 1);
			if (line.compare(objectstring) == 0) {
				DeTeCtQueueFile.close();
				UnlockQueue(QueueFilename);
				return TRUE;
			}
		}
	}
	DeTeCtQueueFile.close();
	UnlockQueue(QueueFilename);
	return FALSE;
}


int  NbFilesFromQueue(const CString QueueFilename)
{
	GetLockQueue((CString)__FUNCTION__, QueueFilename);
	int nbfiles = NbItemFromQueue((CString)"file", (CString)QueueFilename, FALSE) + NbItemFromQueue((CString)"file_processing", (CString)QueueFilename, FALSE)
		+ NbItemFromQueue((CString)"file_processed ", (CString)QueueFilename, FALSE) + NbItemFromQueue((CString)"file_ok        ", (CString)QueueFilename, FALSE) + NbItemFromQueue((CString)"file_ko        ", (CString)QueueFilename, FALSE);
	//int nbfiles = NbItemFromQueue((CString)"file", (CString)QueueFilename, TRUE) + NbItemFromQueue((CString)"file_processing", (CString)QueueFilename, TRUE)
	//	+ NbItemFromQueue((CString)"file_processed ", (CString)QueueFilename, TRUE) + NbItemFromQueue((CString)"file_ok        ", (CString)QueueFilename, TRUE) + NbItemFromQueue((CString)"file_ko        ", (CString)QueueFilename, TRUE);
	UnlockQueue(QueueFilename);
	return nbfiles;
}

BOOL GetFileFromQueue(CString *objectname, const CString QueueFilename)
{
	//if (!file_exists(CString2string(QueueFilename))) exit(EXIT_FAILURE);	// exits DeTeCt if Queuefile does not exists

	GetLockQueue((CString)__FUNCTION__, QueueFilename);
	if (PopFileFromQueue(objectname, QueueFilename, FALSE)) {
		if (!IsItemAlreadyQueued(*objectname, _T("file_processing"), QueueFilename, FALSE)) {
			PushItemToQueue(*objectname, _T("file_processing"), QueueFilename, FALSE);
			UnlockQueue(QueueFilename);
			return TRUE;
		}
	}
	/*if (PopFileFromQueue(objectname, QueueFilename, TRUE)) {
		if (!IsItemAlreadyQueued(*objectname, _T("file_processing"), QueueFilename, TRUE)) {
			PushItemToQueue(*objectname, _T("file_processing"), QueueFilename, TRUE);
			UnlockQueue(QueueFilename);
			return TRUE;
		}
	}*/
	UnlockQueue(QueueFilename);
	return FALSE;
}

void SetFileProcessingFromQueue(const CString objectname, const CString QueueFilename) {
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname, QueueFilename);
	if (!IsItemAlreadyQueued(objectname, _T("file_processing"), QueueFilename, FALSE)) {
		RemoveItemsFromQueue(objectname, _T("file"), QueueFilename, FALSE);
		PushItemToQueue(objectname, _T("file_processing"), QueueFilename, FALSE);
	}
	/*if (!IsItemAlreadyQueued(objectname, _T("file_processing"), QueueFilename, TRUE)) {
		RemoveItemsFromQueue(objectname, _T("file"), QueueFilename, TRUE);
		PushItemToQueue(objectname, _T("file_processing"), QueueFilename, TRUE);
	}*/
	UnlockQueue(QueueFilename);
}

void SetProcessingFileProcessedFromQueue(const CString objectname_cstring, const CString details, const CString tag, const CString QueueFilename) {
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + objectname_cstring + _T(" ") + (CString)details + _T(" ") + tag, QueueFilename);
	PushItemToQueue(objectname_cstring + details, tag, QueueFilename, FALSE);
	//PushItemToQueue(objectname_cstring + details, tag, QueueFilename, TRUE);
	RemoveItemsFromQueue(objectname_cstring, _T("file_processing"), QueueFilename, FALSE);
	//RemoveItemsFromQueue(objectname_cstring, _T("file_processing"), QueueFilename, TRUE);
	UnlockQueue(QueueFilename);
}


BOOL GetProcessedFileFromQueue(CString *processed_filename, CString *processed_filename_acquisition, CString *processed_message, Rating_type *processed_rating, double *duration, int *nframe_child, int *fps_int_child, const CString tag, const CString QueueFilename)
{
	if (!file_exists(CString2string(QueueFilename))) exit(EXIT_FAILURE);  	// exits DeTeCt if Queuefile does not exists
	CString processed_line;
	BOOL status;
	if (!GetItemFromQueue(&processed_line, tag, QueueFilename)) return FALSE;

	(*duration)			= 0;
	(*nframe_child)		= 0;
	(*fps_int_child)	= 0;
	GetLockQueue((CString)__FUNCTION__ + _T(" ") + tag, QueueFilename);
	std::string tmp_line(CString2string(processed_line));
	std::string tag_string = CString2string(tag);
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

	RemoveItemsFromQueue(processed_line,				_T("file_processed "), (CString)QueueFilename, FALSE);
	if (status)	PushItemToQueue(processed_line,			_T("file_ok        "), (CString)QueueFilename, FALSE);
	else		PushItemToQueue(processed_line,			_T("file_ko        "), (CString)QueueFilename, FALSE);
	/*RemoveItemsFromQueue(processed_line, _T("file_processed "), (CString)QueueFilename, TRUE);
	if (status)	PushItemToQueue(processed_line, _T("file_ok        "), (CString)QueueFilename, TRUE);
	else		PushItemToQueue(processed_line, _T("file_ko        "), (CString)QueueFilename, TRUE);*/

	UnlockQueue(QueueFilename);
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

BOOL IsProcessRunning(const DWORD pid)
{
	/*** Check current running processes ***/
	HANDLE hndl = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS | TH32CS_SNAPMODULE, 0);
	if (hndl)
	{
		PROCESSENTRY32  process = { sizeof(PROCESSENTRY32) };
		Process32First(hndl, &process);
		do
		{
			if (process.th32ProcessID == pid) {
				CloseHandle(hndl);
				return TRUE;
			}
		} while (Process32Next(hndl, &process));
		CloseHandle(hndl);
	}
	return FALSE;;
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
