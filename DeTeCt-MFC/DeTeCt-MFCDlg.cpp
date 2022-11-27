// DeTeCt-MFCDlg.cpp : implementation file
//

#include "stdafx.h"

#include "AutoUpdate.h"

#pragma warning(push)
#pragma warning(disable: 4244)
/* C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\VC\Tools\MSVC\14.27.29110\include\xstring(2381): message : voir la référence à l'instanciation de la fonction modèle 'void std::basic_string<char,std::char_traits<char>,std::allocator<char>>::_Construct<wchar_t*>(const _Iter,const _Iter,std::forward_iterator_tag)' en cours de compilation
with
[
 _Iter = wchar_t*
]*/
#include "DeTeCt-MFC.h"
#pragma warning(pop)
#include "DeTeCt-MFCDlg.h"
#include "afxdialogex.h"

#include "dtcgui.hpp"
#include "DetectThread.h"
#include "cmdline.h"
#include <thread>
#include <string>

#include "common.h"
#include "common2.h"
#include "processes_queue.hpp"

#include "wrapper2.h"

//#include <opencv2/imgproc.hpp> //TEST opencv3
#include <iomanip>  // std::setprecision

#define FFMPEGDLL "opencv_ffmpeg2413_64.dll"

std::string message_lines[MAX_STRING];

extern char impact_detection_dirname[MAX_STRING];
extern char zip_detection_location[MAX_STRING];
extern char zipfile[MAX_STRING];
extern char log_detection_dirname[MAX_STRING];
extern char email_subject_probabilities[MAX_STRING];
extern char email_body_probabilities[MAX_STRING];


#ifdef _DEBUG

/**************************************************************************************************
 * @def	new
 *
 * @brief	A macro that defines new.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

#define new DEBUG_NEW
#endif

CListBox		CDeTeCtMFCDlg::impactDetectionLog;
CProgressCtrl	CDeTeCtMFCDlg::progressBar;
CProgressCtrl	CDeTeCtMFCDlg::progressBar_all;
CStatic			CDeTeCtMFCDlg::impactNull;
CStatic			CDeTeCtMFCDlg::impactLow;
CStatic			CDeTeCtMFCDlg::impactHigh;
CStatic			CDeTeCtMFCDlg::probability;
CStatic			CDeTeCtMFCDlg::duration;
CStatic			CDeTeCtMFCDlg::totalProgress;
CStatic			CDeTeCtMFCDlg::fileName;
CStatic			CDeTeCtMFCDlg::computingTime;
CMFCLinkCtrl 	CDeTeCtMFCDlg::detectImageslink;
CMFCLinkCtrl 	CDeTeCtMFCDlg::zipFilelink;
CMFCLinkCtrl 	CDeTeCtMFCDlg::detectLoglink;
CButton			CDeTeCtMFCDlg::Auto;
CButton			CDeTeCtMFCDlg::Exit;
CButton			CDeTeCtMFCDlg::Shutdown;
CButton			CDeTeCtMFCDlg::AS;
CButton			CDeTeCtMFCDlg::dark;
CButton			CDeTeCtMFCDlg::acquisitionLog;
CButton			CDeTeCtMFCDlg::SER;
CButton			CDeTeCtMFCDlg::SERtimestamps;
CButton			CDeTeCtMFCDlg::FITS;
CButton			CDeTeCtMFCDlg::FileInfo;
CStatic			CDeTeCtMFCDlg::acquisitionSW;
CButton			CDeTeCtMFCDlg::execAS;
CStatic			CDeTeCtMFCDlg::Instance;
CStatic			CDeTeCtMFCDlg::MaxInstances;

CListBox	SendEmailDlg::outputLog;
std::vector<LPCTSTR> SendEmailDlg::logMessages;

CStatic c_Frame;

//******************************************************************************************************************************************************************************************************************************************************************************************
// CAboutDlg dialog used for App About
//******************************************************************************************************************************************************************************************************************************************************************************************

/**************************************************************************************************
 * @class	CAboutDlg
 *
 * @brief	Dialog for setting the about.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME

	/**************************************************************************************************
	 * @enum	
	 *
	 * @brief	Values that represent s.
	 **************************************************************************************************/

	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

// Implementation
protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnUpdateExitQuit(CCmdUI *pCmdUI);
	afx_msg void OnBnClickedMfclink1();
	afx_msg void OnBnClickedMfclink2();

};

/**************************************************************************************************
 * @fn	CAboutDlg::CAboutDlg()
 *
 * @brief	Default constructor.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

CAboutDlg::CAboutDlg() : CDialog(IDD_ABOUTBOX)
{
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_UPDATE_COMMAND_UI(ID_EXIT_QUIT,	&CAboutDlg::OnUpdateExitQuit)
	ON_BN_CLICKED(IDC_MFCLINK1,			&CAboutDlg::OnBnClickedMfclink1)
	ON_BN_CLICKED(IDC_MFCLINK2,			&CAboutDlg::OnBnClickedMfclink2)
END_MESSAGE_MAP()

/**************************************************************************************************
 * @fn	void CAboutDlg::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

/**************************************************************************************************
 * @fn	void CAboutDlg::OnUpdateExitQuit(CCmdUI *pCmdUI)
 *
 * @brief	Updates the user interface for the exit quit action.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pCmdUI	If non-null, the command user interface.
 **************************************************************************************************/

void CAboutDlg::OnUpdateExitQuit(CCmdUI *pCmdUI)
{
	pCmdUI; // warning disabling
	this->CloseWindow();
}

/**************************************************************************************************
 * @fn	void CAboutDlg::OnBnClickedMfclink1()
 *
 * @brief	Executes the action performed after the DeTeCt project link is clicked.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CAboutDlg::OnBnClickedMfclink1()
{
	ShellExecute(NULL, L"open", L"http://www.astrosurf.com/planetessaf/doc/project_detect.shtml",	NULL, NULL, SW_SHOWNORMAL);
}

/**************************************************************************************************
 * @fn	void CAboutDlg::OnBnClickedMfclink2()
 *
 * @brief	Executes the action performed after the jovian impacts link is clicked.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CAboutDlg::OnBnClickedMfclink2()
{
	ShellExecute(
		NULL,
		L"open",
		L"http://pvol2.ehu.eus/psws/jovian_impacts/",
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}


//******************************************************************************************************************************************************************************************************************************************************************************************
// CDeTeCtMFCDlg dialog
//******************************************************************************************************************************************************************************************************************************************************************************************

/**************************************************************************************************
 * @fn	CDeTeCtMFCDlg::CDeTeCtMFCDlg(CWnd* pParent )
 *
 * @brief	Constructor. -- Initialises the settings
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pParent	If non-null, the parent.
 **************************************************************************************************/

