
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "dtcas3.h"
//#include "ini.h"
#include "cmdline.h"

//#include <iostream>
#include <fstream>
//#include <sstream>
//#include <cstdio>
#include "processes_queue.h"

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
	std::string DeTeCtName = "DeTeCt.exe";
	opts.interactive = TRUE;
	opts.reprocessing = TRUE;

/*** builds full program name with version, compilation date and platform ***/
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
	if (compilation_date.substr(4,1) != " ") day = compilation_date.substr(4, 2);
	else {
		day.append("0");
		day.append(compilation_date.substr(5, 1));
	}
	year = compilation_date.substr(compilation_date.size() - 4, 4);
	full_version.append(PROGNAME " v" VERSION_NB ".");
	full_version.append(year);
	full_version.append(month);
	full_version.append(day);
	full_version.append("_" DETECT_TARGET);
	
/*** Analyse command line parameters ***/
	for (CString sItem = commandLineArgument.Tokenize(L" ", i); i >= 0; sItem = commandLineArgument.Tokenize(L" ", i)) {
		commandParametres.Add(sItem);
	}
	for (int j = 0; j < commandParametres.GetCount(); j++) {
		CString parameter = commandParametres.GetAt(j);
		std::wstring wparam(parameter);
		std::string param(wparam.begin(), wparam.end());
		param = param.substr(param.find_first_of(" ") + 1, param.length());
		while (param.find('\n') != std::string::npos) {
			param.erase(param.find('\n'), 2);
		}
//CString SuffixString(DTC_QUEUE_SUFFIX);
//CString DeTeCtQueueFilename = DeTeCt_additional_filename_exe_folder(SuffixString);
//PushToQueue(L"Interactive?", DeTeCtQueueFilename);
//PushToQueue(parameter, DeTeCtQueueFilename);
		if (starts_with(param, "-")) {
			DBOUT("option : " << param.c_str() << "\n");
			if (starts_with(param, "-automatic")) opts.interactive = FALSE;
			else if (starts_with(param, "-interactive"))  opts.interactive = TRUE;	
			else if (starts_with(param, "-noreprocessing")) opts.reprocessing = FALSE;
			else if (starts_with(param, "-reprocessing")) opts.reprocessing = TRUE;
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
	if ((!opts.interactive) && (!opts.filename) && (!opts.dirname)) {
		CString objectname; 
		if (PopFromQueue(&objectname, DeTeCt_additional_filename_exe_folder(DTC_QUEUE_SUFFIX))) {
			std::ifstream file(objectname);
			CT2A tmp(objectname);
			if (file) {
				opts.filename = new char[strlen(tmp)+1];
				strcpy(opts.filename, tmp);
				opts.filename[strlen(tmp)] = '\0';
			} else {
				DIR *folder_object;
				if (folder_object = opendir(tmp)) {
					opts.dirname = new char[strlen(tmp)+1];
					strcpy(opts.dirname, tmp);
					opts.dirname[strlen(tmp)] = '\0';
				}
			}
		}
	}
	if (opts.filename) DBOUT("file = " << opts.filename << "\n");
	if (opts.dirname) DBOUT("folder = " << opts.dirname << "\n");

/*** Check if DeTeCt is already running ***/
	int DeTeCt_processes_nb = DectectInstancesNumber();
	DBOUT("Maximum DeTeCt processes = " << opts.maxinstances << "\n");
	DBOUT("Number of DeTeCt processes = " << DeTeCt_processes_nb << "\n");
// tests if logging file/folder to process needed
	if ((DeTeCt_processes_nb > opts.maxinstances) && (!opts.interactive)) {
//log processes to be done and exits
		CString DeTeCtQueueFilename = DeTeCt_additional_filename_exe_folder(DTC_QUEUE_SUFFIX);
		if (opts.filename) {
			CString objectname(opts.filename);
			if (!IsAlreadyQueued(objectname, DeTeCtQueueFilename)) PushToQueue(objectname, DeTeCtQueueFilename);
			else DBOUT("file already queued\n");
		} else if (opts.dirname) {
			CString objectname(opts.dirname);
			if (!IsAlreadyQueued(objectname, DeTeCtQueueFilename)) PushToQueue(objectname, DeTeCtQueueFilename);
			else DBOUT("directory already queued\n");
		}		
		return FALSE;
	}

//clock_t start = clock();
//clock_t end = clock();

// InitCommonControlsEx() is required on Windows XP if an application
// manifest specifies use of ComCtl32.dll version 6 or later to enable
// visual styles.  Otherwise, any window creation will fail.
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
// Set this to include all the common control classes you want to use in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

// Standard initialization
// If you are not using these features and wish to reduce the size of your final executable, you should remove from the following
// the specific initialization routines you do not need
// Change the registry key under which our settings are stored
// TODO: You should modify this string to be something appropriate such as the name of your company or organization
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));

	m_pMainWnd = &dlg;
	INT_PTR nResponse = dlg.DoModal();

	HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
//   void DisableMinimizeButton(HWND hWnd)   {
//	   SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_MINIMIZEBOX);
//   }
//   void EnableMinimizeButton(HWND hWnd) {
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_MINIMIZEBOX);
	//   }

//   void DisableMaximizeButton(HWND hWnd) {
//	   SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
//   }
//   void EnableMaximizeButton(HWND hWnd) {
	SetWindowLong(hWnd, GWL_STYLE, GetWindowLong(hWnd, GWL_STYLE) | WS_MAXIMIZEBOX);
//   }

	if (nResponse == IDOK)
	{
		// TODO: Place code here to handle when the dialog is  dismissed with OK
	}
	else if (nResponse == IDCANCEL)
	{
		// TODO: Place code here to handle when the dialog is dismissed with Cancel
	}
	else if (nResponse == -1)
	{
		TRACE(traceAppMsg, 0, "Warning: dialog creation failed, so application is terminating unexpectedly.\n");
	}

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	// Since the dialog has been closed, return FALSE so that we exit the application, rather than start the application's message pump.
	return TRUE;
}

