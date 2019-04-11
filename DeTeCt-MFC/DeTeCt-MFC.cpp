
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "dtcas3.h"
#include "ini.h"
#include "cmdline.h"

//#include <iostream>
#include <fstream>
//#include <sstream>
//#include <cstdio>

#include <experimental/filesystem>
namespace filesys = std::experimental::filesystem;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string full_version;

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
CDeTeCtMFCDlg dlg;

// CDeTeCtMFCApp initialization

/**********************************************************************************************//**
 * @fn	BOOL CDeTeCtMFCApp::InitInstance()
 *
 * @brief	Initializes the program. 
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL CDeTeCtMFCApp::InitInstance()
{
	CString commandLineArgument = GetCommandLine();
	CStringArray commandParametres;
	int i = 0;
	std::string object;
	std::string target_file;
	std::string target_folder;
	target_file.clear();
	target_folder.clear();
	std::string compilation_date; 
	std::string year;
	std::string month;
	std::string month_string;
	std::string day;
	std::string months[] = { "Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

	opts.interactive = TRUE;

/* builds full program name with version, compilation date and platform */
	compilation_date.append(__DATE__);
	month_string = compilation_date.substr(0, 3);
	for (i = 0; i < 12; i++) {
		if (month_string.compare(months[i]) == 0) {
			std::string m = std::to_string(i+1);
			month.append("0");
			month.append(m);
			if (month.size() > 2) month.erase(0, 1);
		}
	}
	if (compilation_date.size() == 11) day = compilation_date.substr(4, 2);
	else {
		day.append("0");
		day.append(compilation_date.substr(4, 1));
	}
	year = compilation_date.substr(compilation_date.size() - 4, 4);
	full_version.append(PROGNAME " v" VERSION_NB ".");
	full_version.append(year);
	full_version.append(month);
	full_version.append(day);
	full_version.append("_" DETECT_TARGET);
	//" (" __DATE__ ")");

	for (CString sItem = commandLineArgument.Tokenize(L" ", i); i >= 0; sItem = commandLineArgument.Tokenize(L" ", i)) {
		commandParametres.Add(sItem);
	}
	for (int j = 1; j < commandParametres.GetCount(); j++) {
		CString parameter = commandParametres.GetAt(j);
		std::wstring wparam(parameter);
		std::string param(wparam.begin(), wparam.end());
		param = param.substr(param.find_first_of(" ") + 1, param.length());
		while (param.find('\n') != std::string::npos) {
			param.erase(param.find('\n'), 2);
		}

		if (starts_with(param, "-")) {
			DBOUT("option : " << param.c_str() << "\n");
			if (starts_with(param, "-automatic")) {
				opts.interactive = FALSE;
			}
		}
		else {
			object = param;
			std::ifstream file(object);
			if (file) {
				std::string extension;
				extension = object.substr(object.find_last_of('.'), object.size() - object.find_last_of('.'));
				std::wstring filter_wstring(filter);
				std::string filter_string(filter_wstring.begin(), filter_wstring.end());
				filter_string = filter_string.substr(filter_string.find_first_of('|') + 1, filter_string.size() - filter_string.find_first_of('|') - 3);
				filter_string.push_back(';');
				extension.insert(0,1,'*');
				extension.push_back(';');
				if (filter_string.rfind(extension) != std::string::npos) {
					target_file = object;
					//DBOUT("file = " << target_file.c_str() << "\n");
					opts.filename = new char[target_file.size() + 1];
					std::copy(target_file.begin(), target_file.end(), opts.filename);
					opts.filename[target_file.size()] = '\0';
					DBOUT("file = " << opts.filename << "\n");
				}	else {
					DBOUT("WARNING: Extension : " << extension.c_str() << " not supported\n");
				}
			} else {
				DIR *folder_object;
				if ((folder_object = opendir(object.c_str()))) {
					target_folder = object; 
					//DBOUT("folder = " << target_folder.c_str() << "\n");
					opts.dirname = new char[target_folder.size() + 1];
					std::copy(target_folder.begin(), target_folder.end(), opts.dirname);
					opts.dirname[target_folder.size()] = '\0';
					DBOUT("folder = " << target_folder.c_str() << "\n");
				}
				else {
					DBOUT("WARNING: Object : " << object.c_str() << " not found\n");
				}
			}
		}
	}
	if (opts.filename > 0) {
		DBOUT("file = " << opts.filename << "\n");
	}
	if (opts.dirname > 0) {
		DBOUT("folder = " << target_folder.c_str() << "\n");
	}
	//Sleep(5000); 
//clock_t start = clock();
//detect_autostakkert(file);
//clock_t end = clock();

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

	//CDeTeCtMFCDlg dlg;
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

