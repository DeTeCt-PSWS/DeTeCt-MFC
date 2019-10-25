
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "dtcas3.h"
//#include "ini.h"
#include "cmdline.h"

#include <iostream>
using namespace std;
#include <fstream>
//#include <sstream>
//#include <cstdio>
#include "processes_queue.h"

#include <experimental/filesystem>
namespace filesys = std::experimental::filesystem;
#include "direct.h"

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
	char DeTeCtNameChar[MAX_STRING];
	opts.interactive = TRUE;
	opts.reprocessing = TRUE;

	DeTeCtFileName(DeTeCtNameChar);
	//std::string DeTeCtName = "DeTeCt.exe";
	std::string DeTeCtName(DeTeCtNameChar);

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
	LPWSTR *szArglist;
	int nArgs;
	int idx;
	int index_message = 0;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		wprintf(L"CommandLineToArgvW failed\n");
	}
	else for (idx = 0; idx < nArgs; idx++) {
		commandParametres.Add(szArglist[idx]);
		//opts.message[index_message++] = CW2A(szArglist[idx]) + " parameter";
	}
	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);
	
	opts.message[index_message] = "\0";
	std::for_each(DeTeCtName.begin(), DeTeCtName.end(), [](char & c) {
		c = ::tolower(c);
	});
	for (int j = 0; j < commandParametres.GetCount(); j++) {
		CString parameter = commandParametres.GetAt(j);
		/*CT2CA parameter_ansi_string(parameter);
		std::string param(parameter_ansi_string);
		std::string param_org(parameter_ansi_string);*/
		std::wstring wparam(parameter);
		std::string param(wparam.begin(), wparam.end());
		std::string param_org(wparam.begin(), wparam.end());
		
		std::for_each(param.begin(), param.end(), [](char & c) {
			c = ::tolower(c);
		});
		while (param.find('\n') != std::string::npos) param.erase(param.find('\n'), 2);
		if (starts_with(param, "-")) {
			DBOUT("option : " << param.c_str() << "\n");
			if (starts_with(param, "-automatic")) opts.interactive = FALSE;
			else if (starts_with(param, "-interactive"))  opts.interactive = TRUE;	
			else if (starts_with(param, "-noreprocessing")) opts.reprocessing = FALSE;
			else if (starts_with(param, "-reprocessing")) opts.reprocessing = TRUE;
			else if (starts_with(param, "-nozip")) opts.zip = FALSE;
			else if (starts_with(param, "-help")) {
				opts.message[index_message++] = "Info : syntax : " + DeTeCtName + " [options] filename | foldername";
				opts.message[index_message++] = "   -automatic           automatic mode launching detection and exiting without interaction";
				opts.message[index_message++] = "   -noreprocessing   no reprocessing acquisition files already in DeTeCt.log";
				opts.message[index_message++] = "   -nozip                  no generation of impact detection zip file";
				opts.message[index_message++] = "   filename              name of acquisition/autostakkert session file to be processed";
				opts.message[index_message++] = "   foldername          name of folder to be process with all its subfolders";
				opts.message[index_message] = "\0";
			}
			else {
				opts.message[index_message++] = "ERROR : " + param + " option not recognized";
				opts.message[index_message] = "\0";
			}
		}
//		else if ((param.find(DeTeCtName) == std::string::npos) && !starts_with(param, "/")) {
		else if ((param.find(DeTeCtName) == std::string::npos)) {
			object = param_org;
//			while (object.find_first_of('"') != std::string::npos) object.erase(object.find_first_of('"'), 1);
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
					if ((starts_with(object, ".")) || !((starts_with(object, "\\")) || (object.find(":\\") == 1))) {
						//CT2CA tmp_conv(DeTeCt_exe_folder());
						//target_folder = std::string (tmp_conv);
						char buffer[MAX_STRING];
						_fullpath(buffer, object.c_str(), MAX_STRING);
						target_file = std::string(buffer);
					}
					else target_file = object;

					opts.filename = new char[target_file.size() + 1];
					std::copy(target_file.begin(), target_file.end(), opts.filename);
					opts.filename[target_file.size()] = '\0';
					DBOUT("file = " << opts.filename << "\n");
					std::string tmp_string(opts.filename);
					opts.message[index_message++] = "Using file " + tmp_string;

				}	else {
					opts.message[index_message++] = "WARNING: Extension : " + extension + " not supported";
					opts.message[index_message] = "\0";
					DBOUT("WARNING: Extension : " << extension.c_str() << " not supported\n");
				}
			} else {
				DIR *folder_object;
//opts.message[index_message++] = "file or folder " + object;
				if ((folder_object = opendir(object.c_str()))) {
//convert relative path to absolute path (does not work from MSVC debug as exe launched from MFC directory)
					if ((starts_with(object,".")) || !((starts_with(object, "\\")) || (object.find(":\\") == 1))) {
						//CT2CA tmp_conv(DeTeCt_exe_folder());
						//target_folder = std::string (tmp_conv);
						char buffer[MAX_STRING];
						_fullpath(buffer, object.c_str(), MAX_STRING);
						target_folder = std::string(buffer);
					}
					else target_folder = object;

					opts.dirname = new char[target_folder.size() + 1];
					std::copy(target_folder.begin(), target_folder.end(), opts.dirname);
					opts.dirname[target_folder.size()] = '\0';
					DBOUT("folder = " << target_folder.c_str() << "\n");
					std::string tmp_string(opts.dirname);
					opts.message[index_message++] = "Using directory " + tmp_string;
				}
				else {
					DBOUT("WARNING: Object : " << object.c_str() << " not found\n");
					opts.message[index_message++] = "ERROR : file or folder " + object + " not found";
					opts.message[index_message] = "\0";

				}
				closedir(folder_object);
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
				file._close();
			}
			else {
				DIR *folder_object;
				if (folder_object = opendir(tmp)) {
					opts.dirname = new char[strlen(tmp) + 1];
					strcpy(opts.dirname, tmp);
					opts.dirname[strlen(tmp)] = '\0';
					closedir(folder_object);
				}
				else DBOUT("parameter = " << objectname << " not found\n");
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

