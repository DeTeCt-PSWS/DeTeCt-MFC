
// DeTeCt-MFC.cpp : Defines the class behaviors for the application.
//

#include "stdafx.h"
#include "common2.hpp"
#include "DeTeCt-MFC.hpp"
#include "DeTeCt-MFCDlg.hpp"
#include "dtcas3.hpp"
//#include "ini.h" 
#include "cmdline.h"

#include <iostream>
using namespace std;
#include <fstream>
#include "processes_queue.hpp"

#include "direct.h"

#include <thread>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

std::string full_version;
std::string app_title;
std::string message[2048];

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
	char DeTeCtNameChar[MAX_STRING] = { 0 };
	int index_message = 0;
	srand((unsigned int)time(NULL));

	// ****************************************
	// **************** INIT ******************
	// ****************************************

//init_opts
	opts.filename[MAX_STRING] = { 0 };
	opts.ofilename[MAX_STRING] = { 0 };
	opts.darkfilename[MAX_STRING] = { 0 };
	opts.ovfname[MAX_STRING] = { 0 };
	opts.dirname[MAX_STRING] = { 0 };
	opts.impactdirname[MAX_STRING] = { 0 };
	opts.WarningsFilename[MAX_STRING] = { 0 };
	opts.ErrorsFilename[MAX_STRING] = { 0 };
	opts.zipname[MAX_STRING] = { 0 };
	opts.nsaveframe = 0;	// Frame number to <ofilename>
	opts.ostype = 0;	// Source video type to extract frame
	opts.ovtype = 0;	// Output video type to create

// options?
	opts.timeImpact = 0;				// seconds
	opts.incrLumImpact = 0;				// mean value factor
	opts.incrFrameImpact = 0;				// Minimum number of frames for impact
	opts.impact_duration_min = 0;				// Min duration for impact
	opts.radius = 0;				// Impact radio (pixels)
	opts.nframesROI = 0;				// Number of frames for ROI calculation
	opts.nframesRef = 0;				// Number of frames for ROI calculation
	opts.wROI = 0; 				// ROI width  (CM centered)
	opts.hROI = 0;				// ROI height (CM centered)
	opts.bayer = 0;				//debayering code
	opts.medSize = 0;				// Median buffer size
	opts.facSize = 0; 				// Size factor (ROI)
	opts.secSize = 0; 				// Security factor (ROI)
	opts.ROI_min_px_val = 0; 				// Minimum value of pixel to take into account pixels for ROI calculation
	opts.ROI_min_size = 0; 				// Minimum valid pixel size for a ROI 
	opts.threshold = 0;
	opts.learningRate = 0;				// "Alpha Blending" learning rate
	opts.histScale = 0;				// Histogram scale
	opts.wait = 0;				// milliseconds
	opts.thrWithMask = 0;				// Use Mask (!=0) or not (0) for frame reference
	opts.viewROI = FALSE; 			// View ROI
	opts.viewTrk = FALSE; 			// View planet tracking
	opts.viewDif = FALSE; 			// View differential frame
	opts.viewRef = FALSE; 			// View reference frame
	opts.viewMsk = FALSE; 			// View mask
	opts.viewThr = FALSE; 			// View threshold
	opts.viewSmo = FALSE;			// View frame after filter application
	opts.viewHis = FALSE;			// View differential frame histogram
	opts.viewRes = FALSE;			// View final frame
	opts.verbose = FALSE;
	opts.debug = FALSE;			// debug mode with more information
	opts.videotest = FALSE;			// Test input video file
	opts.ADUdtconly = FALSE;			// Use ADUdtc algorithm only
	opts.detail = FALSE;			// Use ADUdtc algorithm only with 2 more images as output
	opts.zip = TRUE;				// Creates zip of impact_detection folder at the end of processing
	opts.email = TRUE;				// Send email at the end of processing
	opts.allframes = FALSE;			// Save all intermediate mac frames from ADUdtc algorithm
	opts.impact_distance_max = 0;				// Maximum value for distance between old algorithm and max in detection image for being a possible impact
	opts.impact_max_avg_min = 0;				// Minimum value for max - mean value of dtc_max-mean image for being a possible impact
	opts.impact_confidence_min = 0;				// Minimum value for confidence for being a possible impact
	opts.minframes = 0;				// Minimum # of frames to start processing
	opts.filter = { 0, {0,0,0,0} };
	opts.dateonly = FALSE;			// Display date information and stops processing
	opts.ignore = TRUE;			// Ignore incorrect frames
	opts.maxinstances = 1;				// Maximum number of DeTeCt instances running in parallel
	opts.reprocessing = FALSE;			// Reprocessing files already in DeTeCt.log
	opts.interactive = FALSE;			// Normal interactive mode or automatic mode
	opts.autoexit = FALSE;			// Automatic exit when processing done
	opts.shutdown = FALSE;			// Automatic PC shutdownn when auto exit
	opts.flat_preparation = FALSE;			// Flag to create flat
	opts.clean_dir = FALSE;			// Cleans directory before processing
	opts.show_detect_image = TRUE;				// show detection image
	opts.show_mean_image = FALSE;			// show mean image
	opts.bg_detection_peak_factor = 0;			// for min threshold to detect background
	opts.bg_detection_consecutive_values = 0;	// # of consecutive frames to be below peak factor for background detection
	opts.transparency_min_pc = 20;					// tolerance in transparency for a frame compared to 1st frame
	opts.similarity_decrease_max_pc = 12;			// max decrease between two frames similarity


