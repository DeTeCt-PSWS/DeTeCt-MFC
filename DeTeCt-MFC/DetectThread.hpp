#ifndef __DETECTTHREAD_H_
#define __DETECTTHREAD_H_

//#include "afxwin.h"
#include "DeTeCt-MFCDlg.hpp"
//#include "dtcgui.hpp"
//#include <string>
//#include <vector>

struct _ImpactDetectParams {
	std::vector<std::string> file_list;
	OPTS *popts;
	std::string scan_folder_path;

	_ImpactDetectParams(std::vector<std::string> fl, OPTS *o, std::string sfp) {
		file_list = fl;
		popts = o;
		scan_folder_path = sfp;
	}
};

typedef struct _ImpactDetectParams ImpactDetectParams;

UINT __cdecl impactDetection(LPVOID pParam);
//UINT __cdecl impactDetection(ImpactDetectParams* pParam);

#endif