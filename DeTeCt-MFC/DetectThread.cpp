#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include "DetectThread.h"
#include "cmdline.h"
#include <queue>
#include <iostream>
#include <ctime>

#include "DeTeCt-MFCDlg.h"
#include "afxwin.h"

UINT __cdecl impactDetection(LPVOID pParam) {
	ImpactDetectParams* params = (ImpactDetectParams*)pParam;
	return detect(params->file_list, params->opts, params->scan_folder_path);
}