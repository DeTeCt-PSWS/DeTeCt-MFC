
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "common2.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "dtcas3.h"
//#include "ini.h"
#include "cmdline.h"

#include <iostream>
using namespace std;
#include <fstream>
#include "processes_queue.h"

#include <experimental/filesystem>
namespace filesys = std::experimental::filesystem;
#include "direct.h"

#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string full_version;
std::string app_title;

// CDeTeCtMFCApp

BEGIN_MESSAGE_MAP(CDeTeCtMFCApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()


// CDeTeCtMFCApp construction

CDeTeCtMFCApp::CDeTeCtMFCApp()
{
	// support Restart Manager
//m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;

	// TODO: add construction code here,
	// Place all significant initialization in InitInstance
}


// The one and only CDeTeCtMFCApp object

CDeTeCtMFCApp	theApp;
CDeTeCtMFCDlg	dlg;

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
	int index_message = 0;
	srand((unsigned int)time(NULL));

	// ****************************************
	// **************** INIT ******************
	// ****************************************

	DeTeCtFileName(DeTeCtNameChar);
	std::string DeTeCtName(DeTeCtNameChar); // "DeTeCt.exe"

	// Sets default values for options then reads options from ini file (before it could be changed by command line)
	//opts.interactive = TRUE; // -interactive / -noautomatic
	CDeTeCtMFCDlg::CDeTeCtMFCDlg();
	opts.interactive_bak = opts.interactive;

	opts.autostakkert =		FALSE;
	opts.autostakkert_PID = 0;
	opts.detect_PID = 0;
	opts.parent_instance =	FALSE;
	opts.interactive_bak =	opts.interactive;

	// Checks if DeTeCt has been launched by AutoStakkert
	if (IsParentAutostakkert(&opts.autostakkert_PID)) {
		opts.message[index_message++] = "Launched from AutoStakkert, DO NOT CLOSE THIS WINDOW (close AutoStakkert when done)";
		opts.message[index_message] = "\0";
		opts.autostakkert =		TRUE;
		opts.parent_instance =	FALSE;
	}
	else {
		// in case code for displaying ProcessName is needed
		DWORD pid, ppid;
		int e;
		wchar_t wfname[MAX_PATH] = { 0 };
		char fname[MAX_PATH] = { 0 };
		pid = GetCurrentProcessId();
		ppid = getParentPID(pid);
		e = getProcessName(ppid, wfname, MAX_PATH);
		sprintf(fname, "%ws", wfname);
	}
	// builds full program name with version, compilation date and platform
	compilation_date.append(__DATE__);
	month_string = compilation_date.substr(0, 3);
	for (i = 0; i < 12; i++) {
		if (month_string.compare(months[i]) == 0) {
			std::string m = std::to_string(i + 1);
			month.append("0");
			month.append(m);
			if (month.size() > 2) month.erase(0, 1);
		}
	}
	if (compilation_date.substr(4, 1) != " ") day = compilation_date.substr(4, 2);
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
	app_title.append(full_version);
	app_title.append("       Analysis of Jupiter/Saturn videos to find impact flashes");
	/*	full_version.append(" ");
		full_version.append(std::to_string(std::thread::hardware_concurrency()).c_str());
		full_version.append(" processors");*/


		// **********************************************************************
		// **************** ANALYSE  COMMAND LINE PARAMETERS ********************
		// **********************************************************************

	LPWSTR *szArglist;
	int nArgs;
	int idx;

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

	//	opts.message[index_message] = "\0";
	std::for_each(DeTeCtName.begin(), DeTeCtName.end(), [](char & c) {
		c = ::tolower(c);
	});
	BOOL param_instances = FALSE;
	BOOL param_dtcpid = FALSE;
	BOOL param_used = FALSE;
	for (int j = 0; j < commandParametres.GetCount(); j++) {
		if (j > 0) {
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
			param_used = FALSE;
			// Gets number of maxinstances
			//may return 0 when not able to detect
			if (param_instances == TRUE) {
				param_instances = FALSE;
				BOOL has_only_digits = (param.find_first_not_of("0123456789") == std::string::npos);
				int number = atoi(param.c_str());
				if (has_only_digits) number = atoi(param.c_str());
				if ((has_only_digits) && (number > 0)) {
					opts.maxinstances = number;
					param_used = TRUE;
					int processor_count = std::thread::hardware_concurrency();
					if (opts.maxinstances > processor_count) {
						opts.maxinstances = processor_count;
						opts.message[index_message++] = "WARNING : number " + param + " of maxinstances greater than number of processors, using " + std::to_string(processor_count) + " number of processors";
						opts.message[index_message] = "\0";
					} else if (opts.maxinstances < 1) opts.maxinstances = 1;

				}
				else {
					opts.maxinstances = 1;
					opts.message[index_message++] = "WARNING : number " + param + " of maxinstances is not correct, using default value";
					opts.message[index_message] = "\0";
				}
			}
			else if (param_dtcpid == TRUE) {
				param_dtcpid = FALSE;
				BOOL has_only_digits = (param.find_first_not_of("0123456789") == std::string::npos);
				int number = atoi(param.c_str());
				if (has_only_digits) number = atoi(param.c_str());
				if ((has_only_digits) && (number > 0)) {
					param_used = TRUE;
					opts.detect_PID = number;
					opts.message[index_message++] = "Using : PID " + param + " as parent DeTeCt ID";
					opts.message[index_message] = "\0";
				}
				else {
					opts.message[index_message++] = "WARNING : number " + param + " not a valid process ID";
					opts.message[index_message] = "\0";
				}
			}
			// Processes command line options
			if (!param_used) {
//if (starts_with(param, "/restartbyrestartmanager")) return FALSE;
				if (starts_with(param, "-")) {
					DBOUT("option : " << param.c_str() << "\n");
					if (starts_with(param, "-auto")) opts.interactive	= FALSE;											// -automatic processing mode
					else if ((starts_with(param, "-inter")) || (starts_with(param, "-noauto")))  opts.interactive = TRUE;	// -interactive / -noautomatic
					else if (starts_with(param, "-exit")) opts.exit		= TRUE;												// -exit exits when done
					else if (starts_with(param, "-noexit")) opts.exit	= FALSE;												// -exit exits when done
					else if (starts_with(param, "-shut"))	opts.shutdown = TRUE;												// -shutdown mode
					else if (starts_with(param, "-noshut")) opts.shutdown = FALSE;												// -shutdown mode
					else if (starts_with(param, "-noreproc")) {
						opts.reprocessing = FALSE;																			// -noreprocessing
						opts.message[index_message++] = "No reprocessing of files already processed";
						opts.message[index_message] = "\0";
					}
					else if (starts_with(param, "-reproc")) opts.reprocessing = TRUE;										// -reprocessing
					else if (starts_with(param, "-zip")) opts.zip = TRUE;
					else if (starts_with(param, "-nozip")) {
						opts.zip = FALSE;
						opts.message[index_message++] = "No detection zip file will be generated";
						opts.message[index_message] = "\0";
					}
					else if (starts_with(param, "-maxinst")) param_instances = TRUE;										// -maxinstances
					else if (starts_with(param, "-dtcpid")) param_dtcpid = TRUE;											// fix detect parent PID (DEV only)
					else if (starts_with(param, "-asact")) {
						opts.autostakkert = TRUE;																			// simulate AutoStakkert launch (DEV only)

						opts.message[index_message++] = "Launched from AutoStakkert, DO NOT CLOSE THIS WINDOW (close AutoStakkert when done)";
						opts.message[index_message] = "\0";
						opts.interactive = FALSE; // By default (but not mandatory), auto mode on when launched from AutoStakkert
						opts.reprocessing = FALSE; // By default (but not mandatory), noreprocessing on when launched from AutoStakkert
					}
					else if (starts_with(param, "-debug")) {
						opts.debug = TRUE;												// simulate AutoStakkert launch (DEV only)
						opts.message[index_message++] = "DEBUG mode on";
						opts.message[index_message] = "\0";
					}
					else if (starts_with(param, "-nodebug")) opts.debug = FALSE;												// simulate AutoStakkert launch (DEV only)
					else if (starts_with(param, "-help")) {
						opts.message[index_message++] = "Info : syntax : " + DeTeCtName + " [options] filename | foldername";
						opts.message[index_message++] = "   -autoprocessing   automatic mode launching detection without interaction";
						opts.message[index_message++] = "   -noreprocessing   no reprocessing acquisition files already in DeTeCt.log";
						opts.message[index_message++] = "   -nozip                  no generation of impact detection zip file";
						opts.message[index_message++] = "   -maxinstances nbinstances  multi_instances mode running nbinstances to process the files in parallel";
						opts.message[index_message++] = "   -exit                automatic exit when processing done without interaction";
						opts.message[index_message++] = "   -shutdown            automatic shutdown of PC when automatic exit";

						opts.message[index_message++] = "   filename              name of acquisition/autostakkert session file to be processed";
						opts.message[index_message++] = "   foldername          name of folder to be process with all its subfolders";
						opts.message[index_message] = "\0";
					}
					else {
						opts.message[index_message++] = "ERROR : " + param + " option not recognized";
						opts.message[index_message] = "\0";
					}
				}
				// Processes filesystem objets parameters (files or folders)
				else
				if ((param.find(DeTeCtName) == std::string::npos)) {
					//		else if ((param.find(DeTeCtName) == std::string::npos) && !starts_with(param, "/")) {
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
						extension.insert(0, 1, '*');
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

						}
						else {
							opts.message[index_message++] = "WARNING: Extension : " + extension + " not supported";
							opts.message[index_message] = "\0";
							DBOUT("WARNING: Extension : " << extension.c_str() << " not supported\n");
						}
					}
					else {
						DIR *folder_object;
						//opts.message[index_message++] = "file or folder " + object;
						if ((folder_object = opendir(object.c_str()))) {
							//converts relative path to absolute path (does not work from MSVC debug as exe launched from MFC directory)
							if ((starts_with(object, ".")) || !((starts_with(object, "\\")) || (object.find(":\\") == 1))) {
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
		}
	}
	
	// ******************************************************************
	// *********************** QUEUE MANAGEMENT *************************
	// ******************************************************************
	
	CreateQueueFileName();

	// if no additional instance possible, adds object to queue
			// Checks if DeTeCt is already running
	int		DeTeCt_processes_nb = 0;
	if (opts.autostakkert_PID > 0)	DeTeCt_processes_nb = ParentChildrenProcessesNumber(opts.autostakkert_PID);
	else if (opts.detect_PID > 0)	DeTeCt_processes_nb = ParentChildrenProcessesNumber(opts.detect_PID) + 1;		//count parent 
	//else DeTeCt_processes_nb = DetectInstancesNumber();;

	DBOUT("Maximum DeTeCt processes = " << opts.maxinstances << "\n");
	DBOUT("Number of DeTeCt processes = " << DeTeCt_processes_nb << "\n");
	if (DeTeCt_processes_nb > opts.maxinstances) {
		//if ((DeTeCt_processes_nb > opts.maxinstances) && (!opts.interactive)) {
		//log processes to be done and exits
		if (opts.filename) {
			CString objectname(opts.filename);
			CString tmp;
			if (opts.reprocessing || (!IsFileAlreadyQueued(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp)))) PushFileToQueue(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp));
			else DBOUT("file already queued\n");
		}
		else if (opts.dirname) {
			CString objectname(opts.dirname);
			CString tmp;
			if (opts.reprocessing || (!IsFileAlreadyQueued(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp)))) PushFileToQueue(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp));
			else DBOUT("directory already queued\n");
		}
		return FALSE;
	}

	// if new instance and no objects given, looks for work in queue in auto mode
	//if ((!opts.interactive) && (!opts.filename) && (!opts.dirname)) {
	if ((!opts.filename) && (!opts.dirname) && (strlen(opts.DeTeCtQueueFilename) > 1)) {
		CString objectname;
		CString tmp;
		if (GetFileFromQueue(&objectname, (CString)opts.DeTeCtQueueFilename)) {
			std::ifstream file(objectname);
			CT2A tmpo(objectname);
			if (file) {
				opts.filename = new char[strlen(tmpo)+1];
				strcpy(opts.filename, tmpo);
				opts.filename[strlen(tmpo)] = '\0';
				file._close();
			}
			else {
				DIR *folder_object;
				if (folder_object = opendir(tmpo)) {
					opts.dirname = new char[strlen(tmpo) + 1];
					strcpy(opts.dirname, tmpo);
					opts.dirname[strlen(tmpo)] = '\0';
					closedir(folder_object);
				}
				else DBOUT("parameter = " << objectname << " not found\n");
			}
		}
	}
	if (opts.filename) DBOUT("file = " << opts.filename << "\n");
	if (opts.dirname) DBOUT("folder = " << opts.dirname << "\n");

	//clock_t start = clock(); clock_t end = clock();