CDeTeCtMFCDlg::CDeTeCtMFCDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_DETECTMFC_DIALOG, pParent)
{
	CString DeTeCtIniFilename = DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX);
	_TCHAR optionStr[MAX_STRING] = { 0 };
	init_string(opts.filename); // exception read access
	init_string(opts.ofilename); // exception read access
	init_string(opts.ovfname); // exception read access

	::GetPrivateProfileString(L"general", L"version", L"0.0.0.0", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	strcpy(opts.version, CT2A(optionStr));

	opts.nsaveframe = 0;
	opts.ostype = OTYPE_NO;
	opts.ovtype = OTYPE_NO;
						::GetPrivateProfileString(L"impact",L"min_strength",			L"0.3", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.timeImpact = std::stod(optionStr);
	opts.incrLumImpact = std::stod(optionStr);
	opts.incrFrameImpact=::GetPrivateProfileInt(L"impact",	L"frames",					5, DeTeCtIniFilename);
						::GetPrivateProfileString(L"impact",L"impact_duration_min",		L"0.4", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.impact_duration_min = std::stod(optionStr);
	opts.radius =		::GetPrivateProfileInt(L"impact",	L"radius",					10, DeTeCtIniFilename);
	opts.nframesROI =	15;
	opts.nframesRef =	::GetPrivateProfileInt(L"other",	L"refmin",					50, DeTeCtIniFilename);
	opts.bayer =		::GetPrivateProfileInt(L"other",	L"debayer",					0, DeTeCtIniFilename);
	opts.medSize =		::GetPrivateProfileInt(L"roi",		L"medbuf",					5, DeTeCtIniFilename);
	opts.ROI_min_px_val=::GetPrivateProfileInt(L"roi",		L"ROI_min_px_val",			10, DeTeCtIniFilename);
	opts.ROI_min_size = ::GetPrivateProfileInt(L"roi",		L"ROI_min_size",			70, DeTeCtIniFilename);
	opts.wait = 1;
						::GetPrivateProfileString(L"roi",	L"sizfac",					L"0.90", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.facSize = std::stod(optionStr);
						::GetPrivateProfileString(L"roi",	L"secfac",					L"1.05", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.secSize = std::stod(optionStr);
	opts.threshold =	::GetPrivateProfileInt(L"impact",	L"thresh",					0, DeTeCtIniFilename);
	opts.learningRate = 0.0;
	opts.thrWithMask =	::GetPrivateProfileInt(L"impact",	L"mask",					0, DeTeCtIniFilename);
						::GetPrivateProfileString(L"impact",L"impact_distance_max",	L"0.03", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.impact_distance_max = std::stod(optionStr);
						::GetPrivateProfileString(L"impact",L"impact_max_avg_min",		L"177.0", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.impact_max_avg_min = std::stod(optionStr);
						::GetPrivateProfileString(L"impact",L"impact_confidence_min",	L"2.10", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	opts.impact_confidence_min = std::stod(optionStr);
	opts.histScale = 1;
	opts.show_detect_image =	::GetPrivateProfileInt(L"view",		L"detect",					TRUE, DeTeCtIniFilename);
	opts.show_mean_image =		::GetPrivateProfileInt(L"view",		L"mean",					FALSE, DeTeCtIniFilename);
	opts.viewROI =				::GetPrivateProfileInt(L"view",		L"roi",						FALSE, DeTeCtIniFilename);
	opts.viewTrk =				::GetPrivateProfileInt(L"view",		L"trk",						FALSE, DeTeCtIniFilename);
	opts.viewDif =				::GetPrivateProfileInt(L"view",		L"dif",						FALSE, DeTeCtIniFilename);
	opts.viewRef =				::GetPrivateProfileInt(L"view",		L"ref",						FALSE, DeTeCtIniFilename);
	opts.viewThr =				::GetPrivateProfileInt(L"view",		L"thr",						FALSE, DeTeCtIniFilename);
	opts.viewSmo =				::GetPrivateProfileInt(L"view",		L"smo",						FALSE, DeTeCtIniFilename);
	opts.viewRes =				::GetPrivateProfileInt(L"view",		L"res",						FALSE, DeTeCtIniFilename);
	opts.viewHis =				::GetPrivateProfileInt(L"view",		L"his",						FALSE, DeTeCtIniFilename);
	opts.viewMsk =				::GetPrivateProfileInt(L"view",		L"msk",						FALSE, DeTeCtIniFilename);
	opts.verbose = 0;
	opts.filter.type = ::GetPrivateProfileInt(L"other",		L"filter",					1, DeTeCtIniFilename);
	opts.filter.param[0] =	3;
	opts.filter.param[1] =	3;
	opts.filter.param[2] =	0;
	opts.filter.param[3] =	0;
	opts.ADUdtconly =		FALSE;
	opts.detail =		::GetPrivateProfileInt(L"impact",		L"detail",				FALSE, DeTeCtIniFilename);
	opts.allframes =	::GetPrivateProfileInt(L"impact",		L"inter",				FALSE, DeTeCtIniFilename);

	opts.minframes =	::GetPrivateProfileInt(L"other",		L"frmin",				15, DeTeCtIniFilename);
	opts.ignore =		::GetPrivateProfileInt(L"other",		L"ignore",				FALSE, DeTeCtIniFilename);
						::GetPrivateProfileString(L"other",		L"darkfile",			L"", optionStr, sizeof(optionStr) / sizeof(optionStr[0]), DeTeCtIniFilename);
	strcpy(opts.darkfilename, CT2A(optionStr));
	opts.videotest = 0;
	opts.wROI = 0;
	opts.hROI = 0;

	opts.debug = FALSE;
	//opts.debug =		::GetPrivateProfileInt(L"processing",	L"debug",				FALSE, DeTeCtIniFilename);
	debug_mode = opts.debug;
	opts.dateonly = FALSE;
	//opts.dateonly =		::GetPrivateProfileInt(L"processing",	L"dateonly",			FALSE, DeTeCtIniFilename);
	opts.zip =			::GetPrivateProfileInt(L"processing",	L"zip",					TRUE, DeTeCtIniFilename);
	opts.email =		::GetPrivateProfileInt(L"processing",	L"email",				TRUE, DeTeCtIniFilename);
	// From main window checkboxes
	opts.interactive =	!::GetPrivateProfileInt(L"processing",	L"autoprocessing",		FALSE, DeTeCtIniFilename);
	opts.autoexit =			::GetPrivateProfileInt(L"processing",	L"autoexit",			FALSE, DeTeCtIniFilename);
	opts.shutdown =		::GetPrivateProfileInt(L"processing",	L"autoshutdown",		FALSE, DeTeCtIniFilename);
	opts.maxinstances =	::GetPrivateProfileInt(L"processing",	L"maxinstances",		1, DeTeCtIniFilename);
	int processor_count = std::thread::hardware_concurrency();
	if (opts.maxinstances > processor_count) opts.maxinstances = processor_count;
	else if (opts.maxinstances < 1) opts.maxinstances = 1;
	opts.reprocessing = ::GetPrivateProfileInt(L"processing",	L"reprocessing",		TRUE, DeTeCtIniFilename);

	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//AFX_MANAGE_STATE(AFX_MODULE_STATE* pModuleState);
	/*CWinApp* pwinapp;
	pwinapp = AfxGetApp();
	m_hIcon = pwinapp->LoadIcon(IDR_MAINFRAME);*/	
	//m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}


/*CDeTeCtMFCDlg::~CDeTeCtMFCDlg() {
	CDeTeCtMFCDlg::OnFileExit();
}*/

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void CDeTeCtMFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST1, impactDetectionLog);
	DDX_Control(pDX, IDC_PROGRESS1, progressBar);
	DDX_Control(pDX, IDC_PROGRESS2, progressBar_all);
	DDX_Control(pDX, IDC_STATICPROBA, probability);
	DDX_Control(pDX, IDC_IMPACTNULL, impactNull);
	DDX_Control(pDX, IDC_IMPACTLOW, impactLow);
	DDX_Control(pDX, IDC_IMPACTHIGH, impactHigh);
	DDX_Control(pDX, IDC_STATIC_TOTALPROGRESS, totalProgress);
	DDX_Control(pDX, IDC_STATIC_FILENAME, fileName);
	DDX_Control(pDX, IDC_STATIC_COMPUTING, computingTime);
	DDX_Control(pDX, IDC_STATIC_DURATION, duration);
	DDX_Control(pDX, IDC_MFCLINK_DETECTIMAGES, detectImageslink);
	DDX_Control(pDX, IDC_MFCLINK_DETECTLOG, detectLoglink);
	DDX_Control(pDX, IDC_MFCLINK_ZIPFILE, zipFilelink);
	DDX_Control(pDX, IDC_CHECK_AUTO, Auto);
	DDX_Control(pDX, IDC_CHECK_EXIT, Exit);
	DDX_Control(pDX, IDC_CHECK_SHUTDOWN, Shutdown);
	DDX_Control(pDX, IDC_CHECK_AS, AS);
	DDX_Control(pDX, IDC_CHECK_DARK, dark);
	DDX_Control(pDX, IDC_CHECK_ACQUISITIONLOG, acquisitionLog);
	DDX_Control(pDX, IDC_CHECK_SER, SER);
	DDX_Control(pDX, IDC_CHECK_SERTIMESTAMP, SERtimestamps);
	DDX_Control(pDX, IDC_CHECK_FITS, FITS);
	DDX_Control(pDX, IDC_CHECK_FILEINFO, FileInfo);
	DDX_Control(pDX, IDC_STATIC_ACQUISITIONSW, acquisitionSW);
	DDX_Control(pDX, IDC_CHECK_EXECAS, execAS);
	DDX_Control(pDX, IDC_STATIC_INSTANCE, Instance);
	DDX_Control(pDX, IDC_STATIC_MAXINST, MaxInstances);
	DDX_Control(pDX, IDC_SPIN_INSTANCES, ValueMaxInstances);
}	

/*
* Maps controls to functions
*/
BEGIN_MESSAGE_MAP(CDeTeCtMFCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDeTeCtMFCDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_BUTTON_CHECKRESULTS, &CDeTeCtMFCDlg::OnBnClickedCheckResultsButton)
	ON_COMMAND(IDC_MFCLINK_DETECTIMAGES, &CDeTeCtMFCDlg::OnDetectImagesClickedOk)
	ON_COMMAND(ID_FILE_OPENFOLDER, &CDeTeCtMFCDlg::OnFileOpenFolder)
	ON_COMMAND(ID_HELP_EXIT, &CDeTeCtMFCDlg::OnHelpExit)
	ON_COMMAND(ID_HELP_TUTORIAL, &CDeTeCtMFCDlg::OnHelpTutorial)
	ON_COMMAND(ID_HELP_DOCUMENTATION, &CDeTeCtMFCDlg::OnHelpDocumentation)
	ON_COMMAND(ID_HELP_CHECKSFORUPDATE, &CDeTeCtMFCDlg::OnHelpChecksForUpdate)
	ON_COMMAND(ID_HELP_HISTORY, &CDeTeCtMFCDlg::OnHelpHistory)
	ON_COMMAND(ID_HELP_PROJECTRESULTS, &CDeTeCtMFCDlg::OnHelpProjectResults)
	ON_COMMAND(ID_SETTINGS_USER, &CDeTeCtMFCDlg::OnSettingsUser)
	ON_COMMAND(ID_SETTINGS_ADVANCED, &CDeTeCtMFCDlg::OnSettingsAdvanced)
	ON_COMMAND(ID_FILE_EXIT, &CDeTeCtMFCDlg::OnFileExit)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDeTeCtMFCDlg::OnLbnSelchangeList1)
	ON_COMMAND(ID_FILE_OPENFILE, &CDeTeCtMFCDlg::OnFileOpenfile)
	ON_COMMAND(ID_FILE_RESETFILELIST, &CDeTeCtMFCDlg::OnFileResetFileList)
	ON_COMMAND(ID_FILE_CLEAREXECUTIONLOG, &CDeTeCtMFCDlg::OnFileClearExecutionLog)
	ON_COMMAND(ID_FILE_CLEARIMPACTFILES, &CDeTeCtMFCDlg::OnFileCleanImpactFiles)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_FRAME, &CDeTeCtMFCDlg::OnBnClickedFrame)
	ON_BN_CLICKED(IDOK3, &CDeTeCtMFCDlg::OnBnClickedOk3)
	ON_STN_CLICKED(IDC_STATICPROBA, &CDeTeCtMFCDlg::OnStnClickedStaticproba)
	ON_BN_CLICKED(IDOK2, &CDeTeCtMFCDlg::OnBnClickedOk2)
	ON_BN_CLICKED(IDC_CHECK_AUTO, &CDeTeCtMFCDlg::OnBnClickedCheckAuto)
	ON_BN_CLICKED(IDC_CHECK_EXIT, &CDeTeCtMFCDlg::OnBnClickedCheckExit)
	ON_BN_CLICKED(IDC_CHECK_SHUTDOWN, &CDeTeCtMFCDlg::OnBnClickedCheckShutdown)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_INSTANCES, &CDeTeCtMFCDlg::OnDeltaposSpinInstances)
END_MESSAGE_MAP()

// CDeTeCtMFCDlg message handlers

/**************************************************************************************************
 * @fn	BOOL CDeTeCtMFCDlg::OnInitDialog()
 *
 * @brief	Initializes this dialog and the controls within it.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL CDeTeCtMFCDlg::OnInitDialog()
{
	if ((opts.autostakkert) && (!opts.parent_instance)) ShowWindow(SW_FORCEMINIMIZE);
	CDialog::OnInitDialog();
	if ((opts.autostakkert) && (!opts.parent_instance)) ShowWindow(SW_FORCEMINIMIZE);

	AS.SetCheck(0);
	dark.SetCheck(0);
	acquisitionLog.SetCheck(0);
	SER.SetCheck(0);
	SERtimestamps.SetCheck(0);
	FITS.SetCheck(0);
	FileInfo.SetCheck(0);
	
	execAS.SetCheck(opts.autostakkert);
	if ((opts.autostakkert) || (opts.detect_PID > 0)) {
		Auto.EnableWindow(FALSE);
		Exit.EnableWindow(FALSE);
		Shutdown.EnableWindow(FALSE);
	}
	Auto.SetCheck((int) (!opts.interactive));
	Exit.SetCheck(opts.autoexit);
	Shutdown.SetCheck(opts.shutdown);


	//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
	BOOL bOk;
	int x_size, y_size;

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(TRUE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	
	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	//SetIcon(m_hIcon, TRUE);			// Set big icon
	//SetIcon(m_hIcon, FALSE);		// Set small icon
	HICON hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	SetIcon(hIcon, FALSE);

	// Set title bar
	//SetWindowText(_T(FULL_PROGNAME));
	std::wstring wstr(app_title.begin(), app_title.end());
	SetWindowText(wstr.c_str());

	// Following does not work
	//	ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);  // enable maximize
	//	ModifyStyle(0, WS_MINIMIZEBOX, SWP_FRAMECHANGED);  // enable minimize
	//	ModifyStyle(1, WS_MAXIMIZEBOX);


	//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing) 
	bOk = m_resizer.Hook(this);
	ASSERT(bOk);

	//  bOk = m_resizer.SetAnchor(IDC_FRAME_MINSIZE, ANCHOR_ALL | ANCHOR_PRIORITY_RIGHT);
	//   ASSERT( bOk );
	//  bOk = m_resizer.SetMinimumSize(IDC_FRAME_MINSIZE, CSize(600, 600));
	//   ASSERT( bOk );
	//	bOk = m_resizer.SetMaximumSize(IDC_FRAME_MINSIZE, CSize(800, 800));
	//   ASSERT( bOk ); 
	//  bOk = m_resizer.SetAnchor(IDC_FRAME_MINSIZE, ANCHOR_ALL | ANCHOR_PRIORITY_RIGHT);
	//   ASSERT(bOk);

	// also set the min/max for this dlg. you have to use "" for the panel name
	//  bOk = m_resizer.SetAnchor(_T("_root"), ANCHOR_ALL | ANCHOR_PRIORITY_RIGHT);
	//  ASSERT(bOk);
	// Limit from full size of window
	// IDD_DETECTMFC_DIALOG DIALOGEX 0, 0, 531, 316
	x_size = 799 - 2;
	y_size = 647 - 52;
	CSize CSize_min = CSize(x_size, y_size);
	bOk = m_resizer.SetMinimumSize(_T("_root"), CSize_min);
	//bOk = m_resizer.SetMinimumSize(_T("_root"), CSize(x_size, y_size));
	ASSERT(bOk);

	//   bOk = m_resizer.SetMaximumSize(_T("_root"), CSize(700, 700));
	//   ASSERT(bOk);

	m_resizer.SetShowResizeGrip(TRUE);
	bOk = m_resizer.InvokeOnResized();
	ASSERT(bOk);

	//end

	CenterWindow();
	if ((opts.autostakkert) && (!opts.parent_instance)) ShowWindow(SW_FORCEMINIMIZE);

	/*
	HWND hWnd = AfxGetMainWnd()->GetSafeHwnd();
		void DisableMinimizeButton(HWND hwnd)
		{
			SetWindowLong(hwnd, GWL_STYLE,
				GetWindowLong(hwnd, GWL_STYLE) & ~WS_MINIMIZEBOX);
		}

		void EnableMinimizeButton(HWND hwnd)
		{
			SetWindowLong(hwnd, GWL_STYLE,
				GetWindowLong(hwnd, GWL_STYLE) | WS_MINIMIZEBOX);
		}

		void DisableMaximizeButton(HWND hwnd)
		{
			SetWindowLong(hwnd, GWL_STYLE,
				GetWindowLong(hwnd, GWL_STYLE) & ~WS_MAXIMIZEBOX);
		}

		void EnableMaximizeButton(HWND hwnd)
		{
			SetWindowLong(hwnd, GWL_STYLE,
				GetWindowLong(hwnd, GWL_STYLE) | WS_MAXIMIZEBOX);
		}
	*/

	// TODO: Add extra initialization here
	impactNull.SetWindowText(L"N/A");
	impactLow.SetWindowText(L"N/A");
	impactHigh.SetWindowText(L"N/A");

	CString maxinstances_cstring;
	short  processor_count = (short) std::thread::hardware_concurrency();
	ValueMaxInstances.SetBuddy(&MaxInstances);
	ValueMaxInstances.SetRange(1, processor_count);
	ValueMaxInstances.SetPos(opts.maxinstances);
	CDeTeCtMFCDlg::getMaxInstances()->SetWindowText(std::to_wstring(opts.maxinstances).c_str() + (CString)"/" + std::to_wstring(processor_count).c_str());

	CWnd *okbtn = GetDlgItem(IDOK);
	if (okbtn) {
		okbtn->EnableWindow(FALSE);
	}
	CWnd *resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
	if (resultsbtn) {
		resultsbtn->EnableWindow(FALSE);
	}
	CDeTeCtMFCDlg::EnableLogLink(FALSE);
	CDeTeCtMFCDlg::EnableImagesLink(FALSE);
	CDeTeCtMFCDlg::EnableZipLink(FALSE);

	CString			instance_cstring;
	CString			nbinstances;
	CString			instance_type_cstring;
	int				nb_instances;
	
	if (opts.parent_instance) {
		nb_instances = 1;
		DisplayInstanceType(&nb_instances);
	}

	std::wstringstream ss2;
	StreamDeTeCtOSversions(&ss2);
	const auto processor_count_str = std::to_string(std::thread::hardware_concurrency());
	ss2 << " " << processor_count_str.c_str() << " processors";
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
	if (opts.dateonly) impactDetectionLog.AddString((CString) "WARNING, datation info only monde on, no detection analysis will be performed");
	if (opts.debug) impactDetectionLog.AddString((CString)"WARNING, debug mode on (lots of verbose and children instances visible, set Debug=0 in DeTeCt.ini to deactivate)");
	if (!opts.interactive) impactDetectionLog.AddString((CString) "Automatic mode on");
	if (!opts.reprocessing) impactDetectionLog.AddString((CString) "No reprocessing mode on");
	if (opts.flat_preparation) impactDetectionLog.AddString((CString) "Creation of image for flat generation");
	if (opts.maxinstances > 1) { // Displays maxinstances if multi instances
		maxinstances_cstring.Format(L"%d", opts.maxinstances);
		impactDetectionLog.AddString(L"Will use maximum " + maxinstances_cstring + " " + PROGNAME + " instances");
	}

	int index_message = 0;
	while ((message_lines[index_message].size() > 1) && (index_message < 100)) {
		impactDetectionLog.AddString((CString)(message_lines[index_message++].c_str()));
	}
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();

	OnCheckUpdate();
	//Call directly file/directory addition if option passed to exe
	if (strlen(opts.dirname) > 0) {
		DIR *dir;
		dir = opendir(opts.dirname);
		if (dir != NULL) {
			closedir(dir);
			OnFileOpenFolder();
		}
		else impactDetectionLog.AddString((CString)getDateTime().str().c_str() + "ERROR : " + opts.dirname + " directory not found.");
	} else if (strlen(opts.filename) > 0) {
		std::ifstream filetest(opts.filename);
		if (filetest) {
			filetest.close();
			OnFileOpenfile();
		}
		else {
			filetest.close();
			impactDetectionLog.AddString((CString)getDateTime().str().c_str() + "ERROR : " + opts.filename + " file not found.");
		}
	}

	std::ifstream filetest(FFMPEGDLL);
	if (!filetest) {
		MessageBox(_T("File ") + CString(FFMPEGDLL) + _T(" not found,\n") + CString(PROGNAME) + _T(" will not be able to open avi, mov, mpg, etc... files\n\nIt will close without processing if finding such files to analyse.\n\nTo fix this, go to menu Help->Version history to download this missing dll from the latest ") + CString(PROGNAME) + _T(" zip file available."), _T("Warning: file ") + CString(FFMPEGDLL) + _T(" not found"), MB_OK + MB_ICONWARNING + MB_SETFOREGROUND + MB_TOPMOST);
	}
	filetest.close();


	return TRUE;  // return TRUE  unless you set the focus to a control
}

BOOL CDeTeCtMFCDlg::EndDialog() {
	//AfxMessageBox(L"End");
	return TRUE;
}

/**************************************************************************************************
 * @fn	BOOL CDeTeCtMFCDlg::OnCheckUpdate()
 *
 * @brief	Initializes this dialog and the controls within it.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL CDeTeCtMFCDlg::OnCheckUpdate()
{
	std::vector<CString> log_cstring_lines;
	AutoUpdate au(&log_cstring_lines);								// For auto updating

	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + "Checking for software update ...\n");
	au.CheckForUpdates(&log_cstring_lines);
	std::for_each(log_cstring_lines.begin(), log_cstring_lines.end(), [&](const CString log_cstring_line) {
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + log_cstring_line);
		});

	return TRUE;
}

void CDeTeCtMFCDlg::EnableImagesLink(BOOL enable) {
	if (enable) {
		CDeTeCtMFCDlg::getdetectImageslink()->EnableWindow(TRUE);
		CDeTeCtMFCDlg::getdetectImageslink()->ShowWindow(SW_SHOW);
	}
	else {
		CDeTeCtMFCDlg::getdetectImageslink()->EnableWindow(FALSE);
		CDeTeCtMFCDlg::getdetectImageslink()->ShowWindow(SW_HIDE);
	}
	CWnd *okbtn = GetDlgItem(IDC_MFCLINK_DETECTIMAGES);
	if (okbtn) {
		if (enable) {
			okbtn->EnableWindow(TRUE);
			okbtn->ShowWindow(SW_SHOW);
		}
		else {
			okbtn->EnableWindow(FALSE);
			okbtn->ShowWindow(SW_HIDE);
		}
	}
}

void CDeTeCtMFCDlg::EnableLogLink(BOOL enable) {
	if (enable) {
		CDeTeCtMFCDlg::getdetectLoglink()->EnableWindow(TRUE);
		CDeTeCtMFCDlg::getdetectLoglink()->ShowWindow(SW_SHOW);
	}
	else {
		CDeTeCtMFCDlg::getdetectLoglink()->EnableWindow(FALSE);
		CDeTeCtMFCDlg::getdetectLoglink()->ShowWindow(SW_HIDE);
	}
	CWnd *okbtn = GetDlgItem(IDC_MFCLINK_DETECTLOG);
	if (okbtn) {
		if (enable) {
			okbtn->EnableWindow(TRUE);
			okbtn->ShowWindow(SW_SHOW);
		}
		else {
			okbtn->EnableWindow(FALSE);
			okbtn->ShowWindow(SW_HIDE);
		}
	}
}

void CDeTeCtMFCDlg::EnableZipLink(BOOL enable) {
	if (enable) {
		CDeTeCtMFCDlg::getzipFilelink()->EnableWindow(TRUE);
		CDeTeCtMFCDlg::getzipFilelink()->ShowWindow(SW_SHOW);
	}
	else {
		CDeTeCtMFCDlg::getzipFilelink()->EnableWindow(FALSE);
		CDeTeCtMFCDlg::getzipFilelink()->ShowWindow(SW_HIDE);
	}
	CWnd *okbtn = GetDlgItem(IDC_MFCLINK_ZIPFILE);
	if (okbtn) {
		if (enable) {
			okbtn->EnableWindow(TRUE);
			okbtn->ShowWindow(SW_SHOW);
		}
		else {
			okbtn->EnableWindow(FALSE);
			okbtn->ShowWindow(SW_HIDE);
		}
	}
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
 *
 * @brief	Executes the system command action.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param	nID   	The identifier.
 * @param	lParam	The lParam field of the message.
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	//AfxMessageBox((CString)(std::to_string(nID)).c_str() + (CString)(" | ") + (CString)(std::to_string(SC_CLOSE).c_str()));
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_CLOSE) CDeTeCtMFCDlg::OnFileExit();
	else CDialog::OnSysCommand(nID, lParam);
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnPaint()
 *
 * @brief	Paints this window.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.

/**************************************************************************************************
 * @fn	HCURSOR CDeTeCtMFCDlg::OnQueryDragIcon()
 *
 * @brief	Executes the query drag icon action.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	The handle of the cursor.
 **************************************************************************************************/

HCURSOR CDeTeCtMFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnBnClickedOk()
 *
 * @brief	Executes the button clicked ok action -- Runs the algorithm in an independent thread
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

#include "winuser.h"
#include <direct.h>

void CDeTeCtMFCDlg::OnBnClickedOk()
{
	char buffer[MAX_STRING] = { 0 };
	sprintf_s(buffer, MAX_STRING, "OnBnClickedOk:	opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
	OutputDebugStringA(buffer);
	
	CWnd *okbtn = GetDlgItem(IDOK);
	if (okbtn) {
		okbtn->EnableWindow(FALSE);
		//okbtn->SetIcon(AfxGetApp()->LoadIcon(IDR_MAINFRAME), FALSE);
	}
	CWnd *openfolderbtn = GetDlgItem(IDOK3);
	if (openfolderbtn) {
		openfolderbtn->EnableWindow(FALSE);
	}
	CWnd *openfilebtn = GetDlgItem(IDOK2);
	if (openfilebtn) {
		openfilebtn->EnableWindow(FALSE);
	}
	if (acquisition_files.file_list.size() > 0) {

		if (opts.dateonly) 	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"WARNING, datation info only, no detection analysis will be performed\n");
		else impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"Running analysis");

		// Disable file menus
		CMenu *mmenu = GetMenu();
		CMenu *submenu = mmenu->GetSubMenu(0);
		submenu->EnableMenuItem(ID_FILE_OPENFOLDER,		MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_FILE_OPENFILE,		MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);
		submenu->EnableMenuItem(ID_FILE_RESETFILELIST,	MF_BYCOMMAND | MF_DISABLED | MF_GRAYED);

		/*CMenu *okbtn1 = GetMenuItemInfo(MENU);
		if (okbtn1) {
			okbtn1->EnableMenuItem(ID_FILE_OPENFOLDER,FALSE);
		}*/
/*		CWnd *okbtn2 = GetDlgItem(ID_FILE_OPENFILE);
		if (okbtn2) {
			okbtn2->EnableWindow(FALSE);
		}
		CWnd *okbtn3 = GetDlgItem(ID_FILE_RESETFILELIST);
		if (okbtn3) {
			okbtn3->EnableWindow(FALSE);
		}*/

		/* C++ standard threading */
		/*
		std::thread detection_thread(detect, file_list, opts, scan_folder_path);
		detection_thread.detach();
		*/

		/* MFC native threading, check DetechThread.h/cpp for details */
		ImpactDetectParams* params = new ImpactDetectParams(acquisition_files.file_list, &opts, scan_folder_path);
		sprintf_s(buffer, MAX_STRING, "OnBnClickedOk2a:	popts   : %p	popts->ignore	:	%i\n", params->popts, params->popts->ignore);
		OutputDebugStringA(buffer); 
		sprintf_s(buffer, MAX_STRING, "OnBnClickedOk2a:	opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
		OutputDebugStringA(buffer);

		AfxBeginThread(impactDetection, (LPVOID)params);
		//AfxBeginThread(impactDetection, (ImpactDetectParams*)params);
	}
	else {
		if ((opts.autostakkert) && (opts.parent_instance)) { // launch detection function even if no file available (to go to waiting queue)
			/* MFC native threading, check DetechThread.h/cpp for details */
			ImpactDetectParams* params = new ImpactDetectParams(acquisition_files.file_list, &opts, scan_folder_path);
			sprintf_s(buffer, MAX_STRING, "OnBnClickedOk2b:	popts   : %p	popts->ignore	:	%i\n", params->popts, params->popts->ignore);
			OutputDebugStringA(buffer); 
			sprintf_s(buffer, MAX_STRING, "OnBnClickedOk2b:	opts    : %p	opts->ignore	:	%i\n", &opts, opts.ignore);
			OutputDebugStringA(buffer);
			AfxBeginThread(impactDetection, (LPVOID)params);
			//AfxBeginThread(impactDetection, (ImpactDetectParams*)params);
		}
		else {
			impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"Error: no files selected");
			if (!opts.interactive) dlg.OnFileExit();
		}
	}
}

/**********************************************************************************************/
 /*	Action of button to check detection image folder and send zip file                        */
/**********************************************************************************************/

void CDeTeCtMFCDlg::OnBnClickedCheckResultsButton()
{
	//extern char impact_detection_dirname[MAX_STRING];
	//extern char zip_detection_location[MAX_STRING];
	//extern char zipfile[MAX_STRING];
	//extern char log_detection_dirname[MAX_STRING];
	//extern char email_subject_probabilities[MAX_STRING];
	//extern char email_body_probabilities[MAX_STRING];

	wchar_t	wimpact_detection_dirname[MAX_STRING];
	size_t ReturnValue;
	mbstowcs_s(&ReturnValue, wimpact_detection_dirname, strlen(impact_detection_dirname) + 1, impact_detection_dirname, strlen(impact_detection_dirname));

	// email start
	char	mailto_command[MAX_STRING]	= { 0 };
	wchar_t	wmailto_command[MAX_STRING] = { 0 };
	if (opts.email) {
		strcpy(mailto_command, "mailto:delcroix.marc@free.fr?subject=Impact detection ");
		strcat_s(mailto_command, sizeof(mailto_command), log_detection_dirname);
		strcat_s(mailto_command, sizeof(mailto_command), email_subject_probabilities);
		strcat(mailto_command, "&body=(*please attach ");
	}

	// Zip post-processing
	if (strlen(zip_detection_location) > 0) {

		wchar_t wzip_detection_location[MAX_STRING];
		mbstowcs(wzip_detection_location, zip_detection_location, strlen(zip_detection_location) + 1);//Plus null
		ShellExecute(NULL, L"explore", wzip_detection_location, NULL, NULL, SW_SHOWNORMAL);

		if (opts.email) {
			// email continued with zip file
			strcat(mailto_command, "impact_detection zip file ");
			strcat_s(mailto_command, sizeof(mailto_command), zipfile);
		}
	}
	else {
		if (opts.email) {
			// email continued with detection log and images
			strcat(mailto_command, "detection images and detect log file from ");
			strcat(mailto_command, impact_detection_dirname);
		}
	}
	//E-Mail post-processing
	if (opts.email) {
		// email end
		strcat(mailto_command, " *)%0A%0AHi Marc,%0A%0AHere are the results and the images of my analysis with ");
		strcat(mailto_command, PROGNAME);
		strcat(mailto_command, " v");
		strcat(mailto_command, VERSION_NB);
		strcat(mailto_command, ", I checked the images and found: %0A*please check: (X)*%0A ( ) nothing suspect%0A ( ) something suspect like an impact%0A%0ADetails:%0A");
		strcat(mailto_command, email_body_probabilities);
		strcat(mailto_command, "%0A%0ACheers,%0A");
		mbstowcs(wmailto_command, mailto_command, strlen(mailto_command) + 1);
		ShellExecute(NULL, L"open", wmailto_command, NULL, NULL, SW_SHOWNORMAL);
	}

	//Explorer post-processing
	ShellExecute(NULL, L"explore", wimpact_detection_dirname, NULL, NULL, SW_SHOWNORMAL);
	//SendMail(zip_detection_dirname);
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnFileOpenfile()
 *
 * @brief	Executes the file open dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileOpenfile()
{
	acquisition_files.file_list = {};
	acquisition_files.acquisition_file_list = {};
	acquisition_files.nb_prealigned_frames = {};
	std::wstringstream ss, ssint, ssopt;
	std::wstring file_path;
	std::string file;
	std::string filename_acquisition;
	BOOL exit_detect = FALSE;

	impactNull.SetWindowText(L"N/A");
	impactLow.SetWindowText(L"N/A");
	impactHigh.SetWindowText(L"N/A");
	// Disables detection/results buttons
	CWnd *okbtn = GetDlgItem(IDOK);
	if (okbtn) {
		okbtn->EnableWindow(FALSE);
	}
	CWnd *resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
	if (resultsbtn) {
		resultsbtn->EnableWindow(FALSE);
	}
	CDeTeCtMFCDlg::EnableLogLink(FALSE);
	CDeTeCtMFCDlg::EnableImagesLink(FALSE);
	CDeTeCtMFCDlg::EnableZipLink(FALSE);

	CFileDialog dialog(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, filter, this, sizeof(OPENFILENAME), true);

	// Gets filename from parameter or dialog window
	if (opts.debug) {
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"!Debug info: AS PID=" + (CString)std::to_string(opts.autostakkert).c_str() + L" " + (CString)std::to_string(opts.autostakkert_PID).c_str());
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
	}

	if (strlen(opts.filename) > 0) file = std::string(opts.filename);
	else if (dialog.DoModal() == IDOK) {
		file_path = std::wstring(dialog.GetPathName().GetString());
		file = wstring2string(file_path);
	}
	std::string extension = file.substr(file.find_last_of(".") + 1, file.size() - file.find_last_of(".") - 1);

	if (file.size() <= 0) {
		//********* Error if no file selected

		ss << "No file selected";
		if ((opts.autostakkert) && (!opts.parent_instance)) exit_detect = TRUE; //Exit autostakkert child if no file to process
		//			if (!opts.interactive) CDeTeCtMFCDlg::OnFileExit();
	}
	else {
		std::ifstream filetest(file);
		if (!filetest) {
			// ********* Error if file is missing
			ss << "Error, ignoring " << file.c_str() << ", cannot open file (" << strerror(errno) << ")\n";
			if ((opts.autostakkert) && (!opts.parent_instance)) exit_detect = TRUE; //Exit autostakkert child if no file to process
		}
		else {
			// Clears window
			std::wstringstream ss2;
			OnFileResetFileList();
			CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);

			// Gets file acquisition name from autostakkert session file
			int nframe = -1;
			PIPPInfo pipp_info;

			if ((Is_Capture_OK_from_File(file, &filename_acquisition, &nframe, &ss)) &&
				// ********* Error if acquisition has not enough frames
				(Is_Capture_Long_Enough(file, nframe, &ss)) &&
				// ********* Ignores dark, pipp, winjupos derotated files
				(!Is_Capture_Special_Type(file, &ss)) &&
				// ********* Ignores PIPP with no integrity
				(!Is_PIPP(file) || ((Is_PIPP(file) && Is_PIPP_OK(file, &pipp_info, &ss))))) {
				std::string folder_path;
				if (!opts.autostakkert) folder_path = filename_acquisition.substr(0, filename_acquisition.find_last_of("\\"));
				else {
					//log directory when autostakkert mode or multi instance mode
					folder_path = CString2string(DeTeCt_exe_folder());
				}
				CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder((CString)folder_path.c_str(), DTC_LOG_SUFFIX));
				std::string log_file(DeTeCtLogFilename);
if (opts.debug) impactDetectionLog.AddString(L"!Debug info : Logfile=" +  (CString)log_file.c_str());
				if (Is_CaptureFile_To_Be_Processed(filename_acquisition, log_file, &ss)) {
					// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode
												// ********* Finally adds file to the list !
													// Set-up global variable
					scan_folder_path = file.substr(0, file.find_last_of("\\"));
					acquisition_files.file_list.push_back(file);
					file = file.substr(file.find_last_of("\\") + 1, file.length());
					if (extension.compare(AUTOSTAKKERT_EXT) != 0) ss << "Adding " << file.c_str() << " (in " << scan_folder_path.c_str() << ") for analysis\n";
					else ss << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << scan_folder_path.c_str() << ") for analysis\n";

					CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(_T("Total\n(0/1)"));
					CDeTeCtMFCDlg::getfileName()->SetWindowText(L"Click on Detect impacts!");
					okbtn->EnableWindow(TRUE);
					resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
					if (resultsbtn) {
						resultsbtn->EnableWindow(FALSE);
					}
				}
				else if ((opts.autostakkert) && (!opts.parent_instance)) exit_detect = TRUE; //Exit autostakkert child if no file to process
			}
			else if ((opts.autostakkert) && (!opts.parent_instance)) exit_detect = TRUE; //Exit autostakkert child if no file to process
		}
		filetest.close();
	}
	if (exit_detect) CDeTeCtMFCDlg::OnFileExit();

	if (opts.debug) {
		impactDetectionLog.AddString(L"!Debug info: AS PID=" + (CString)std::to_string(opts.autostakkert).c_str() + L" " + (CString)std::to_string(opts.autostakkert_PID).c_str());
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
	}

	// Prints message
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();
	if (((acquisition_files.file_list.size() > 0) && (!opts.interactive)) || ((opts.autostakkert && (acquisition_files.file_list.size() <= 0)))) OnBnClickedOk(); // xxxx or if no file and autostakkert mode continue
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnFileOpenFolder()
 *
 * @brief	Executes the multiple file open action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileOpenFolder()
{
	acquisition_files.file_list = {};
	acquisition_files.acquisition_file_list = {};
	acquisition_files.nb_prealigned_frames = {};
	std::wstring		folder_path;
	std::string			path;
	std::wstringstream	ss;
	int					files_count = 0;
	
	impactNull.SetWindowText(L"N/A");
	impactLow.SetWindowText(L"N/A");
	impactHigh.SetWindowText(L"N/A");
	// Disables detection/results buttons
	CWnd *okbtn = GetDlgItem(IDOK);
	if (okbtn) {
		okbtn->EnableWindow(FALSE);
	}
	CWnd *resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
	if (resultsbtn) {
		resultsbtn->EnableWindow(FALSE);
	}
	CDeTeCtMFCDlg::EnableLogLink(FALSE);
	CDeTeCtMFCDlg::EnableImagesLink(FALSE);
	CDeTeCtMFCDlg::EnableZipLink(FALSE);

	CFolderPickerDialog dialog(NULL, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this, sizeof(OPENFILENAME));

	if (strlen(opts.dirname) > 0) {
		path = std::string(opts.dirname);
	}
	else {
		if (dialog.DoModal() == IDOK) {
			folder_path = std::wstring(dialog.GetPathName().GetString());
			path = wstring2string(folder_path);
		}
	}
	if (path.size() <= 0) {
		ss << "No folder selected";
//		if (!opts.interactive) CDeTeCtMFCDlg::OnFileExit();
	} else {
		std::wstringstream ss2;

	//TODO: clearscreen
		OnFileResetFileList();
	// Set-up global variable
		scan_folder_path = path;
		strcpy(opts.LogConsolidatedDirname, path.c_str());
		ss2 << "Scanning " << folder_path << " for files to be analysed, please wait...";
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
		CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
		CDeTeCtMFCDlg::getLog()->RedrawWindow();
		
		read_files(path, &acquisition_files);
		
		if (acquisition_files.file_list.size() > 0) {
			int index = 0;
			std::string filename;
			while (index< acquisition_files.file_list.size()) {
				filename = acquisition_files.file_list.at(index);
				std::wstringstream ss3;
				std::string filename_acquisition;
				int nframe = -1;
				PIPPInfo pipp_info;

				if ((Is_Capture_OK_from_File(filename, &filename_acquisition, &nframe, &ss3)) &&
					// ********* Ignores if less than minimum frames
					(Is_Capture_Long_Enough(filename, nframe, &ss3)) &&
					// ********* Ignores dark, pipp, winjupos derotated files
					(!Is_Capture_Special_Type(filename, &ss3)) &&
					// ********* Ignores PIPP with no integrity
					(!Is_PIPP(filename) || ((Is_PIPP(filename) && Is_PIPP_OK(filename, &pipp_info, &ss3))))) {
						std::string folder_path_consolidated;
						if (!opts.autostakkert) folder_path_consolidated = std::string (folder_path.begin(), folder_path.end());
						else {
							//log directory when autostakkert mode or multi instance mode
							folder_path_consolidated = CString2string(DeTeCt_exe_folder());
						}
						CT2A DeTeCtLogFilename(DeTeCt_additional_filename_from_folder((CString)folder_path_consolidated.c_str(), DTC_LOG_SUFFIX));
						std::string log_file(DeTeCtLogFilename);
if (opts.debug) impactDetectionLog.AddString(L"!Debug info: Logfile=" + (CString)log_file.c_str());
						if (Is_CaptureFile_To_Be_Processed(filename_acquisition, log_file, &ss3)) {
					// ***** if option noreprocessing on, check in detect log file if file already processed or processed with in datation only mode
								// ********* Finally adds file to the list !
							std::string extension	= filename.substr(filename.find_last_of(".") + 1, filename.size() - filename.find_last_of(".") - 1);
							std::string file		= filename.substr(filename.find_last_of("\\") + 1, filename.length());
							std::string file_path	= filename.substr(0, filename.find_last_of("\\"));
// Debug
							if ((index >= 0) && (strlen(opts.DeTeCtQueueFilename) > 1)) {			// MODIFIED to keep even 1st file in list!
// Debug
							//if ((index>0) && (opts.maxinstances > 1)) {			// if multi instances mode, keep only one acquisition in the list and the rest in the queue
								CString log_cstring;
								if ((index == 0) && (!GetItemFromQueue(&log_cstring, L"output_dir: ", (CString)opts.DeTeCtQueueFilename, NULL, TRUE))) {	// defines output directory if not already set
									std::string output_dir = path;
									output_dir.append("\\Impact_detection_run@").append(getRunTime().str().c_str());
									CString output_dir_cstring(output_dir.c_str());
									PushItemToQueue(output_dir_cstring, L"output_dir", (CString) opts.DeTeCtQueueFilename, NULL, TRUE);
									opts.parent_instance = TRUE;
								}
								CString tmp, tmp2;
								if (!filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) {
									 char msgtext[MAX_STRING] = { 0 };
									snprintf(msgtext, MAX_STRING, "cannot find acquisitions queue file %s", opts.DeTeCtQueueFilename);
									ErrorExit(TRUE, "cannot finf acquisitions queue file", "OnFileOpenFolder()", msgtext);
								}
								else PushFileToQueue(char2CString(filename.c_str(), &tmp), char2CString(opts.DeTeCtQueueFilename, &tmp2));
								if (index > 0) {// MODIFIED: if multi instances mode, keep only one acquisition in the list and the rest in the queue
									acquisition_files.file_list.erase(acquisition_files.file_list.begin() + index);
									acquisition_files.acquisition_file_list.erase(acquisition_files.acquisition_file_list.begin() + index);
									acquisition_files.nb_prealigned_frames.erase(acquisition_files.nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
								}
								else index++;
								if (extension.compare(AUTOSTAKKERT_EXT) != 0) ss3 << "Adding " << file.c_str() << " (in " << file_path.c_str() << ") for analysis\n";
								else ss3 << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << file_path.c_str() << ") for analysis\n";
							}
							else {
								if (extension.compare(AUTOSTAKKERT_EXT) != 0) ss3 << "Adding " << file.c_str() << " (in " << file_path.c_str() << ") for analysis\n";
								else ss3 << "Adding " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " in " << file_path.c_str() << ") for analysis\n";
								index++;
							}
							files_count++;
							std::wstring totalProgress_wstring = L"Total\n(0/" + std::to_wstring(files_count) + L")";
							CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());
						}
						else {
							acquisition_files.file_list.erase(acquisition_files.file_list.begin() + index);
							acquisition_files.acquisition_file_list.erase(acquisition_files.acquisition_file_list.begin() + index);
							acquisition_files.nb_prealigned_frames.erase(acquisition_files.nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
						}
					}
					else {
						acquisition_files.file_list.erase(acquisition_files.file_list.begin() + index);
						acquisition_files.acquisition_file_list.erase(acquisition_files.acquisition_file_list.begin() + index);
						acquisition_files.nb_prealigned_frames.erase(acquisition_files.nb_prealigned_frames.begin() + index); // WARNING in debug, error in .begin()
					}
					impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss3.str().c_str());
					CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
					//this->RedrawWindow();
					CDeTeCtMFCDlg::getLog()->RedrawWindow();
			}
		}
		if (acquisition_files.file_list.size() <= 0) {
			ss << "No file selected";
		}
		else {
			ss << std::to_string(files_count).c_str() << " files added for analysis";
			okbtn->EnableWindow(TRUE);
			CDeTeCtMFCDlg::getfileName()->SetWindowText(L"Click on Detect impacts!");
			resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
			if (resultsbtn) {
				resultsbtn->EnableWindow(FALSE);
			}
		}
	}
	std::wstring totalProgress_wstring = L"Total\n(0/" + std::to_wstring(files_count) + L")";
	CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(totalProgress_wstring.c_str());

	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getLog()->RedrawWindow();
	//this->RedrawWindow();
	if (opts.clean_dir) CDeTeCtMFCDlg::OnFileCleanImpactFiles();
	if ((acquisition_files.file_list.size() > 0) && (!opts.interactive)) OnBnClickedOk();
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpExit()
 *
 * @brief	opens about window
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpExit()
{
	CAboutDlg *about = new CAboutDlg();
	about->DoModal();
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpTutorial()
 *
 * @brief	opens tutorial webpage
 *
 * @author	Marc
 * @date	2019-10-24
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpTutorial()
{
	ShellExecute(NULL, L"open", L"http://astrosurf.com/planetessaf/doc/dtc/doc/detect_guide/DeTeCt_quick_guide.htm", NULL, NULL, SW_SHOWNORMAL);
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpChecksForUpdate()
 *
 * @brief	Opens latest version webpage
 *
 * @author	Marc
 * @date	2019-10-24
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpChecksForUpdate()
{
	OnCheckUpdate();
}
/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpChecksForUpdate()
 *
 * @brief	Opens latest version webpage
 *
 * @author	Marc
 * @date	2019-10-24
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpHistory()
{
	ShellExecute(NULL, L"open", L"https://github.com/DeTeCt-PSWS/DeTeCt-MFC/releases", NULL, NULL, SW_SHOWNORMAL);
}
/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpDocumentation()
 *
 * @brief	opens documentation page
 *
 * @author	Marc
 * @date	2019-10-24
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnDetectImagesClickedOk()
{
		ShellExecute(NULL, L"open", L"https://github.com/DeTeCt-PSWS/DeTeCt-MFC/releases/latest", NULL, NULL, SW_SHOWNORMAL);
}

void CDeTeCtMFCDlg::OnHelpDocumentation()
{
	ShellExecute(NULL, L"open", L"https://github.com/DeTeCt-PSWS/Documentation/releases/latest", NULL, NULL, SW_SHOWNORMAL);
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnHelpProjectResults()
 *
 * @brief	opens project webpage
 *
 * @author	Marc
 * @date	2019-10-24
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpProjectResults()
{
	ShellExecute(NULL, L"open", L"http://www.astrosurf.com/planetessaf/doc/project_detect.shtml", NULL, NULL, SW_SHOWNORMAL);
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnSettingsAdvanced()
 *
 * @brief	Executes the settings -> preferences action. Opens the preferences window
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnSettingsAdvanced()
{
	PrefDialog *SettingsAdvanced = new PrefDialog();
	SettingsAdvanced->DoModal();
}

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnSettingsUser()
 *
 * @brief	Executes the settings -> preferences action. Opens the preferences window
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnSettingsUser()
{
	PrefDialogUser* SettingsUser = new PrefDialogUser();
	SettingsUser->DoModal();
}

void CDeTeCtMFCDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// Limits the size to the frame
	// cf. http://www.flounder.com/getminmaxinfo.htm

	if (c_Frame.GetSafeHwnd() != NULL)            // [1]
	{	// has frame
		CRect r;                                 // [2]
		c_Frame.GetWindowRect(&r);               // [3]
		ScreenToClient(&r);                      // [4]
		CalcWindowRect(&r);                      // [5] 
		lpMMI->ptMinTrackSize.x = r.Width();     // [6]
		lpMMI->ptMinTrackSize.y = r.Height();    // [7]
	}	// has frame
	else
		CDialog::OnGetMinMaxInfo(lpMMI);          // [8]
}

/**************************************************************************************************
 * @fn	OnFileResetFileList()
 *
 * @brief	Reset to be processed file list
 *
 * @author	Marc
 * @date	2020-04-18
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileResetFileList() {
	// Init
	if (opts.parent_instance) {
		remove(opts.DeTeCtQueueFilename);
		strcpy(opts.DeTeCtQueueFilename, "");
	}
	
	acquisition_files.file_list = {};
	acquisition_files.acquisition_file_list = {};
	acquisition_files.nb_prealigned_frames = {};
	opts.interactive_bak =			opts.interactive;
	//opts.autostakkert =				FALSE;
	//opts.autostakkert_PID =			0;
	//opts.detect_PID =				0;
	//opts.parent_instance =			FALSE;
	//opts.filename =					NULL;
	//opts.dirname =					NULL;
	init_string(opts.filename);
	init_string(opts.dirname);
	strcpy(opts.LogConsolidatedDirname, "");
	
	CreateQueueFileName();
	
	// Disables detection/results buttons
	CWnd *okbtn = GetDlgItem(IDOK);
	if (okbtn) {
		okbtn->EnableWindow(FALSE);
	}
	CWnd *resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
	if (resultsbtn) {
		resultsbtn->EnableWindow(FALSE);
	}
	CDeTeCtMFCDlg::EnableLogLink(FALSE);
	CDeTeCtMFCDlg::EnableImagesLink(FALSE);
	CDeTeCtMFCDlg::EnableZipLink(FALSE);
	CDeTeCtMFCDlg::gettotalProgress()->SetWindowText(_T("Total\n(0/0)"));
	
	std::wstringstream ss2;
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"\n");
	ss2 << "Resetting file list for analysis";
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
	CDeTeCtMFCDlg::getLog()->SetTopIndex(CDeTeCtMFCDlg::getLog()->GetCount() - 1);
	CDeTeCtMFCDlg::getfileName()->SetWindowText(L"Open file or folder!");
	CDeTeCtMFCDlg::getLog()->RedrawWindow();
}

/**************************************************************************************************
 * @fn	OnFileClearExecutionLog()
 *
 * @brief	Clears execution full log window
 *
 * @author	Marc
 * @date	2020-04-18
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileClearExecutionLog() {
	CDeTeCtMFCDlg::getLog()->ResetContent();
}

/**************************************************************************************************
 * @fn	OnFileCleanImpactFiles()
 *
 * @brief	Clears execution full log window
 *
 * @author	Marc
 * @date	2020-04-18
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileCleanImpactFiles() {
	bool return_value = TRUE;

	// test empty directory 
	if (strlen(opts.LogConsolidatedDirname) == 0) {
		Warning(TRUE, "Clean impact files", "", "Please select folder first...");
		bool interactive_status = opts.interactive;
		opts.interactive = TRUE;
		OnFileOpenFolder();
		opts.interactive = interactive_status;
		if (strlen(opts.LogConsolidatedDirname) == 0) {
			Info(TRUE, "Clean impact files", "", "No folder selected, nothing to be cleaned...");
			return;
		}
	}

	DIR*			directory	= NULL;
	struct dirent*	entry		= NULL;
	std::vector<std::string> file_to_be_deleted_list = {};
	std::vector<std::string> directory_to_be_deleted_list = {};
	CString DeTeCtRoot = (DeTeCt_additional_filename_from_folder(L"", L""));
	CT2CA pszConvertedAnsiString(DeTeCtRoot);
	std::string DeTeCtRoot_string(pszConvertedAnsiString);
	CString message = L"List of files and folders which will be deleted:\n";

	directory = opendir(opts.LogConsolidatedDirname);
	if (directory == NULL) {
		closedir(directory);
		return;
	}
	entry = readdir(directory);
	if (entry == NULL) {
		closedir(directory);
		return;
	}
	do {
		if (entry->d_type == DT_DIR) { //directory
			if ((!(strcmp(entry->d_name, ".") == 0) && !(strcmp(entry->d_name, "..") == 0)) && (starts_with(entry->d_name, "Impact_detection"))) {
				std::string folder(entry->d_name);
				directory_to_be_deleted_list.push_back(std::string(opts.LogConsolidatedDirname) + "\\" + entry->d_name);
				message = message + (CString)(folder.c_str()) + L"\n";
			}
		}
		else { //file
			std::string file(entry->d_name);
			std::string extension = file.substr(file.find_last_of(".") + 1, file.length());
			if (((starts_with(file, DeTeCtRoot_string) || (starts_with(file, "output"))) && (extension == "log")) || ((starts_with(file, "Impact_detection_run@")) && (extension == "zip"))) {
				file_to_be_deleted_list.push_back(std::string(opts.LogConsolidatedDirname) + "\\" + entry->d_name);
				message = message + (CString)(file.c_str()) + L"\n";
			}
		}
	} while ((entry = readdir(directory)) != 0);

	if ((directory_to_be_deleted_list.size() == 0) && (file_to_be_deleted_list.size() == 0)) {
		MessageBox(_T("Nothing to be deleted in \"") + CString(opts.LogConsolidatedDirname) + _T("\", exiting."), _T("Clear impact files"), MB_OK + MB_ICONINFORMATION + MB_SETFOREGROUND + MB_TOPMOST);
		return;
	}
	if (!(MessageBox(message + _T("Are you sure you want to delete those elements from \"") + CString(opts.LogConsolidatedDirname) + _T("\" ?"), _T("Clear impact files"), MB_OKCANCEL + MB_ICONQUESTION + MB_SETFOREGROUND + MB_TOPMOST) == IDOK)) return;

	std::for_each(directory_to_be_deleted_list.begin(), directory_to_be_deleted_list.end(), [&](const std::string dirname) {
		if (!rmdir_force(dirname.c_str())) return_value = FALSE;
	});
	std::for_each(file_to_be_deleted_list.begin(), file_to_be_deleted_list.end(), [&](const std::string filename) {
		if (!remove(filename.c_str())) return_value = FALSE;
	});
}
	
/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnFileExit()
 *
 * @brief	Executes the exit action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileExit()
{
	cv::destroyWindow("Detection image");
	
	if (opts.parent_instance) {
		CString message;
		char tmp[MAX_STRING] = { 0 };
		// interactive status was forced FALSE in autostakkert parent mode, saves the initial value if not manually modified afterwards
		if ((opts.autostakkert) && (!opts.interactive)) opts.interactive = opts.interactive_bak;		
		WriteIni();																				// writes parameters only if not child mode
		message = L"";
		CWnd *okbtn = GetDlgItem(IDOK);
		CWnd *resultsbtn = GetDlgItem(IDC_BUTTON_CHECKRESULTS);
		if ((okbtn && !okbtn->IsWindowEnabled())) {
			if (resultsbtn && !resultsbtn->IsWindowEnabled()) {
				if ((opts.autostakkert_PID > 0) && (IsParentAutostakkertRunning(opts.autostakkert_PID))) message = message + (CString)"Autostakkert still running, please close it first.\n";
				if ((acquisition_files.acquisition_file_list.size() > 0) || filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) message = message + (CString)"Impact detection still running.\n";
				//else message = message + (CString)"Folder/files not selected yet.\n"; 
			}
		}
		else message = message + (CString)"Impact detection not run yet.\n";

		if (strlen(CString2char(message, tmp)) > 1) {
			if (!(MessageBox(message + _T("Are you sure you want to stop and exit DeTeCt ?"), _T("Close"), MB_OKCANCEL + MB_SETFOREGROUND + MB_TOPMOST) == IDOK)) return; // exits only with confirmation for parent instance
		}
		KillsChildrenProcesses();
		remove(opts.DeTeCtQueueFilename);
		strcpy(opts.DeTeCtQueueFilename, "");
		CDialog::OnOK();
	}
	else {		// exit without confirmation for child instances
		KillsChildrenProcesses(); //Kills images left???
		CDialog::OnOK();
	}
}



//******************************************************************************************************************************************************************************************************************************************************************************************
// PrefDialog dialog
//******************************************************************************************************************************************************************************************************************************************************************************************

IMPLEMENT_DYNAMIC(PrefDialog, CDialog)

/**
* Maps the IDS of the controls defined above to the actions which are the functions below
*/
BEGIN_MESSAGE_MAP(PrefDialog, CDialog)
	ON_BN_CLICKED(ID_PREFOK,			&PrefDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK15,			&PrefDialog::OnBnClickedCheck15)			//use filter
	ON_BN_CLICKED(IDC_BUTTON1,			&PrefDialog::OnBnClickedButton1)			//Reset to default
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1,	&PrefDialog::OnDeltaposSpin1)	//Mean value (min impact strength)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2,	&PrefDialog::OnDeltaposSpin2)	//Impact mean time
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN14,	&PrefDialog::OnDeltaposSpin14)	//Histscale
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN12,	&PrefDialog::OnDeltaposSpin12)	//ROI size factor
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN13,	&PrefDialog::OnDeltaposSpin13)	//ROI sec factor
	ON_CBN_SELCHANGE(IDC_COMBO2,		&PrefDialog::OnCbnSelchangeCombo2)		//Debayer
