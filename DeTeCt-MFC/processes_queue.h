#pragma once

#include "stdafx.h"

int DectectInstancesNumber();

char *DeTeCtFileName(char *DeTeCtFileNameChar);

CString  DeTeCt_additional_filename_exe_folder(CString suffix);

CString  DeTeCt_additional_filename(CString folder, CString suffix);

CString  DeTeCt_exe_folder();

BOOL IsAlreadyQueued(CString objectname, CString DeTeCtQueueFilename);

void RemoveFromQueue(CString objectname, CString DeTeCtQueueFilename);

void PushToQueue(CString objectname, CString DeTeCtQueueFilename);

BOOL PopFromQueue(CString *objectname, CString DeTeCtQueueFilename);
