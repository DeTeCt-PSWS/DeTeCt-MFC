#pragma once

#include "stdafx.h"
#include "dtc.h"

enum class _Rating_type { Error, Null, Low, High };
typedef enum _Rating_type Rating_type;

// ************** DeTeCt file and folder functions **********
char	*DeTeCtFileName(char *DeTeCtFileNameChar);
CString  DeTeCt_additional_filename_exe_fullpath(CString suffix);
CString  DeTeCt_additional_filename_from_folder(const CString foldername, const CString suffix);
CString  DeTeCt_exe_folder();

// ************** DeTeCt process queue management **********	
BOOL	PushItemToQueue			(const CString line, const CString tag, CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);			//API+internal	-W-
BOOL	GetItemFromQueue		(CString *objectname, const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);	//API+internal	R--
int		NbItemFromQueue			(const CString tag, const CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);							//API+internal	R--
void	RemoveItemsFromQueue	(const CString objectname, const CString tag, CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);		//API+internal	--D
void	SetIntParamToQueue		(const int param, const CString tag, const CString QueueFilename);																	//API			-WD
int		GetIntParamFromQueue(const CString tag, const CString QueueFilename);																						//API			R--

BOOL	IsFileAlreadyQueued		(const CString objectname, const CString QueueFilename);																			//API			R--
void	PushFileToQueue			(const CString objectname, const CString QueueFilename);																			//API			-W-
void	RemoveFileFromQueue		(const CString objectname, const  CString QueueFilename, HANDLE* pQueueFileHandle, const BOOL close_handle_at_end);					//API+internal	--D

int		NbFilesFromQueue		(const CString QueueFilename);																										//API			R--
BOOL	GetFileFromQueue		(CString *objectname, const CString QueueFilename);																					//API			-WD
void	SetFileProcessingFromQueue(const CString objectname, const CString QueueFilename);																			//API			-WD
void	SetProcessingFileProcessedFromQueue(const CString objectname, const CString details, const CString tag, const CString QueueFilename);						//API			-WD
BOOL	GetProcessedFileFromQueue(CString *processed_filename, CString *processed_filename_acquisition, CString *processed_message, Rating_type *processed_rating, double *duration, int *nframe_child, int *fps_int_child, const CString QueueFilename);  //API	-WD

BOOL OpenRWQueueFile(const CString QueueFilename, HANDLE* pQueueFileHandle);

// ************** Process functions **********
int		ProcessRunningInstancesNumber(const char *ProcessFilename);
BOOL	IsProcessRunning(const DWORD pid);

DWORD	getParentPID(const DWORD pid);
int		getProcessName(const DWORD pid, wchar_t *fname, DWORD sz);
int		KillsChildrenProcesses();
int		ChildrenProcessesNumber();
int		ParentChildrenProcessesNumber(const DWORD parent_PID);