END_MESSAGE_MAP()


/**************************************************************************************************
 * @fn	PrefDialog::PrefDialog(CWnd* pParent )
 *
 * @brief	Constructor.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pParent	If non-null, the parent.
 **************************************************************************************************/

PrefDialog::PrefDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SETTINGS_ADVANCED, pParent)
{
}

/**************************************************************************************************
 * @fn	PrefDialog::~PrefDialog()
 *
 * @brief	Destructor.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

PrefDialog::~PrefDialog()
{
}

/**************************************************************************************************
 * @fn	BOOL PrefDialog::OnInitDialog()
 *
 * @brief	Initializes this dialog and the controls within it. Sets the values of the controls to
 * 			the current option values. And sets the spinners to have reasonable value ranges.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL PrefDialog::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::wstringstream ss;

	//Processing visualisation
	showROI.SetCheck(opts.viewROI);
	showTrack.SetCheck(opts.viewTrk);
	showDif.SetCheck(opts.viewDif);
	showRef.SetCheck(opts.viewRef);
	showMask.SetCheck(opts.viewMsk);
	showThresh.SetCheck(opts.viewThr);
	showSmooth.SetCheck(opts.viewSmo);
	showHist.SetCheck(opts.viewHis);
	showResult.SetCheck(opts.viewRes);
	
	//Impact
	meanValueSpin.SetBuddy(&meanValue);
	meanValueSpin.SetRange(0, 1);
	minTimeSpin.SetBuddy(&impactMinTime);
	minTimeSpin.SetRange(0, 1);
	radiusSpin.SetBuddy(&impactRadius);
	radiusSpin.SetRange(5, 20);
	brightThreshSpin.SetBuddy(&impactBrightThresh);
	brightThreshSpin.SetRange(0, 255);
	
	ss << std::fixed << std::setprecision(2) << opts.incrLumImpact;
	meanValue.SetWindowText(ss.str().c_str());
	impactFrameNum.SetWindowText(std::to_wstring(opts.nframesRef).c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.incrFrameImpact;
	impactMinTime.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << opts.radius;
	impactRadius.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << opts.threshold;
	impactBrightThresh.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.facSize;
	applyMask.SetCheck(opts.thrWithMask);

	saveIntFramesADUdtc.SetCheck(opts.allframes);
	NoZip.SetCheck((int)(!opts.zip));
	Debug.SetCheck(opts.debug);
	CleanDir.SetCheck(opts.clean_dir);

	//ROI
	sizeFactSpin.SetBuddy(&roiSizeFactor);
	sizeFactSpin.SetRange(0, 2);
	secFactSpin.SetBuddy(&roiSecFactor);
	secFactSpin.SetRange(0, 1);
	medianBufSpin.SetBuddy(&roiMedianBufSize);
	medianBufSpin.SetRange(5, 50);
	
	roiSizeFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.secSize;
	roiSecFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << opts.medSize;
	roiMedianBufSize.SetWindowText(ss.str().c_str());

	//Other processingconfiguration
	nframeSpin.SetBuddy(&impactFrameNum);
	nframeSpin.SetRange(1, 50);
	minFrameSpin.SetBuddy(&minimumFrames);
	minFrameSpin.SetRange(3, 10000);
	histoSpin.SetBuddy(&histScale);
	histoSpin.SetRange(0, 1);
	
	minimumFrames.SetWindowText(std::to_wstring(opts.minframes).c_str()); //not shown
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.histScale;
	histScale.SetWindowText(ss.str().c_str());

	debayeringCode.AddString(L"None");
	debayeringCode.AddString(L"RGGB");
	debayeringCode.AddString(L"GRBG");
	debayeringCode.AddString(L"BGGR");
	debayeringCode.AddString(L"GBRG");
	ignoreIncorrectFrames.SetCheck(opts.ignore);
	useFilter.SetCheck(true);
	filterSelect.EnableWindow(useFilter.GetCheck());
	filterSelect.AddString(L"None");
	filterSelect.AddString(L"Box filter");
	filterSelect.AddString(L"Median filter");
	filterSelect.AddString(L"Gaussian filter");
	if (opts.bayer > 0)
		debayeringCode.SetCurSel(opts.bayer - 45);
	else
		debayeringCode.SetCurSel(0);
	filterSelect.SetCurSel(opts.filter.type);

	return TRUE;
}

/**************************************************************************************************
 * @fn	void PrefDialog::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog. ID - Variable
 * 			Check the .rc file and click the controls to know the correspondences.
 * 			Variable names should help somehow.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void PrefDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	//Processing visualisation
	DDX_Control(pDX, IDC_CHECK1, showROI);
	DDX_Control(pDX, IDC_CHECK2, showTrack);
	DDX_Control(pDX, IDC_CHECK3, showDif);
	DDX_Control(pDX, IDC_CHECK4, showRef);
	DDX_Control(pDX, IDC_CHECK5, showMask);
	DDX_Control(pDX, IDC_CHECK6, showThresh);
	DDX_Control(pDX, IDC_CHECK7, showSmooth);
	DDX_Control(pDX, IDC_CHECK8, showHist);
	DDX_Control(pDX, IDC_CHECK9, showResult);
	
	//Impact
	DDX_Control(pDX, IDC_EDIT1, meanValue);
	DDX_Control(pDX, IDC_SPIN1, meanValueSpin);
	DDX_Control(pDX, IDC_EDIT16, impactFrameNum);
	DDX_Control(pDX, IDC_EDIT2, impactMinTime); //not shown
	DDX_Control(pDX, IDC_SPIN2, minTimeSpin); //not shown
	DDX_Control(pDX, IDC_EDIT6, impactRadius);
	DDX_Control(pDX, IDC_SPIN6, radiusSpin);
	DDX_Control(pDX, IDC_EDIT3, impactBrightThresh);
	DDX_Control(pDX, IDC_SPIN3, brightThreshSpin);
	DDX_Control(pDX, IDC_CHECK16, applyMask);
	
	DDX_Control(pDX, IDC_CHECK12,		saveIntFramesADUdtc);
	DDX_Control(pDX, IDC_CHECK10,		NoZip);
	DDX_Control(pDX, IDC_CHECK_DEBUG,	Debug);
	DDX_Control(pDX, IDC_CHECK_CLEAN,	CleanDir);

	//ROI
	DDX_Control(pDX, IDC_EDIT12, roiSizeFactor);
	DDX_Control(pDX, IDC_SPIN12, sizeFactSpin);
	DDX_Control(pDX, IDC_EDIT13, roiSecFactor);
	DDX_Control(pDX, IDC_SPIN13, secFactSpin);
	DDX_Control(pDX, IDC_EDIT15, roiMedianBufSize);
	DDX_Control(pDX, IDC_SPIN15, medianBufSpin);

	//Other processing configuration
	DDX_Control(pDX, IDC_SPIN16, nframeSpin);
	DDX_Control(pDX, IDC_EDIT17, minimumFrames);
	DDX_Control(pDX, IDC_SPIN17, minFrameSpin);
	DDX_Control(pDX, IDC_EDIT14, histScale);
	DDX_Control(pDX, IDC_SPIN14, histoSpin);

	
	DDX_Control(pDX, IDC_COMBO2, debayeringCode);
	DDX_Control(pDX, IDC_CHECK14, ignoreIncorrectFrames);
	DDX_Control(pDX, IDC_CHECK15, useFilter);
	DDX_Control(pDX, IDC_COMBO1, filterSelect);
}

// PrefDialog message handlers

/**************************************************************************************************
 * @fn	void PrefDialog::OnBnClickedOk()
 *
 * @brief	Save button from the preference dialog. Updates the data with the received options.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialog::OnBnClickedOk()
{
	CString str;
	CString DeTeCtIniFilename = DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX);
	
	//Processing visualisation
	opts.viewROI = showROI.GetCheck();
	opts.viewTrk = showTrack.GetCheck();
	opts.viewDif = showDif.GetCheck();
	opts.viewRef = showRef.GetCheck();
	opts.viewMsk = showMask.GetCheck();
	opts.viewThr = showThresh.GetCheck();
	opts.viewSmo = showSmooth.GetCheck();
	opts.viewHis = showHist.GetCheck();
	opts.viewRes = showResult.GetCheck();
	
	//Impact
	meanValue.GetWindowTextW(str);
	opts.incrLumImpact = std::stof(str.GetString());
	impactMinTime.GetWindowTextW(str);
	opts.incrFrameImpact = std::stoi(str.GetString());
	str.Format(L"%.2f", opts.impact_duration_min);
	//opts.impact_duration_min = std::stof(str.GetString());
	impactRadius.GetWindowTextW(str);
	opts.radius = std::stod(str.GetString());
	impactBrightThresh.GetWindowTextW(str);
	opts.threshold = std::stod(str.GetString());
	opts.thrWithMask = applyMask.GetCheck();

	opts.allframes =	saveIntFramesADUdtc.GetCheck();
	opts.zip =			!NoZip.GetCheck();
	opts.debug =		Debug.GetCheck();
	opts.clean_dir = CleanDir.GetCheck();

	//ROI
	roiSizeFactor.GetWindowTextW(str);
	opts.facSize = std::stof(str.GetString());
	roiSecFactor.GetWindowTextW(str);
	opts.secSize = std::stof(str.GetString());
	roiMedianBufSize.GetWindowTextW(str);
	opts.medSize = std::stol(str.GetString());

	//Other processing configuration
	impactFrameNum.GetWindowTextW(str);
	opts.nframesRef = std::stoi(str.GetString());
	minimumFrames.GetWindowTextW(str);
	opts.minframes = std::stoi(str.GetString());
	histScale.GetWindowTextW(str);
	opts.histScale = std::stod(str.GetString());

	//int bayerCodes[] = { 0, cv::COLOR_BayerBG2RGB, cv::COLOR_BayerGB2RGB, cv::COLOR_BayerRG2RGB, cv::COLOR_BayerGR2RGB };
	opts.ignore = ignoreIncorrectFrames.GetCheck();
	opts.filter.type = filterSelect.GetCurSel();

	WriteIni();
	CDialog::OnOK();
}

//useFilter button

/**************************************************************************************************
 * @fn	void PrefDialog::OnBnClickedCheck15()
 *
 * @brief	Executes the filter check selection button, activating the list depending on the
 * 			resulting value.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialog::OnBnClickedCheck15()
{
	filterSelect.EnableWindow(useFilter.GetCheck());
}

/**************************************************************************************************
 * @fn	void PrefDialog::OnBnClickedButton1()
 *
 * @brief	Sets the default parametres of the preference dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialog::OnBnClickedButton1()
{
	//Processing visualisation
	showROI.SetCheck(0);
	showTrack.SetCheck(0);
	showDif.SetCheck(0);
	showRef.SetCheck(0);
	showMask.SetCheck(0);
	showThresh.SetCheck(0);
	showSmooth.SetCheck(0);
	showResult.SetCheck(0);
	showHist.SetCheck(0);

	//Impact
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << 0.3;
	meanValue.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	impactFrameNum.SetWindowText(std::to_wstring(50).c_str());
	ss << std::fixed << std::setprecision(2) << 5;	// not shown
	impactMinTime.SetWindowText(ss.str().c_str());	// not shown
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 10.0;
	impactRadius.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 0.0;
	impactBrightThresh.SetWindowText(ss.str().c_str());
	applyMask.SetCheck(0);

	saveIntFramesADUdtc.SetCheck(0);
	NoZip.SetCheck(0);
	Debug.SetCheck(0);
	CleanDir.SetCheck(0);

		//ROI
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 0.9;
	roiSizeFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 1.05;
	roiSecFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 5.0;
	roiMedianBufSize.SetWindowText(ss.str().c_str());
	
	//Other processing configuration
	minimumFrames.SetWindowText(std::to_wstring(15).c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 0.8;
	histScale.SetWindowText(ss.str().c_str());
	
	ignoreIncorrectFrames.SetCheck(0);
	useFilter.SetCheck(true);
	filterSelect.EnableWindow(useFilter.GetCheck());

	opts.impact_duration_min =		0.4;
	opts.ROI_min_px_val =			10;
	opts.ROI_min_size =				70;
	opts.impact_distance_max =		0.03;
	opts.impact_max_avg_min =		177.0;
	opts.impact_confidence_min =	3.0;
	opts.maxinstances =				1;
	strcpy(opts.darkfilename, "darkfile.tif");

	// Apply changes
	PrefDialog::OnBnClickedOk();
}


/**************************************************************************************************
 * @fn	void PrefDialog::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
 *
 * @brief	Executes the deltapos spin 1 action.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pNMHDR 	If non-null, the nmhdr.
 * @param [out]	  	pResult	If non-null, the result.
 **************************************************************************************************/

