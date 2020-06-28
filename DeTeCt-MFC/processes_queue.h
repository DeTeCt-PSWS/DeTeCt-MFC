#pragma once

#include "stdafx.h"
#include "dtc.h"

// ************** DeTeCt file and folder functions **********
char	*DeTeCtFileName(char *DeTeCtFileNameChar);
CString  DeTeCt_additional_filename_exe_fullpath(CString suffix);
CString  DeTeCt_additional_filename_from_folder(const CString foldername, const CString suffix);
CString  DeTeCt_exe_folder();

// ************** DeTeCt process queue management **********
void	PushItemToQueue			(const CString line, const CString tag, CString QueueFilename, const BOOL use_lock);
BOOL	GetItemFromQueue		(CString *objectname, const CString tag, const CString QueueFilename);
int		NbItemFromQueue			(const CString tag, const CString QueueFilename, const BOOL use_lock);
void	RemoveItemsFromQueue	(const CString objectname, const CString tag, CString QueueFilename, const BOOL use_lock);
void	SetIntParamToQueue		(const int param, const CString tag, const CString QueueFilename);
int		GetIntParamFromQueue(const CString tag, const CString QueueFilename);

BOOL	IsItemAlreadyQueued		(const CString objectname, const CString tag, const CString QueueFilename, const BOOL use_lock);

BOOL	IsFileAlreadyQueued		(const CString objectname, const CString QueueFilename);
void	PushFileToQueue			(const CString objectname, const CString QueueFilename);
void	RemoveFileFromQueue		(const CString objectname, const  CString QueueFilename, const BOOL use_lock);
BOOL	PopFileFromQueue		(CString *objectname, const CString QueueFilename, const BOOL use_lock);

void	LockQueue				(const CString text, const CString QueueFilename);
void	UnlockQueue				(const CString QueueFilename);
void	GetLockQueue			(const CString text, const CString QueueFilename);

int		NbFilesFromQueue		(const CString QueueFilename);
BOOL	GetFileFromQueue		(CString *objectname, const CString QueueFilename);
void	SetFileProcessingFromQueue(const CString objectname, const CString QueueFilename);
void	SetProcessingFileProcessedFromQueue(const CString objectname, const CString details, const CString tag, const CString QueueFilename);
BOOL	GetProcessedFileFromQueue(CString *processed_filename, CString *processed_filename_acquisition, CString *processed_message, Rating_type *processed_rating, double *duration, int *nframe_child, int *fps_int_child, const CString tag, const CString QueueFilename);

// ************** Process functions **********
int		DetectInstancesNumber();
int		ProcessRunningInstancesNumber(const char *ProcessFilename);
BOOL	IsProcessRunning(const DWORD pid);

DWORD	getParentPID(const DWORD pid);
int		getProcessName(const DWORD pid, wchar_t *fname, DWORD sz);
int		KillsChildrenProcesses();
int		ChildrenProcessesNumber();
int		ParentChildrenProcessesNumber(const DWORD parent_PID);