#define _WIN32_WINNT _WIN32_WINNT_WINXP

#include "DetectThread.h"
#include "cmdline.h"
#include <queue>
#include <iostream>
#include <ctime>

#include "DeTeCt-MFCDlg.h"
#include "afxwin.h"

UINT __cdecl impactDetection(LPVOID pParam) {
//UINT __cdecl impactDetection(ImpactDetectParams* pParam) {
 //ImpactDetectParams* params = (ImpactDetectParams*)pParam;
//	return detect(params->file_list, &params->opts, params->scan_folder_path);
//	return detect(params->file_list, params->popts, params->scan_folder_path);
	
	char buffer[MAX_STRING] = { 0 };
	sprintf_s(buffer, MAX_STRING, "impactDetection:	popts   : %p	popts->ignore	:	%i\n", ((ImpactDetectParams*)pParam)->popts, (((ImpactDetectParams*)pParam)->popts)->ignore);
	OutputDebugStringA(buffer);
	sprintf_s(buffer, MAX_STRING, "impactDetection:	opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
	OutputDebugStringA(buffer);

	return detect(((ImpactDetectParams*)pParam)->file_list, ((ImpactDetectParams*)pParam)->scan_folder_path);
}