// **********************************************************
// *********************** MFC INIT *************************
// **********************************************************

/* InitCommonControlsEx() is required on Windows XP if an application
// manifest specifies use of ComCtl32.dll version 6 or later to enable
// visual styles.  Otherwise, any window creation will fail.*/
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
// Set this to include all the common control classes you want to use in your application.
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);

	CWinApp::InitInstance();

/* Standard initialization
// If you are not using these features and wish to reduce the size of your final executable, you should remove from the following
// the specific initialization routines you do not need
// Change the registry key under which our settings are stored
// TODO: You should modify this string to be something appropriate such as the name of your company or organization*/
	SetRegistryKey(_T("DeTeCt settings"));

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

void CreateQueueFileName() {
	
	// Filter instances even if non auto mode (v3.2.2)
	DWORD	parent_id;

	// Create filename for processes queue DeTeCtQueueFilename
	CString log_cstring;
	if (opts.autostakkert_PID > 0) {				// Autostakkert mode
		CString pid_cstring;
		pid_cstring.Format(L"%d", opts.autostakkert_PID);
		CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_as") + pid_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		if (!file_exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
			opts.parent_instance = TRUE;
		}
		else {
			opts.parent_instance = FALSE;
			opts.exit = TRUE;		//automatic exit in child mode
			opts.shutdown = FALSE;	//never shutdown in child mode, should be done only in parent mode!
		}
		opts.interactive = FALSE;	// By default (mandatory), auto mode on when launched from AutoStakkert
		opts.reprocessing = FALSE;	// By default (but not mandatory), noreprocessing on when launched from AutoStakkert
	}
	else if ((IsParentDeTeCt(&parent_id)) || (opts.detect_PID > 0)) {	// DeTeCt mode (child instance)
		//CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		CString parent_id_cstring;
		if (IsParentDeTeCt(&parent_id)) {
			parent_id_cstring.Format(L"%d", parent_id);
			opts.detect_PID = parent_id;
		}
		else {
			parent_id_cstring.Format(L"%d", opts.detect_PID);
		}
		CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_dtc") + parent_id_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		opts.parent_instance = FALSE;

		opts.interactive = FALSE;
		opts.exit = TRUE;		//automatic exit in child mode
		opts.shutdown = FALSE;	//never shutdown in child mode, should be done only in parent mode!
	}
	else if (opts.maxinstances > 1) {				// multi-instances mode, parent instance
		CString pid_cstring;
		pid_cstring.Format(L"%d", GetCurrentProcessId());
		CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_dtc") + pid_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
		opts.parent_instance = TRUE;
	}
	else {											// normal mode, no queue
		strcpy(opts.DeTeCtQueueFilename, "");
// Debug
//CString pid_cstring;
//pid_cstring.Format(L"%d", GetCurrentProcessId());
//CString2char(DeTeCt_additional_filename_exe_fullpath(CString(_T(DTC_QUEUE_PREFIX)) + _T("_dtc") + pid_cstring + _T(DTC_QUEUE_EXT)), opts.DeTeCtQueueFilename);
// Debug
	opts.parent_instance = TRUE;
	}

}