// Status
	opts.interactive_bak = FALSE;			// Backup of interactive status
	opts.autostakkert = FALSE;			// Launched from autostakkert
	opts.autostakkert_PID = 0;				// Parent autostakkert PID
	opts.detect_PID = 0;				// Parent detect PID
	opts.version[MAX_STRING] = { 0 };
	opts.DeTeCtQueueFilename[MAX_STRING] = { 0 };
	opts.LogConsolidatedDirname[MAX_STRING] = { 0 };
	opts.parent_instance = FALSE;

	DeTeCtFileName(DeTeCtNameChar);
	std::string DeTeCtName(DeTeCtNameChar); // "DeTeCt.exe"

	// Sets default values for options then reads options from ini file (before it could be changed by command line)
	//opts.interactive = TRUE; // -interactive / -noautomatic
	CDeTeCtMFCDlg::CDeTeCtMFCDlg(NULL);
	opts.flat_preparation = FALSE;
	opts.interactive_bak = opts.interactive;

	opts.autostakkert = FALSE;
	opts.autostakkert_PID = 0;
	opts.detect_PID = 0;
	opts.parent_instance = FALSE;
	opts.interactive_bak = opts.interactive;

	// Checks if DeTeCt has been launched by AutoStakkert
	if (IsParentAutostakkert(&opts.autostakkert_PID)) {
		message_lines[index_message++] = "Launched from AutoStakkert, DO NOT CLOSE THIS WINDOW (close AutoStakkert when done)";
		message_lines[index_message] = "\0";
		opts.autostakkert = TRUE;
		opts.parent_instance = FALSE;
	}
	else {
		// in case code for displaying ProcessName is needed
		DWORD pid, ppid;
		int e;
		wchar_t wfname[MAX_PATH] = { 0 };
		char fname[MAX_PATH] = { 0 };
		pid = GetCurrentProcessId();
		e = getProcessName(pid, wfname, MAX_PATH);
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

	LPWSTR* szArglist;
	int nArgs;
	int idx;

	szArglist = CommandLineToArgvW(GetCommandLineW(), &nArgs);
	if (NULL == szArglist)
	{
		wprintf(L"CommandLineToArgvW failed\n");
	}
	else for (idx = 0; idx < nArgs; idx++) {
		commandParametres.Add(szArglist[idx]);
	}

	// Free memory allocated for CommandLineToArgvW arguments.
	LocalFree(szArglist);

	lowercase_string(&DeTeCtName);
	BOOL param_instances = FALSE;
	BOOL param_dtcpid = FALSE;
	BOOL param_aspid = FALSE;
	BOOL param_used = FALSE;
	for (int j = 0; j < commandParametres.GetCount(); j++) {
		if (j > 0) {
			CString parameter = commandParametres.GetAt(j);
			/*CT2CA parameter_ansi_string(parameter);
			std::string param(parameter_ansi_string);
			std::string param_org(parameter_ansi_string);*/
			std::wstring wparam(parameter);
			//std::string param(wparam.begin(), wparam.end());
			std::string param = wstring2string(wparam);
			//std::string param_org(wparam.begin(), wparam.end());
			std::string param_org = wstring2string(wparam);

			lowercase_string(&param);
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
						message_lines[index_message++] = "WARNING : number " + param + " of maxinstances greater than number of processors, using " + std::to_string(processor_count) + " number of processors";
						message_lines[index_message] = "\0";
					}
					else if (opts.maxinstances < 1) opts.maxinstances = 1;

				}
				else {
					opts.maxinstances = 1;
					message_lines[index_message++] = "WARNING : number " + param + " of maxinstances is not correct, using default value";
					message_lines[index_message] = "\0";
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
					message_lines[index_message++] = "Using : PID " + param + " as parent DeTeCt ID";
					message_lines[index_message] = "\0";
				}
				else {
					message_lines[index_message++] = "WARNING : number " + param + " not a valid process ID";
					message_lines[index_message] = "\0";
				}
			} else if (param_aspid == TRUE) {
					param_aspid = FALSE;
					BOOL has_only_digits = (param.find_first_not_of("0123456789") == std::string::npos);
					int number = atoi(param.c_str());
					if (has_only_digits) number = atoi(param.c_str());
					if ((has_only_digits) && (number > 0)) {
						param_used = TRUE;
						opts.autostakkert_PID = number;
						message_lines[index_message++] = "Using : PID " + param + " as parent Autostakkert ID";
						message_lines[index_message] = "\0";
					}
					else {
						message_lines[index_message++] = "WARNING : number " + param + " not a valid process ID";
						message_lines[index_message] = "\0";
					}
				}
			// Processes command line options
			if (!param_used) {
				//if (starts_with(param, "/restartbyrestartmanager")) return FALSE;
				if (starts_with(param, "-")) {
					DBOUT("option : " << param.c_str() << "\n");
					if (starts_with(param, "-auto")) opts.interactive = FALSE;												// -automatic processing mode
					else if ((starts_with(param, "-inter")) || (starts_with(param, "-noauto")))  opts.interactive = TRUE;	// -interactive / -noautomatic
					else if (starts_with(param, "-exit")) opts.autoexit = TRUE;													// -exit exits when done
					else if (starts_with(param, "-noexit")) opts.autoexit = FALSE;												// -exit exits when done
					else if (starts_with(param, "-shut"))	opts.shutdown = TRUE;											// -shutdown mode
					else if (starts_with(param, "-noshut")) opts.shutdown = FALSE;											// -noshutdown mode
					else if (starts_with(param, "-noreproc")) {
						opts.reprocessing = FALSE;																			// -noreprocessing
						message_lines[index_message++] = "No reprocessing of files already processed";
						message_lines[index_message] = "\0";
					}
					else if (starts_with(param, "-reproc")) opts.reprocessing = TRUE;										// -reprocessing
					else if (starts_with(param, "-zip")) opts.zip = TRUE;
					else if (starts_with(param, "-nozip")) {
						opts.zip = FALSE;
						message_lines[index_message++] = "No detection zip file will be generated";
						message_lines[index_message] = "\0";
					}
					else if (starts_with(param, "-maxinst")) param_instances = TRUE;										// -maxinstances
					else if (starts_with(param, "-dtcpid")) param_dtcpid = TRUE;											// fix detect parent PID (DEV only)
					else if (starts_with(param, "-aspid")) param_aspid = TRUE;												// fix detect parent PID (DEV only)
					else if (starts_with(param, "-flatprep")) opts.flat_preparation = TRUE;									// generate flat preparation image (max of all frames not aligned) (DEV only)
					else if (starts_with(param, "-asact")) {
						opts.autostakkert = TRUE;																			// simulate AutoStakkert launch (DEV only)

						message_lines[index_message++] = "Launched from AutoStakkert, DO NOT CLOSE THIS WINDOW (close AutoStakkert when done)";
						message_lines[index_message] = "\0";
						opts.interactive = FALSE; // By default (but not mandatory), auto mode on when launched from AutoStakkert
						//opts.reprocessing = FALSE; // By default (but not mandatory), noreprocessing on when launched from AutoStakkert - implemented in function using reprocessing
					}
					else if (starts_with(param, "-debug")) {
						opts.debug = TRUE;																					// debug info mode (DEV only)
						message_lines[index_message++] = "DEBUG mode on";
						message_lines[index_message] = "\0";
					}
					else if (starts_with(param, "-nodebug")) opts.debug = FALSE;											// debug info mode (DEV only)
					else if (starts_with(param, "-help")) {
						message_lines[index_message++] = "Info : syntax : " + DeTeCtName + " [options] filename | foldername";
						message_lines[index_message++] = "   -autoprocessing   automatic mode launching detection without interaction";
						message_lines[index_message++] = "   -noreprocessing   no reprocessing acquisition files already in DeTeCt.log";
						message_lines[index_message++] = "   -nozip                  no generation of impact detection zip file";
						message_lines[index_message++] = "   -maxinstances nbinstances  multi_instances mode running nbinstances to process the files in parallel";
						message_lines[index_message++] = "   -exit                automatic exit when processing done without interaction";
						message_lines[index_message++] = "   -shutdown            automatic shutdown of PC when automatic exit";

						message_lines[index_message++] = "   filename              name of acquisition/autostakkert session file to be processed";
						message_lines[index_message++] = "   foldername          name of folder to be process with all its subfolders";
						message_lines[index_message] = "\0";
					}
					else {
						message_lines[index_message++] = "ERROR : " + param + " option not recognized";
						message_lines[index_message] = "\0";
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
							//std::string filter_string(filter_wstring.begin(), filter_wstring.end());
							std::string filter_string = wstring2string(filter_wstring);
							filter_string = filter_string.substr(filter_string.find_first_of('|') + 1, filter_string.size() - filter_string.find_first_of('|') - 3);
							filter_string.push_back(';');
							extension.insert(0, 1, '*');
							extension.push_back(';');
							if (filter_string.rfind(extension) != std::string::npos) {
								if ((starts_with(object, ".")) || !((starts_with(object, "\\")) || (object.find(":\\") == 1))) {
									//CT2CA tmp_conv(DeTeCt_exe_folder());
									//target_folder = std::string (tmp_conv);
									char buffer[MAX_STRING] = { 0 };
									if (_fullpath(buffer, object.c_str(), MAX_STRING) == NULL) {
										 char msgtext[MAX_STRING] = { 0 };							
										snprintf(msgtext, MAX_STRING, "cannot construct full path %s", object.c_str());
										ErrorExit(TRUE, "cannot construct full path", __func__, msgtext);
									};
									target_file = std::string(buffer);
								}
								else target_file = object;

								//opts.filename = new char[target_file.size() + 1]; // exception read access
								std::copy(target_file.begin(), target_file.end(), opts.filename);
								opts.filename[target_file.size()] = '\0';
								DBOUT("file = " << opts.filename << "\n");
								std::string tmp_string(opts.filename);
								message_lines[index_message++] = "Using file " + tmp_string;
								message_lines[index_message] = "\0";
							}
							else {
								message_lines[index_message++] = "WARNING: Extension : " + extension + " not supported";
								message_lines[index_message] = "\0";
								DBOUT("WARNING: Extension : " << extension.c_str() << " not supported\n");
							}
						}
						else {
							DIR* folder_object;
							//message_lines[index_message++] = "file or folder " + object;
							if ((folder_object = opendir(object.c_str()))) {
								//converts relative path to absolute path (does not work from MSVC debug as exe launched from MFC directory)
								if ((starts_with(object, ".")) || !((starts_with(object, "\\")) || (object.find(":\\") == 1))) {
									//CT2CA tmp_conv(DeTeCt_exe_folder());
									//target_folder = std::string (tmp_conv);
									char buffer[MAX_STRING] = { 0 };
									if (_fullpath(buffer, object.c_str(), MAX_STRING) == NULL) {
										 char msgtext[MAX_STRING] = { 0 };
										snprintf(msgtext, MAX_STRING, "cannot construct full path %s", object.c_str());
										ErrorExit(TRUE, "cannot construct full path", __func__, msgtext);
									};
									target_folder = std::string(buffer);
								}
								else target_folder = object;

								//opts.dirname = new char[target_folder.size() + 1]; // exception read access
								std::copy(target_folder.begin(), target_folder.end(), opts.dirname);
								opts.dirname[target_folder.size()] = '\0';
								DBOUT("folder = " << target_folder.c_str() << "\n");
								std::string tmp_string(opts.dirname);
								message_lines[index_message++] = "Using directory " + tmp_string;
								message_lines[index_message] = "\0";
							}
							else {
								DBOUT("WARNING: Object : " << object.c_str() << " not found\n");
								message_lines[index_message++] = "ERROR : file or folder " + object + " not found";
								message_lines[index_message] = "\0";

							}
							closedir(folder_object);
						}
						file.close();
					}
			}
		}
	}

	// ******************************************************************
	// *********************** QUEUE MANAGEMENT *************************
	// ******************************************************************
	if (opts.debug) {
		message_lines[index_message++] = "!Debug info: Exit=" + std::to_string(opts.autoexit);
		message_lines[index_message] = "\0";
	}

	CreateQueueFileName();
	if ((opts.autostakkert) && (opts.parent_instance)) app_title.append("       (from AutoStakkert!)");
	if (opts.debug) {
		std::string msg(opts.DeTeCtQueueFilename);
		message_lines[index_message++] = msg + "!Debug info: AS PID=" + std::to_string(opts.autostakkert) + " " + std::to_string(opts.autostakkert_PID);
		message_lines[index_message++] = "!Debug info: Exit=" + std::to_string(opts.autoexit);
		message_lines[index_message] = "\0";
	}

	// if no additional instance possible, adds object to queue
			// Checks if DeTeCt is already running
	int		DeTeCt_processes_nb = 0;
	if (opts.autostakkert_PID > 0)	DeTeCt_processes_nb = ParentChildrenProcessesNumber(opts.autostakkert_PID);
	else if (opts.detect_PID > 0)	DeTeCt_processes_nb = ParentChildrenProcessesNumber(opts.detect_PID) + 1;		//count parent 
	//else DeTeCt_processes_nb = DetectInstancesNumber();

	DBOUT("Maximum DeTeCt processes = " << opts.maxinstances << "\n");
	DBOUT("Number of DeTeCt processes = " << DeTeCt_processes_nb << "\n");
	if ((DeTeCt_processes_nb > opts.maxinstances) || ((opts.autostakkert) && (!opts.parent_instance))) {
		//if ((DeTeCt_processes_nb > opts.maxinstances) && (!opts.interactive)) {
		//log processes to be done and exits
		if (strlen(opts.filename) > 0) {
if (opts.debug) {
	message_lines[index_message++] = "!Debug info: Queuing filename " + std::string (opts.filename);
	message_lines[index_message] = "\0";
}
			CString objectname(opts.filename);
			CString tmp;
			if (opts.reprocessing || (!IsFileAlreadyQueued(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp)))) {
				std::wstringstream ss;
				std::string file;
				std::string filename_acquisition;
				int nframe = -1;
				PIPPInfo pipp_info;

				file = std::string(opts.filename);
				if ((Is_Capture_OK_from_File(file, &filename_acquisition, &nframe, &ss)) &&
					// ********* Error if acquisition has not enough frames
					(Is_Capture_Long_Enough(file, nframe, &ss)) &&
					// ********* Ignores if required dark, pipp, winjupos derotated files
					(!Is_Capture_Special_Type(file, &ss)) &&
					// ********* Ignores PIPP with no integrity
					(!Is_PIPP(file) || ((Is_PIPP(file) && Is_PIPP_OK(file, &pipp_info, &ss))))) {
					std::string folder_path;
					if (!opts.autostakkert) folder_path = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\"));
					else {
						//log directory when autostakkert mode or multi instance mode
						folder_path = CString2string(DeTeCt_exe_folder());
					}
					//std::string folder_path = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\")); 
					CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder((CString)folder_path.c_str(), DTC_LOG_SUFFIX));
					std::string log_file(DeTeCtLogFilename);
					if (Is_CaptureFile_To_Be_Processed(filename_acquisition, log_file, &ss)) {
						PushFileToQueue(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp));
					}
				}
			}
			else DBOUT("file already queued\n");
		}
		else if (strlen(opts.dirname) > 0) {
if (opts.debug) {
	message_lines[index_message++] = "!Debug info: Queuing directory " + std::string(opts.dirname);
	message_lines[index_message] = "\0";
}
			CString objectname(opts.dirname);
			CString tmp;
			//if (opts.reprocessing || (!IsFileAlreadyQueued(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp)))) PushFileToQueue(objectname, char2CString(opts.DeTeCtQueueFilename, &tmp));
			//else DBOUT("directory already queued\n");
		}
		return FALSE;
	}

	// if new instance and no objects given, looks for work in queue in auto mode
	//if ((!opts.interactive) && (!opts.filename) && (!opts.dirname)) {
	if ((strlen(opts.filename) == 0) && (strlen(opts.dirname) == 0) && (strlen(opts.DeTeCtQueueFilename) > 1)) {
		CString objectname;
		CString tmp;
if (opts.debug) {
	message_lines[index_message++] = "Getting file from queue";
	message_lines[index_message] = "\0";
}
		if (GetFileFromQueue(&objectname, (CString)opts.DeTeCtQueueFilename)) {
if (opts.debug) {
	CT2A char_objectname(objectname);
	message_lines[index_message++] = "Fetched file from queue" + std::string(char_objectname);
	message_lines[index_message] = "\0";
}
			std::ifstream file(objectname);
			CT2A tmpo(objectname);
			if (file) {
				//opts.filename = new char[strlen(tmpo)+1];  // exception read access
				strcpy(opts.filename, tmpo);
				opts.filename[strlen(tmpo)] = '\0';
				message_lines[index_message++] = "Using file " + std::string(tmpo);
				message_lines[index_message] = "\0";
			}
			else {
				DIR *folder_object;
				if (folder_object = opendir(tmpo)) {
					//opts.dirname = new char[strlen(tmpo) + 1];  // exception read access
					strcpy(opts.dirname, tmpo);
					opts.dirname[strlen(tmpo)] = '\0';
					closedir(folder_object);
					message_lines[index_message++] = "Using directory " + std::string(tmpo);
					message_lines[index_message] = "\0";
				}
				else DBOUT("parameter = " << objectname << " not found\n");
			}
			file.close();
		}
		else if (opts.autostakkert_PID > 0) return FALSE; // No file for autostakkert child
		else { // No file for child
			if (opts.debug) {
				message_lines[index_message++] = "!Degug info : Can't fetch file ...";
				message_lines[index_message] = "\0";
				//if (!opts.parent_instance) return FALSE;
			}
			if (!opts.parent_instance) return FALSE;
		}
	}
	if (strlen(opts.filename) > 0) DBOUT("file = " << opts.filename << "\n");
	if (strlen(opts.dirname) > 0) DBOUT("folder = " << opts.dirname << "\n");

	//clock_t start = clock(); clock_t end = clock();
	if (opts.debug) {
		std::string msg(opts.DeTeCtQueueFilename);
		message_lines[index_message++] = msg + "!Debug info: AS PID=" + std::to_string(opts.autostakkert) + " " + std::to_string(opts.autostakkert_PID);
		message_lines[index_message++] = "!Debug info: Exit=" + std::to_string(opts.autoexit);
		message_lines[index_message] = "\0";
	}