void PrefDialog::OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	CString str;
	meanValue.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	val += pNMUpDown->iDelta * 0.1f;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	meanValue.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**************************************************************************************************
 * @fn	void PrefDialog::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
 *
 * @brief	Executes the deltapos spin for the minimum impact time.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pNMHDR 	If non-null, the action carried in the spin.
 * @param [out]	  	pResult	If non-null, the result.
 **************************************************************************************************/

void PrefDialog::OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	impactMinTime.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	//val += pNMUpDown->iDelta * 0.1;
	val += pNMUpDown->iDelta;
	std::wstringstream ss;
	ss << val;
	impactMinTime.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**************************************************************************************************
 * @fn	void PrefDialog::OnDeltaposSpin14(NMHDR *pNMHDR, LRESULT *pResult)
 *
 * @brief	Executes the deltapos spin for the histogram scale.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pNMHDR 	If non-null, the action carried in the spin.
 * @param [out]	  	pResult	If non-null, the result.
 **************************************************************************************************/

void PrefDialog::OnDeltaposSpin14(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	histScale.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	val += pNMUpDown->iDelta * 0.1f;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	histScale.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**************************************************************************************************
 * @fn	void PrefDialog::OnDeltaposSpin12(NMHDR *pNMHDR, LRESULT *pResult)
 *
 * @brief	Executes the deltapos spin for the ROI size factor.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pNMHDR 	If non-null, the action carried in the spin.
 * @param [out]	  	pResult	If non-null, the result.
 **************************************************************************************************/

void PrefDialog::OnDeltaposSpin12(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	roiSizeFactor.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	val += pNMUpDown->iDelta * 0.05f;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	roiSizeFactor.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**************************************************************************************************
 * @fn	void PrefDialog::OnDeltaposSpin13(NMHDR *pNMHDR, LRESULT *pResult)
 *
 * @brief	Executes the deltapos spin for the ROI security factor.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pNMHDR 	If non-null, the action carried in the spin.
 * @param [out]	  	pResult	If non-null, the result.
 **************************************************************************************************/

void PrefDialog::OnDeltaposSpin13(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	CString str;
	roiSecFactor.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	val += pNMUpDown->iDelta * 0.05f;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	roiSecFactor.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}


void PrefDialog::OnCbnSelchangeCombo2()
{
	int bayerCodes[] = { 0, cv::COLOR_BayerRG2RGB, cv::COLOR_BayerGR2RGB, cv::COLOR_BayerBG2RGB, cv::COLOR_BayerGB2RGB };
	opts.bayer = bayerCodes[debayeringCode.GetCurSel()];
}

//******************************************************************************************************************************************************************************************************************************************************************************************
// PrefDialogUser dialog
//******************************************************************************************************************************************************************************************************************************************************************************************

IMPLEMENT_DYNAMIC(PrefDialogUser, CDialog)

/**
* Maps the IDS of the controls defined above to the actions which are the functions below
*/
BEGIN_MESSAGE_MAP(PrefDialogUser, CDialog)
	ON_BN_CLICKED(ID_PREFOK_USER, &PrefDialogUser::OnBnClickedOk)
	ON_BN_CLICKED(IDC_RESET_USER, &PrefDialogUser::OnBnClickedButton1)
END_MESSAGE_MAP()


/**************************************************************************************************
 * @fn	PrefDialogUser::PrefDialogUser(CWnd* pParent )
 *
 * @brief	Constructor.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pParent	If non-null, the parent.
 **************************************************************************************************/

PrefDialogUser::PrefDialogUser(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_SETTINGS_USER, pParent)
{
}

/**************************************************************************************************
 * @fn	PrefDialogUser::~PrefDialogUser()
 *
 * @brief	Destructor.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

PrefDialogUser::~PrefDialogUser()
{
}

/**************************************************************************************************
 * @fn	BOOL PrefDialogUser::OnInitDialog()
 *
 * @brief	Initializes this dialog and the controls within it. Sets the values of the controls to
 * 			the current option values. And sets the spinners to have reasonable value ranges.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @return	True if it succeeds, false if it fails.
 **************************************************************************************************/

BOOL PrefDialogUser::OnInitDialog()
{
	CDialog::OnInitDialog();

	std::wstringstream ss;

	ShowDetectImg.SetCheck(opts.show_detect_image);
	ShowMeanImg.SetCheck(opts.show_mean_image);

	Email.SetCheck(opts.email);
	Noreprocessing.SetCheck((int)(!opts.reprocessing));
	detailedADUdtc.SetCheck(opts.detail);

	datesOnly.SetCheck(opts.dateonly);
	Flat.SetCheck(opts.flat_preparation);

	return TRUE;
}

/**************************************************************************************************
 * @fn	void PrefDialogUser::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog. ID - Variable
 * 			Check the .rc file and click the controls to know the correspondences.
 * 			Variable names should help somehow.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void PrefDialogUser::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	
	DDX_Control(pDX, IDC_CHECK_DETECTION_IMG,	ShowDetectImg);
	DDX_Control(pDX, IDC_CHECK_MEAN_IMG,		ShowMeanImg);
	DDX_Control(pDX, IDC_CHECK18,				Email);
	DDX_Control(pDX, IDC_NOREPROC,				Noreprocessing);
	DDX_Control(pDX, IDC_CHECK11,				detailedADUdtc);
	DDX_Control(pDX, IDC_CHECK13,				datesOnly);
	DDX_Control(pDX, IDC_CHECK_FLAT,			Flat);
}


// PrefDialogUser message handlers

/**************************************************************************************************
 * @fn	void PrefDialogUser::OnBnClickedOk()
 *
 * @brief	Save button from the preference dialog. Updates the data with the received options.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialogUser::OnBnClickedOk()
{
	CString str;
	CString DeTeCtIniFilename = DeTeCt_additional_filename_exe_fullpath(DTC_INI_SUFFIX);

	opts.show_detect_image =	ShowDetectImg.GetCheck();
	opts.show_mean_image =		ShowMeanImg.GetCheck();

	opts.email =				Email.GetCheck();
	opts.reprocessing = !Noreprocessing.GetCheck();
	opts.detail =				detailedADUdtc.GetCheck();

	opts.dateonly =				datesOnly.GetCheck();
	opts.flat_preparation = Flat.GetCheck();

	WriteIni();
	CDialog::OnOK(); 
}


/**************************************************************************************************
 * @fn	void PrefDialogUser::OnBnClickedButton1()
 *
 * @brief	Sets the default parametres of the preference dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialogUser::OnBnClickedButton1()
{
	strcpy(opts.darkfilename, "darkfile.tif");

	//Explorer.SetCheck(1);

	ShowDetectImg.SetCheck(TRUE);
	ShowMeanImg.SetCheck(FALSE);

	Email.SetCheck(TRUE);
	Noreprocessing.SetCheck(FALSE);
	detailedADUdtc.SetCheck(FALSE);

	datesOnly.SetCheck(FALSE);
	Flat.SetCheck(FALSE);

	// Apply changes
	PrefDialogUser::OnBnClickedOk();
}

// SendEmailDlg dialog

IMPLEMENT_DYNAMIC(SendEmailDlg, CDialog)

BEGIN_MESSAGE_MAP(SendEmailDlg, CDialog)
	ON_STN_CLICKED(IDC_STATICS, &SendEmailDlg::OnStnClickedStatics)
	ON_STN_CLICKED(IDC_STATICF3, &SendEmailDlg::OnStnClickedStaticf3)
	ON_BN_CLICKED(IDC_MFCLINK1, &SendEmailDlg::OnBnClickedMfclink1)
	ON_BN_CLICKED(IDC_BUTTON1, &SendEmailDlg::OnBnClickedButton1)
	ON_STN_CLICKED(IDC_STATICF, &SendEmailDlg::OnStnClickedStaticf)
END_MESSAGE_MAP()// DeTeCt-MFCDlg.cpp : implementation file
//


//******************************************************************************************************************************************************************************************************************************************************************************************
//  SendEmailDlg dialog
//******************************************************************************************************************************************************************************************************************************************************************************************

/**************************************************************************************************
 * @fn	SendEmailDlg::SendEmailDlg(CWnd* pParent)
 *
 * @brief	Constructor.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pParent	If non-null, the parent.
 **************************************************************************************************/

SendEmailDlg::SendEmailDlg(CWnd* pParent)
	: CDialog(IDD_SENDLOGDIALOG, pParent)
{

}

SendEmailDlg::SendEmailDlg(CWnd* pParent, std::vector<std::string> logMessages2)
	: CDialog(IDD_SENDLOGDIALOG, pParent)
{
	this->messages = logMessages2;
}

/**************************************************************************************************
 * @fn	SendEmailDlg::~SendEmailDlg()
 *
 * @brief	Destructor.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

SendEmailDlg::~SendEmailDlg()
{
}

/**************************************************************************************************
 * @fn	void SendEmailDlg::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void SendEmailDlg::DoDataExchange(CDataExchange* pDX)
{
	 CDialog::DoDataExchange(pDX);
	 DDX_Control(pDX, IDC_LIST1, outputLog);
 }

BOOL SendEmailDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing) 
	BOOL bOk = m_resizer.Hook(this);
	ASSERT(bOk);

	int x_size = 944-18;
	int y_size = 430-36;
	CSize CSize_min = CSize(x_size, y_size);
	bOk = m_resizer.SetMinimumSize(_T("_root"), CSize_min);
	ASSERT(bOk);

	m_resizer.SetShowResizeGrip(TRUE);
	bOk = m_resizer.InvokeOnResized();
	ASSERT(bOk);

	for (std::string msg : messages) {
		std::wstring wmsg = std::wstring(msg.begin(), msg.end());
		CString Cmsg = CString(wmsg.c_str(), (int)wmsg.length());
		outputLog.AddString(Cmsg);
	}
	return TRUE;

}

void SendEmailDlg::OnBnClickedButton1()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
	//extern char impact_detection_dirname[MAX_STRING];
	//extern char zip_detection_location[MAX_STRING];
	//extern char zipfile[MAX_STRING];
	//extern char log_detection_dirname[MAX_STRING];

	wchar_t	wimpact_detection_dirname[MAX_STRING];
	size_t ReturnValue;
	mbstowcs_s(&ReturnValue, wimpact_detection_dirname, strlen(impact_detection_dirname) + 1, impact_detection_dirname, strlen(impact_detection_dirname));
	// email start
	char	mailto_command[MAX_STRING]	= { 0 };
	wchar_t	wmailto_command[MAX_STRING] = { 0 };
	if (opts.email) {
		strcpy(mailto_command, "mailto:delcroix.marc@free.fr?subject=Impact detection ");
		strcat_s(mailto_command, sizeof(mailto_command), log_detection_dirname);
		strcat(mailto_command, "&body=(*please attach ");
	}

	// Zip post-processing
	if (strlen(zip_detection_location) > 0) {
		wchar_t wzip_detection_location[MAX_STRING];
		mbstowcs(wzip_detection_location, zip_detection_location, strlen(zip_detection_location) + 1);//Plus null
		ShellExecute(NULL, L"explore", wzip_detection_location, NULL, NULL, SW_SHOWNORMAL);

		if (opts.email) {
			// email continued with zip file
			strcat(mailto_command, "impact_detection zip file ");
			strcat_s(mailto_command, sizeof(mailto_command), zipfile);
		}
	}
	else {
		if (opts.email) {
			// email continued with detection log and images
			strcat(mailto_command, "detection images and detect log file from ");
			strcat_s(mailto_command, sizeof(mailto_command), impact_detection_dirname);
		}
	}
	//E-Mail post-processing
	if (opts.email) {
		// email end
		strcat(mailto_command, " *)%0A%0AHi Marc,%0A%0AHere are the results and the images of my analysis, I checked the images and found: %0A*please check: (X)*%0A ( ) nothing suspect%0A ( ) something suspect like an impact%0A%0ACheers,%0A");
		mbstowcs(wmailto_command, mailto_command, strlen(mailto_command) + 1);
		ShellExecute(NULL, L"open", wmailto_command, NULL, NULL, SW_SHOWNORMAL);
	}

	//Explorer post-processing
	ShellExecute(NULL, L"explore", wimpact_detection_dirname, NULL, NULL, SW_SHOWNORMAL);
	//SendMail(zip_detection_dirname);
}


void SendEmailDlg::OnBnClickedMfclink1()
{
	ShellExecute(NULL, L"open", L"http://www.astrosurf.com/planetessaf/doc/project_detect.shtml", NULL, NULL, SW_SHOWNORMAL);
}


//******************************************************************************************************************************************************************************************************************************************************************************************
// ProgressDialog dialog
//******************************************************************************************************************************************************************************************************************************************************************************************

IMPLEMENT_DYNAMIC(ProgressDialog, CDialog)

BEGIN_MESSAGE_MAP(ProgressDialog, CDialog)
END_MESSAGE_MAP()

/**************************************************************************************************
 * @fn	ProgressDialog::ProgressDialog(CWnd* pParent )
 *
 * @brief	Constructor for the progress dialog - UNUSED
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pParent	If non-null, the parent.
 **************************************************************************************************/
ProgressDialog::ProgressDialog(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_PROGRESSDIALOG, pParent)
{
	MSG msg;
	while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

/**************************************************************************************************
 * @fn	ProgressDialog::~ProgressDialog()
 *
 * @brief	Destructor.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

ProgressDialog::~ProgressDialog()
{
}

/**************************************************************************************************
 * @fn	void ProgressDialog::DoDataExchange(CDataExchange* pDX)
 *
 * @brief	Exchanges data to/from the controls in this dialog. -- UNUSED
 *
 * @author	Jon
 * @date	2017-05-12
 *
 * @param [in,out]	pDX	If non-null, an object that manages the data exchange operation.
 **************************************************************************************************/

void ProgressDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_PROGRESS1, dtcProgress);
	DDX_Control(pDX, IDC_PROGRESS2, dtcProgress_all);
	DDX_Control(pDX, IDC_STATICI, progressInfo);
}



/**********************************************************************************************/
 /*	Action of button to check detection image folder and send zip file                        */
/**********************************************************************************************/

/**************************************************************************************************
 * @fn	void CDeTeCtMFCDlg::OnLbnSelchangeList1()
 *
 * @brief	Executes the lbn selchange list 1 action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnLbnSelchangeList1()
{
}

void CDeTeCtMFCDlg::OnBnClickedFrame()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}

void SendEmailDlg::OnStnClickedStatics()
{
	// TODO: Add your control notification handler code here
}

void SendEmailDlg::OnStnClickedStaticf3()
{
	// TODO: Add your control notification handler code here
}

void SendEmailDlg::OnStnClickedStaticf4()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}

void SendEmailDlg::OnStnClickedStaticf()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}

void CDeTeCtMFCDlg::OnBnClickedOk3()
{
	OnFileOpenFolder();
}


void CDeTeCtMFCDlg::OnStnClickedStaticproba()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}


void CDeTeCtMFCDlg::OnBnClickedOk2()
{
	OnFileOpenfile();
}

void CDeTeCtMFCDlg::OnBnClickedCheckAuto()
{
	if (opts.interactive) {
		opts.interactive = FALSE;
	}
	else {
		opts.interactive = TRUE;
	}
	CDeTeCtMFCDlg::getAuto()->SetCheck((int) (!opts.interactive));
}

void CDeTeCtMFCDlg::OnBnClickedCheckExit()
{
	if (opts.autoexit) {
		opts.autoexit = FALSE;
	}
	else {
		opts.autoexit = TRUE;
	}
	CDeTeCtMFCDlg::getExit()->SetCheck(opts.autoexit);
}


void CDeTeCtMFCDlg::OnBnClickedCheckShutdown()
{
	if (opts.shutdown) {
		opts.shutdown = FALSE;
	}
	else {
		opts.shutdown = TRUE;
	}
	CDeTeCtMFCDlg::getShutdown()->SetCheck(opts.shutdown);
}


void CDeTeCtMFCDlg::OnDeltaposSpinInstances(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);

	int processor_count = std::thread::hardware_concurrency();
	opts.maxinstances += pNMUpDown->iDelta;
	ValueMaxInstances.SetPos(opts.maxinstances);
	if (opts.maxinstances > processor_count)	opts.maxinstances = processor_count;
	else if (opts.maxinstances < 1)				opts.maxinstances = 1;
	else										if (filesys::exists(CString2string((CString)opts.DeTeCtQueueFilename))) SetIntParamToQueue(opts.maxinstances, _T("max_instances"), (CString)opts.DeTeCtQueueFilename);
	//DisplayInstanceType(); //too long (~1s) due to count of child process number
	CDeTeCtMFCDlg::getMaxInstances()->SetWindowText(std::to_wstring(opts.maxinstances).c_str() + (CString)"/" + std::to_wstring(processor_count).c_str());
	*pResult = 0;
}