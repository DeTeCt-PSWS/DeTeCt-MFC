
// DeTeCt-MFC.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

//#include "auxfunc.hpp"
//#include "resource.h"		// main symbols
#include "DeTeCt-MFCDlg.hpp"

//#include "dtcgui.hpp"

// CDeTeCtMFCApp:
// See DeTeCt-MFC.cpp for the implementation of this class
//

class CDeTeCtMFCApp : public CWinApp
{
public:
	CDeTeCtMFCApp();

// Overrides
public:
	virtual BOOL InitInstance();
	
// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CDeTeCtMFCApp	theApp;
extern CDeTeCtMFCDlg	dlg;

void CreateQueueFileName();