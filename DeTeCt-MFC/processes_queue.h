#pragma once

#include "stdafx.h"

char *DeTeCtFileName(char *DeTeCtFileNameChar);

int DectectInstancesNumber();

CString  DeTeCt_additional_filename_exe_folder(CString suffix);

CString  DeTeCt_additional_filename(CString folder, CString suffix);

BOOL IsAlreadyQueued(CString objectname, CString DeTeCtQueueFilename);

void RemoveFromQueue(CString objectname, CString DeTeCtQueueFilename);

void PushToQueue(CString objectname, CString DeTeCtQueueFilename);

BOOL PopFromQueue(CString *objectname, CString DeTeCtQueueFilename);
