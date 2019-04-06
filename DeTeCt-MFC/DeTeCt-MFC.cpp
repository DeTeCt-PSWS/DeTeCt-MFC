
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "dtcas3.h"
#include "ini.h"
#include "cmdline.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CDeTeCtMFCApp

BEGIN_MESSAGE_MAP(CDeTeCtMFCApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDeTeCtMFCApp construction

CDeTeCtMFCApp::CDeTeCtMFCApp()
{
	// support Restart Manager
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDeTeCtMFCApp object

CDeTeCtMFCApp theApp;


// CDeTeCtMFCApp initialization

/**********************************************************************************************//**
 * @fn	BOOL CDeTeCtMFCApp::InitInstance()
 *
 * @brief	Initializes the program. if the parameters include the word "-autostakkert" a console instance
 * 			launches running the algorithm for that file (unfinished).
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL CDeTeCtMFCApp::InitInstance()
{
	CString commandLineArgument = GetCommandLine();


	int i = 0;
	CStringArray commandParametres;
	for (CString sItem = commandLineArgument.Tokenize(L" ", i); i >= 0; sItem = commandLineArgument.Tokenize(L" ", i)) {
		commandParametres.Add(sItem);
	}
	if (commandParametres.GetCount() > 1 && !commandParametres.GetAt(1).Compare(L"-autostakkert")) {
		AllocConsole();
		freopen("CONOUT$", "w", stdout);
		std::cout << "Autostakkert mode" << std::endl;

		CString filePath = commandParametres.GetAt(2);
		std::wstring wfile(filePath);
		std::string file(wfile.begin(), wfile.end());
		file = file.substr(file.find_first_of(" ") + 1, file.length());
		while (file.find('\n') != std::string::npos) {
			file.erase(file.find('\n'), 2);
		}
		std::cout << "file to be processed : " << file << std::endl;
Sleep(5000); 
		clock_t start = clock();
		detect_autostakkert(file);
		clock_t end = clock();
		std::cout << "Algorithm has finished executing in " << int(end - start) / CLOCKS_PER_SEC << " seconds." << std::endl;
		Sleep(10000);
		return FALSE;
	} else {
	// InitCommonControlsEx() is required on Windows XP if an application
	// manifest specifies use of ComCtl32.dll version 6 or later to enable
	// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	// Set this to include all the common control classes you want to use
	// in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();



	// Standard initialization
	// If you are not using these features and wish to reduce the size
	// of your final executable, you should remove from the following
	// the specific initialization routines you do not need
	// Change the registry key under which our settings are stored
	// TODO: You should modify this string to be something appropriate
	// such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	CDeTeCtMFCDlg dlg;
	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();
	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is
		//  dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}


#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the
	//  application, rather than start the application's message pump.
	return TRUE;
	}
}