// **********************************************************
// *********************** TEST *************************
// **********************************************************

	//DBOUT("DBOUT test " << "\n");	// works
	//fprintf(stderr, "stderr test\n"); // does not work
	//fprintf(stdout, "stdout test\n"); // does not work
	//Warning(WARNING_MESSAGE_BOX, "Warning test", __func__, "Warning display test"); // works

// **********************************************************
// *********************** MFC INIT *************************
// **********************************************************

/* InitCommonControlsEx() is required on Windows XP if an application
// manifest specifies use of ComCtl32.dll version 6 or later to enable
// visual styles.  Otherwise, any window creation will fail.*/
	INITCOMMONCONTROLSEX InitCtrls = {};
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
	
	INT_PTR nResponse = dlg.DoModal(); //Launches windows

	//Window closed
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
		if (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
			opts.parent_instance = TRUE;
		}
		else {
			opts.parent_instance = FALSE;
			opts.autoexit = TRUE;		//automatic exit in child mode
			opts.shutdown = FALSE;	//never shutdown in child mode, should be done only in parent mode!
		}
		opts.interactive = FALSE;	// By default (mandatory), auto mode on when launched from AutoStakkert
		//opts.reprocessing = FALSE;	// By default (but not mandatory), noreprocessing on when launched from AutoStakkert //implemented in function using reprocessing
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
		opts.autoexit = TRUE;		//automatic exit in child mode
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
		opts.parent_instance = TRUE;
	}
}