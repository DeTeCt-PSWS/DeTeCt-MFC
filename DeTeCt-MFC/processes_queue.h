#pragma once

#include "stdafx.h"

#define AUTOSTAKKERTFILENAME	"AutoStakkert.exe"
#define LAUTOSTAKKERTFILENAME	L"AutoStakkert.exe"

int		AutostakkertInstancesNumber();
int		DectectInstancesNumber();
int		ProcessRunningInstancesNumber(const char *ProcessFilename);

char	*DeTeCtFileName(char *DeTeCtFileNameChar);
CString  DeTeCt_additional_filename_exe_folder(CString suffix);
CString  DeTeCt_additional_filename(const CString foldername, const CString suffix);
CString  DeTeCt_exe_folder();

void	WriteItemToQueue(const CString line, const CString tag, CString DeTeCtQueueFilename);
BOOL	GetItemFromQueue(CString *object, const CString tag, const CString DeTeCtQueueFilename);
BOOL	IsAlreadyQueued(const CString objectname, const CString DeTeCtQueueFilename);
void	PushToQueue(const CString objectname, CString DeTeCtQueueFilename);
void	RemoveFromQueue(const CString objectname, CString DeTeCtQueueFilename);
BOOL	PopFromQueue(CString *objectname, CString DeTeCtQueueFilename);

DWORD	getParentPID(const DWORD pid);
int		getProcessName(const DWORD pid, wchar_t *fname, DWORD sz);
BOOL	IsParentAutostakkert();