
// DeTeCt-MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "afxdialogex.h"

#include "dtcgui.hpp"
#include "DetectThread.h"
#include "cmdline.h"
#include <thread>
#include <string>

#ifdef _DEBUG

/**********************************************************************************************//**
 * @def	new
 *
 * @brief	A macro that defines new.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

#define new DEBUG_NEW
#endif

CListBox CDeTeCtMFCDlg::impactDetectionLog;
CProgressCtrl CDeTeCtMFCDlg::progressBar;
CListBox SendEmailDlg::outputLog;
std::vector<LPCTSTR> SendEmailDlg::logMessages;

CStatic c_Frame;

// CAboutDlg dialog used for App About

/**********************************************************************************************//**
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

	/**********************************************************************************************//**
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

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	ON_UPDATE_COMMAND_UI(ID_EXIT_QUIT, &CAboutDlg::OnUpdateExitQuit)
	ON_BN_CLICKED(IDC_MFCLINK1, &CAboutDlg::OnBnClickedMfclink1)
	ON_BN_CLICKED(IDC_MFCLINK2, &CAboutDlg::OnBnClickedMfclink2)
END_MESSAGE_MAP()


// CDeTeCtMFCDlg dialog

/**********************************************************************************************//**
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
	wchar_t exepath[1000];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CString folder = exepath;
	folder = folder.Left(folder.ReverseFind(_T('\\')) + 1);
	folder.Append(L"options.ini");
	_TCHAR optionStr[1000];
	opts.filename = opts.ofilename = opts.darkfilename = opts.ovfname = opts.sfname = NULL;
	opts.nsaveframe = 0;
	opts.ostype = OTYPE_NO;
	opts.ovtype = OTYPE_NO;
	::GetPrivateProfileString(L"impact", L"min_strength", L"0.3", optionStr, sizeof(optionStr) / sizeof(optionStr[0]),
		folder);
	opts.timeImpact = std::stod(optionStr);
	opts.incrLumImpact = std::stod(optionStr);
	opts.incrFrameImpact = ::GetPrivateProfileInt(L"impact", L"frames", 5, folder);
	opts.radius = ::GetPrivateProfileInt(L"impact", L"radius", 10, folder);
	opts.nframesROI = 1;
	opts.nframesRef = ::GetPrivateProfileInt(L"other", L"refmin", 50, folder);
	opts.bayer = ::GetPrivateProfileInt(L"other", L"debayer", 0, folder);
	opts.medSize = ::GetPrivateProfileInt(L"roi", L"medbuf", 5, folder);
	opts.wait = 1;
	::GetPrivateProfileString(L"roi", L"sizfac", L"0.90", optionStr, sizeof(optionStr) / sizeof(optionStr[0]),
		folder);
	opts.facSize = std::stod(optionStr);
	::GetPrivateProfileString(L"roi", L"secfac", L"1.05", optionStr, sizeof(optionStr) / sizeof(optionStr[0]),
		folder);
	opts.secSize = std::stod(optionStr);
	opts.threshold = ::GetPrivateProfileInt(L"impact", L"thresh", 0, folder);
	opts.learningRate = 0.0;
	opts.thrWithMask = ::GetPrivateProfileInt(L"impact", L"mask", 0, folder);
	opts.histScale = 1;
	opts.viewROI = ::GetPrivateProfileInt(L"view", L"roi", 0, folder);
	opts.viewTrk = ::GetPrivateProfileInt(L"view", L"trk", 0, folder);
	opts.viewDif = ::GetPrivateProfileInt(L"view", L"dif", 0, folder);
	opts.viewRef = ::GetPrivateProfileInt(L"view", L"ref", 0, folder);
	opts.viewThr = ::GetPrivateProfileInt(L"view", L"thr", 0, folder);
	opts.viewSmo = ::GetPrivateProfileInt(L"view", L"smo", 0, folder);
	opts.viewRes = ::GetPrivateProfileInt(L"view", L"res", 0, folder);
	opts.viewHis = ::GetPrivateProfileInt(L"view", L"his", 0, folder);
	opts.viewMsk = ::GetPrivateProfileInt(L"view", L"msk", 0, folder);
	opts.verbose = 0;
	opts.filter.type = ::GetPrivateProfileInt(L"other", L"filter", 1, folder);
	opts.filter.param[0] = 3;
	opts.filter.param[1] = 3;
	opts.filter.param[2] = 0;
	opts.filter.param[3] = 0;
	opts.debug = 0;
	opts.ADUdtconly = 0;
	opts.detail = ::GetPrivateProfileInt(L"impact", L"detail", 0, folder);
	opts.allframes = ::GetPrivateProfileInt(L"impact", L"inter", 0, folder);
	opts.minframes = ::GetPrivateProfileInt(L"other", L"frmin", 0, folder);
	opts.dateonly = ::GetPrivateProfileInt(L"other", L"dateonly", 0, folder);
	opts.ignore = ::GetPrivateProfileInt(L"other", L"ignore", 0, folder);
	opts.videotest = 0;
	opts.wROI = 0;
	opts.hROI = 0;
	//AFX_MANAGE_STATE(AfxGetStaticModuleState());
	//AFX_MANAGE_STATE(AFX_MODULE_STATE* pModuleState);
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

}

/**********************************************************************************************//**
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
}

/*
* Maps controls to functions
*/
BEGIN_MESSAGE_MAP(CDeTeCtMFCDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDeTeCtMFCDlg::OnBnClickedOk)
	ON_COMMAND(ID_FILE_OPEN32771, &CDeTeCtMFCDlg::OnFileOpen32771)
	ON_COMMAND(ID_HELP_EXIT, &CDeTeCtMFCDlg::OnHelpExit)
	ON_COMMAND(ID_SETTINGS_PREFERENCES, &CDeTeCtMFCDlg::OnSettingsPreferences)
	ON_COMMAND(ID_FILE_EXIT, &CDeTeCtMFCDlg::OnFileExit)
	ON_LBN_SELCHANGE(IDC_LIST1, &CDeTeCtMFCDlg::OnLbnSelchangeList1)
	ON_COMMAND(ID_FILE_OPENFILE, &CDeTeCtMFCDlg::OnFileOpenfile)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_FRAME, &CDeTeCtMFCDlg::OnBnClickedFrame)
END_MESSAGE_MAP()


// CDeTeCtMFCDlg message handlers

/**********************************************************************************************//**
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
	CDialog::OnInitDialog();

	//Test WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
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
	std::wstring wstr(full_version.begin(), full_version.end());
	SetWindowText(wstr.c_str());

// Following does not work
//	ModifyStyle(0, WS_MAXIMIZEBOX, SWP_FRAMECHANGED);  // enable maximize
//	ModifyStyle(0, WS_MINIMIZEBOX, SWP_FRAMECHANGED);  // enable minimize
//	ModifyStyle(1, WS_MAXIMIZEBOX);


//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing) 
  bOk = m_resizer.Hook(this);
  ASSERT( bOk );

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
  x_size = 441 + 221;
  y_size = 267 + 168;
  bOk = m_resizer.SetMinimumSize(_T("_root"), CSize(x_size, y_size));
   ASSERT(bOk);

//   bOk = m_resizer.SetMaximumSize(_T("_root"), CSize(700, 700));
//   ASSERT(bOk);

//   m_resizer.SetShowResizeGrip(TRUE);
   bOk = m_resizer.InvokeOnResized();
   ASSERT(bOk);

//end

	CenterWindow();

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
	std::wstringstream ss2,ss3;
	StreamDeTeCtOSversions(&ss2);
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
	if (!opts.interactive) {
		ss3 << "Automatic mode on";
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss3.str().c_str());
	}

	//Call directly file/directory addition if option passed to exe
	if (opts.dirname > 0) OnFileOpen32771();
	else if (opts.filename > 0) OnFileOpenfile();

	return TRUE;  // return TRUE  unless you set the focus to a control
}

/**********************************************************************************************//**
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
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

/**********************************************************************************************//**
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
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.

/**********************************************************************************************//**
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

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnBnClickedOk()
 *
 * @brief	Executes the button clicked ok action -- Runs the algorithm in an independent thread
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnBnClickedOk()
{
	if (file_list.size() > 0) {
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"Running analysis");
		
		/* C++ standard threading */
		/*
		std::thread detection_thread(detect, file_list, opts, scan_folder_path);
		detection_thread.detach();
		*/
		
		/* MFC native threading, check DetechThread.h/cpp for details */
		ImpactDetectParams* params = new ImpactDetectParams(file_list, opts, scan_folder_path);
		AfxBeginThread(impactDetection, (LPVOID) params);
		
	} else {
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + L"Error: no files selected");
	}
}

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnFileOpen32771()
 *
 * @brief	Executes the multiple file open action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileOpen32771()
{
	file_list = {};
	acquisition_file_list = {};
	CFolderPickerDialog dialog(NULL, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, this, sizeof(OPENFILENAME));
	std::wstring folder_path;
	std::string path;
	std::wstringstream ss;

	if (opts.dirname > 0) {
		path = std::string(opts.dirname);
	}
	else {
		if (dialog.DoModal() == IDOK) {
			folder_path = std::wstring(dialog.GetPathName().GetString());
			path = std::string(folder_path.begin(), folder_path.end());
		}
	}
	if (path.size() <= 0) {
		ss << "No file selected";
	} else {
		std::wstringstream ss2;

		scan_folder_path = path;
//TODO: clearscreen
		ss2 << "Resetting file list and scanning " << folder_path << " for files to be analysed...";
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());
		read_files(path, &file_list, &acquisition_file_list);
		for (std::string filename : file_list) {
			std::wstringstream ss;
			bool fileexists = FALSE;
			std::string filename_acquisition;

			std::string extension = filename.substr(filename.find_last_of(".") + 1, filename.size() - filename.find_last_of(".") - 1);
			if (extension.compare(autostakkert_extension) == 0) {
				std::vector<cv::Point> cm_list;

				read_autostakkert_file(filename, &filename_acquisition, &cm_list);
				std::ifstream filetest(filename_acquisition);
				if (filetest) fileexists = TRUE;
				filename_acquisition = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());
				//TODO: test if filename_acquisition is already in the list and remove it
			}
			filename = filename.substr(filename.find_last_of("\\") + 1, filename.length());
			if ((extension.compare(autostakkert_extension) != 0) || (fileexists)) {
				if (extension.compare(autostakkert_extension) != 0) ss << "Adding " << filename.c_str() << " for analysis";
				else ss << "Adding " << filename.c_str() << " (" << filename_acquisition.c_str() << " acquisition file) for analysis";
			}
			else if ((extension.compare(autostakkert_extension) == 0) && (!fileexists)) {
				ss << "Ignoring " << filename.c_str() << " (acquisition file " << filename_acquisition.c_str() << " is missing)";
			}
			impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());
		}
		//		if (file_list.size() > 0) {
		//			max_mean_folder_path = folder_path.append(L"\\Impact_detection");
		//			if (GetFileAttributes(max_mean_folder_path.c_str()) == INVALID_FILE_ATTRIBUTES) CreateDirectory(max_mean_folder_path.c_str(), 0);
		//		}
	}
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());
	this->RedrawWindow();
	if (opts.dirname > 0) opts.dirname = NULL;
	if (!opts.interactive) OnBnClickedOk();
}

/**********************************************************************************************//**
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
	this->CloseWindow();
}

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnHelpExit()
 *
 * @brief	Executes the exit action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnHelpExit()
{
	CAboutDlg *about = new CAboutDlg();
	about->DoModal();
}

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnSettingsPreferences()
 *
 * @brief	Executes the settings -> preferences action. Opens the preferences window
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnSettingsPreferences()
{
	PrefDialog *preferences = new PrefDialog();
	preferences->DoModal();
}
// C:\Users\Jon\Documents\Visual Studio 2015\Projects\DeTeCt-MFC\DeTeCt-MFC\DeTeCt-MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "afxdialogex.h"


// PrefDialog dialog

IMPLEMENT_DYNAMIC(PrefDialog, CDialog)

/**********************************************************************************************//**
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
	: CDialog(IDD_PREFERENCES, pParent)
{
}

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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
	meanValueSpin.SetBuddy(&meanValue);
	meanValueSpin.SetRange(0.0, 1.0);
	minTimeSpin.SetBuddy(&impactMinTime);
	minTimeSpin.SetRange(0.0, 1.0);
	radiusSpin.SetBuddy(&impactRadius);
	radiusSpin.SetRange(5.0, 20.0);
	brightThreshSpin.SetBuddy(&impactBrightThresh);
	brightThreshSpin.SetRange(0.0, 255.0);
	sizeFactSpin.SetBuddy(&roiSizeFactor);
	sizeFactSpin.SetRange(0.0, 2.0);
	secFactSpin.SetBuddy(&roiSecFactor);
	secFactSpin.SetRange(0.0, 1.0);
	medianBufSpin.SetBuddy(&roiMedianBufSize);
	medianBufSpin.SetRange(5.0, 50.0);
	nframeSpin.SetBuddy(&impactFrameNum);
	nframeSpin.SetRange(1.0, 50.0);
	minFrameSpin.SetBuddy(&minimumFrames);
	minFrameSpin.SetRange(3.0, 10000.0);
	histoSpin.SetBuddy(&histScale);
	histoSpin.SetRange(0.0, 1.0);
	std::wstringstream ss;
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
	roiSizeFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.secSize;
	roiSecFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << opts.medSize;
	roiMedianBufSize.SetWindowText(ss.str().c_str());
	minimumFrames.SetWindowText(std::to_wstring(opts.minframes).c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << opts.histScale;
	histScale.SetWindowText(ss.str().c_str());
	applyMask.SetCheck(opts.thrWithMask);
	//ADUdtconly.SetCheck(opts.ADUdtconly);
	//detailedADUdtc.EnableWindow(opts.ADUdtconly);
	//saveIntFramesADUdtc.EnableWindow(opts.ADUdtconly);
	detailedADUdtc.SetCheck(opts.detail);
	saveIntFramesADUdtc.SetCheck(opts.allframes);
	showROI.SetCheck(opts.viewROI);
	showTrack.SetCheck(opts.viewTrk);
	showDif.SetCheck(opts.viewDif);
	showRef.SetCheck(opts.viewRef);
	showMask.SetCheck(opts.viewMsk);
	showThresh.SetCheck(opts.viewThr);
	showSmooth.SetCheck(opts.viewSmo);
	showResult.SetCheck(opts.viewRes);
	showHist.SetCheck(opts.viewHis);
	datesOnly.SetCheck(opts.dateonly);
	ignoreIncorrectFrames.SetCheck(opts.ignore);
	useFilter.SetCheck(true);
	filterSelect.EnableWindow(useFilter.GetCheck());
	filterSelect.AddString(L"None");
	filterSelect.AddString(L"Box filter");
	filterSelect.AddString(L"Median filter");
	filterSelect.AddString(L"Gaussian filter");
	debayeringCode.AddString(L"None");
	debayeringCode.AddString(L"RGGB");
	debayeringCode.AddString(L"GRBG");
	debayeringCode.AddString(L"BGGR");
	debayeringCode.AddString(L"GBRG");
	if (opts.bayer > 0)
		debayeringCode.SetCurSel(opts.bayer - 45);
	else 
		debayeringCode.SetCurSel(0);
	filterSelect.SetCurSel(opts.filter.type);
	return TRUE;
}

/**********************************************************************************************//**
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
	DDX_Control(pDX, IDC_EDIT1, meanValue);
	DDX_Control(pDX, IDC_EDIT16, impactFrameNum);
	DDX_Control(pDX, IDC_EDIT2, impactMinTime);
	DDX_Control(pDX, IDC_EDIT6, impactRadius);
	DDX_Control(pDX, IDC_EDIT3, impactBrightThresh);
	DDX_Control(pDX, IDC_EDIT12, roiSizeFactor);
	DDX_Control(pDX, IDC_EDIT13, roiSecFactor);
	DDX_Control(pDX, IDC_EDIT15, roiMedianBufSize);
	DDX_Control(pDX, IDC_EDIT14, histScale);
	DDX_Control(pDX, IDC_CHECK16, applyMask);
	DDX_Control(pDX, IDC_CHECK11, detailedADUdtc);
	DDX_Control(pDX, IDC_CHECK12, saveIntFramesADUdtc);
	DDX_Control(pDX, IDC_CHECK1, showROI);
	DDX_Control(pDX, IDC_CHECK2, showTrack);
	DDX_Control(pDX, IDC_CHECK3, showDif);
	DDX_Control(pDX, IDC_CHECK4, showRef);
	DDX_Control(pDX, IDC_CHECK5, showMask);
	DDX_Control(pDX, IDC_CHECK6, showThresh);
	DDX_Control(pDX, IDC_CHECK7, showSmooth);
	DDX_Control(pDX, IDC_CHECK8, showHist);
	DDX_Control(pDX, IDC_CHECK9, showResult);
	DDX_Control(pDX, IDC_CHECK13, datesOnly);
	DDX_Control(pDX, IDC_CHECK14, ignoreIncorrectFrames);
	DDX_Control(pDX, IDC_CHECK15, useFilter);
	DDX_Control(pDX, IDC_COMBO1, filterSelect);
	DDX_Control(pDX, IDC_EDIT17, minimumFrames);
	DDX_Control(pDX, IDC_SPIN1, meanValueSpin);
	DDX_Control(pDX, IDC_SPIN2, minTimeSpin);
	DDX_Control(pDX, IDC_SPIN6, radiusSpin);
	DDX_Control(pDX, IDC_SPIN3, brightThreshSpin);
	DDX_Control(pDX, IDC_SPIN12, sizeFactSpin);
	DDX_Control(pDX, IDC_SPIN13, secFactSpin);
	DDX_Control(pDX, IDC_SPIN15, medianBufSpin);
	DDX_Control(pDX, IDC_SPIN16, nframeSpin);
	DDX_Control(pDX, IDC_SPIN17, minFrameSpin);
	DDX_Control(pDX, IDC_SPIN14, histoSpin);
	DDX_Control(pDX, IDC_COMBO2, debayeringCode);
}


/**
* Maps the IDS of the controls defined above to the actions which are the functions below
*/
BEGIN_MESSAGE_MAP(PrefDialog, CDialog)
	ON_BN_CLICKED(IDOK, &PrefDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK15, &PrefDialog::OnBnClickedCheck15)
	ON_BN_CLICKED(IDC_BUTTON1, &PrefDialog::OnBnClickedButton1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN1, &PrefDialog::OnDeltaposSpin1)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN2, &PrefDialog::OnDeltaposSpin2)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN14, &PrefDialog::OnDeltaposSpin14)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN12, &PrefDialog::OnDeltaposSpin12)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN13, &PrefDialog::OnDeltaposSpin13)
	ON_CBN_SELCHANGE(IDC_COMBO2, &PrefDialog::OnCbnSelchangeCombo2)
END_MESSAGE_MAP()


// PrefDialog message handlers

/**********************************************************************************************//**
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
	wchar_t exepath[1000];
	GetModuleFileName(NULL, exepath, MAX_PATH);
	CString folder = exepath;
	folder = folder.Left(folder.ReverseFind(_T('\\')) + 1);
	folder.Append(L"options.ini");
	meanValue.GetWindowTextW(str);
	opts.incrLumImpact = std::stof(str.GetString());
	::WritePrivateProfileString(L"impact", L"min_strength", str, folder);
	str.Empty();
	impactFrameNum.GetWindowTextW(str);
	opts.nframesRef = std::stoi(str.GetString());
	::WritePrivateProfileString(L"other", L"refmin", str, folder);
	str.Empty();
	impactMinTime.GetWindowTextW(str);
	opts.incrFrameImpact = std::stof(str.GetString());
	::WritePrivateProfileString(L"impact", L"frames", str, folder);
	str.Empty();
	impactRadius.GetWindowTextW(str);
	opts.radius = std::stod(str.GetString());
	::WritePrivateProfileString(L"impact", L"radius", str, folder);
	str.Empty();
	impactBrightThresh.GetWindowTextW(str);
	opts.threshold = std::stod(str.GetString());
	::WritePrivateProfileString(L"impact", L"thresh", str, folder);
	str.Empty();
	roiSizeFactor.GetWindowTextW(str);
	opts.facSize = std::stof(str.GetString());
	::WritePrivateProfileString(L"roi", L"sizfac", str, folder);
	str.Empty();
	roiSecFactor.GetWindowTextW(str);
	opts.secSize = std::stof(str.GetString());
	::WritePrivateProfileString(L"roi", L"secfac", str, folder);
	str.Empty();
	roiMedianBufSize.GetWindowTextW(str);
	opts.medSize = std::stol(str.GetString());
	::WritePrivateProfileString(L"roi", L"medbuf", str, folder);
	str.Empty();
	minimumFrames.GetWindowTextW(str);
	opts.minframes = std::stoi(str.GetString());
	::WritePrivateProfileString(L"other", L"frmin", str, folder);
	str.Empty();
	histScale.GetWindowTextW(str);
	opts.histScale = std::stod(str.GetString());
	::WritePrivateProfileString(L"other", L"histscale", str, folder);
	str.Empty();
	opts.thrWithMask = applyMask.GetCheck();
	str.Format(L"%d", opts.thrWithMask);
	::WritePrivateProfileString(L"impact", L"mask", str, folder);
	opts.detail = detailedADUdtc.GetCheck();
	str.Format(L"%d", opts.detail);
	::WritePrivateProfileString(L"impact", L"detail", str, folder);
	opts.allframes = saveIntFramesADUdtc.GetCheck();
	str.Format(L"%d", opts.allframes);
	::WritePrivateProfileString(L"impact", L"inter", str, folder);
	opts.viewROI = showROI.GetCheck();
	str.Format(L"%d", opts.viewROI);
	::WritePrivateProfileString(L"view", L"roi", str, folder);
	opts.viewTrk = showTrack.GetCheck();
	str.Format(L"%d", opts.viewTrk);
	::WritePrivateProfileString(L"view", L"trk", str, folder);
	opts.viewRef = showRef.GetCheck();
	str.Format(L"%d", opts.viewRef);
	::WritePrivateProfileString(L"view", L"ref", str, folder);
	opts.viewMsk = showMask.GetCheck();
	str.Format(L"%d", opts.viewMsk);
	::WritePrivateProfileString(L"view", L"msk", str, folder);
	opts.viewThr = showThresh.GetCheck();
	str.Format(L"%d", opts.viewThr);
	::WritePrivateProfileString(L"view", L"thr", str, folder);
	opts.viewSmo = showSmooth.GetCheck();
	str.Format(L"%d", opts.viewSmo);
	::WritePrivateProfileString(L"view", L"smo", str, folder);
	opts.viewRes = showResult.GetCheck();
	str.Format(L"%d", opts.viewRes);
	::WritePrivateProfileString(L"view", L"res", str, folder);
	opts.viewDif = showDif.GetCheck();
	str.Format(L"%d", opts.viewDif);
	::WritePrivateProfileString(L"view", L"dif", str, folder);
	opts.viewHis = showHist.GetCheck();
	str.Format(L"%d", opts.viewHis);
	::WritePrivateProfileString(L"view", L"his", str, folder);
	opts.dateonly = datesOnly.GetCheck();
	str.Format(L"%d", opts.dateonly);
	::WritePrivateProfileString(L"other", L"dateonly", str, folder);
	opts.ignore = ignoreIncorrectFrames.GetCheck();
	str.Format(L"%d", opts.ignore);
	::WritePrivateProfileString(L"other", L"ignore", str, folder);
	//int bayerCodes[] = { 0, cv::COLOR_BayerBG2RGB, cv::COLOR_BayerGB2RGB, cv::COLOR_BayerRG2RGB, cv::COLOR_BayerGR2RGB };
	str.Format(L"%d", opts.bayer);
	::WritePrivateProfileString(L"other", L"debayer", str, folder);
	opts.filter.type = filterSelect.GetCurSel();
	str.Format(L"%d", opts.filter.type);
	::WritePrivateProfileString(L"other", L"filter", str, folder);
	CDialog::OnOK();
}

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnFileExit()
 *
 * @brief	Executes the exit action.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileExit()
{
	CDialog::OnOK();
}

/**********************************************************************************************//**
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


//useFilter button

/**********************************************************************************************//**
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

/**********************************************************************************************//**
 * @fn	void CAboutDlg::OnBnClickedMfclink1()
 *
 * @brief	Executes the action performed after the DeTeCt project link is clicked.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CAboutDlg::OnBnClickedMfclink1()
{
	ShellExecute(
		NULL,
		L"open",
		L"http://www.astrosurf.com/planetessaf/doc/project_detect.shtml",
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}

/**********************************************************************************************//**
 * @fn	void CDeTeCtMFCDlg::OnFileOpenfile()
 *
 * @brief	Executes the file open dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void CDeTeCtMFCDlg::OnFileOpenfile()
{
	file_list = {};
	std::wstringstream ss;
	std::wstring file_path;
	std::string file;
	CFileDialog dialog(true, NULL, NULL, OFN_FILEMUSTEXIST | OFN_ENABLESIZING, filter, this, sizeof(OPENFILENAME), true);

	if (opts.filename > 0) {
		file = std::string(opts.filename);
	} else {
		if (dialog.DoModal() == IDOK) {
			file_path = std::wstring(dialog.GetPathName().GetString());
			file = std::string(file_path.begin(), file_path.end());
		}
	}
	if (file.size() <=0) {
			ss << "No file selected";
	} else {
		std::wstringstream ss2;
//TODO: clearscreen
		ss2 << "Resetting file list for analysis";
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss2.str().c_str());

		std::wstringstream ss;
		bool fileexists = FALSE;
		std::string filename_acquisition;

		std::string extension = file.substr(file.find_last_of(".") + 1, file.size() - file.find_last_of(".") - 1);
		if (extension.compare(autostakkert_extension) == 0) {
			std::vector<cv::Point> cm_list;

			read_autostakkert_file(file, &filename_acquisition, &cm_list);
			std::ifstream filetest(filename_acquisition);
			if (filetest) fileexists = TRUE;
			filename_acquisition = filename_acquisition.substr(filename_acquisition.find_last_of("\\") + 1, filename_acquisition.length());
		}
		if ((extension.compare(autostakkert_extension) != 0) || (fileexists)) {
			scan_folder_path = file.substr(0, file.find_last_of("\\"));
			file_list.push_back(file);
			file = file.substr(file.find_last_of("\\") + 1, file.length());
			if (extension.compare(autostakkert_extension) != 0) ss << "Adding " << file.c_str() << " for analysis";
			else ss << "Adding " << file.c_str() << " (" << filename_acquisition.c_str() << " acquisition file) for analysis";
		}
		else if ((extension.compare(autostakkert_extension) == 0) && (!fileexists)) {
			file = file.substr(file.find_last_of("\\") + 1, file.length());
			ss << "Ignoring " << file.c_str() << " (acquisition file " << filename_acquisition.c_str() << " is missing)";
		}
		impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());


//		if (opts.filename > 0) {
//			ss << "Adding automatically " << file.c_str() << " for analysis";
//		}
//		else {
//			ss << "Adding " << file.c_str() << " for analysis";
//		}
	}
	impactDetectionLog.AddString((CString)getDateTime().str().c_str() + ss.str().c_str());
	this->RedrawWindow();
	if (opts.filename > 0) opts.filename = NULL;
	if (!opts.interactive) OnBnClickedOk();
}

/**********************************************************************************************//**
 * @fn	void PrefDialog::OnBnClickedButton1()
 *
 * @brief	Sets the default parametres of the preference dialog.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

void PrefDialog::OnBnClickedButton1()
{
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << 0.3;
	meanValue.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	impactFrameNum.SetWindowText(std::to_wstring(50).c_str());
	ss << std::fixed << std::setprecision(2) << 5;
	impactMinTime.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 10.0;
	impactRadius.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 0.0;
	impactBrightThresh.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 0.9;
	roiSizeFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 1.05;
	roiSecFactor.SetWindowText(ss.str().c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(0) << 5.0;
	roiMedianBufSize.SetWindowText(ss.str().c_str());
	minimumFrames.SetWindowText(std::to_wstring(15).c_str());
	ss.str(std::wstring());
	ss << std::fixed << std::setprecision(2) << 0.8;
	histScale.SetWindowText(ss.str().c_str());
	applyMask.SetCheck(0);
	detailedADUdtc.SetCheck(0);
	saveIntFramesADUdtc.SetCheck(0);
	showROI.SetCheck(0);
	showTrack.SetCheck(0);
	showDif.SetCheck(0);
	showRef.SetCheck(0);
	showMask.SetCheck(0);
	showThresh.SetCheck(0);
	showSmooth.SetCheck(0);
	showResult.SetCheck(0);
	showHist.SetCheck(0);
	datesOnly.SetCheck(0);
	ignoreIncorrectFrames.SetCheck(0);
	useFilter.SetCheck(true);
	filterSelect.EnableWindow(useFilter.GetCheck());
	// TODO: Add your control notification handler code here
}
// C:\Users\Jon\Documents\Visual Studio 2015\Projects\DeTeCt-MFC\DeTeCt-MFC\DeTeCt-MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTeCt-MFCDlg.h"
#include "afxdialogex.h"


// SendEmailDlg dialog

IMPLEMENT_DYNAMIC(SendEmailDlg, CDialog)

/**********************************************************************************************//**
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

SendEmailDlg::SendEmailDlg(CWnd* pParent, std::vector<std::string> logMessages)
	: CDialog(IDD_SENDLOGDIALOG, pParent)
{
	this->messages = logMessages;
}

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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
	for (std::string msg : messages) {
		std::wstring wmsg = std::wstring(msg.begin(), msg.end());
		CString Cmsg = CString(wmsg.c_str(), wmsg.length());
		outputLog.AddString(Cmsg);
	}
	return TRUE;
}


BEGIN_MESSAGE_MAP(SendEmailDlg, CDialog)
	ON_STN_CLICKED(IDC_STATICS, &SendEmailDlg::OnStnClickedStatics)
	ON_STN_CLICKED(IDC_STATICF3, &SendEmailDlg::OnStnClickedStaticf3)
	ON_BN_CLICKED(IDC_MFCLINK1, &SendEmailDlg::OnBnClickedMfclink1)
END_MESSAGE_MAP()// DeTeCt-MFCDlg.cpp : implementation file
//

#include "stdafx.h"
#include "DeTeCt-MFC.h"
#include "DeTect-MFCDlg.h"
#include "afxdialogex.h"


// ProgressDialog dialog

IMPLEMENT_DYNAMIC(ProgressDialog, CDialog)

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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
	DDX_Control(pDX, IDC_STATICI, progressInfo);
}


BEGIN_MESSAGE_MAP(ProgressDialog, CDialog)
END_MESSAGE_MAP()

/**********************************************************************************************//**
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

/**********************************************************************************************//**
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
	// TODO: Add your control notification handler code here

	CString str;
	meanValue.GetWindowTextW(str);
	float val = std::stof(str.GetString());
	val += pNMUpDown->iDelta * 0.1;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	meanValue.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**********************************************************************************************//**
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
	//ss << std::fixed << std::setprecision(2) << val;
	ss << val;
	impactMinTime.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**********************************************************************************************//**
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
	val += pNMUpDown->iDelta * 0.1;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	histScale.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**********************************************************************************************//**
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
	val += pNMUpDown->iDelta * 0.05;
	std::wstringstream ss;
	ss << std::fixed << std::setprecision(2) << val;
	roiSizeFactor.SetWindowTextW(ss.str().c_str());
	*pResult = 0;
}

/**********************************************************************************************//**
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
	val += pNMUpDown->iDelta * 0.05;
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


void SendEmailDlg::OnStnClickedStatics()
{
	// TODO: Add your control notification handler code here
}


void SendEmailDlg::OnStnClickedStaticf3()
{
	// TODO: Add your control notification handler code here
}


void SendEmailDlg::OnBnClickedMfclink1()
{
	ShellExecute(
		NULL,
		L"open",
		L"http://www.astrosurf.com/planetessaf/doc/project_detect.shtml",
		NULL,
		NULL,
		SW_SHOWNORMAL
	);
}

void CDeTeCtMFCDlg::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	// TODO: ajoutez ici le code de votre gestionnaire de messages et/ou les paramètres par défaut des appels
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

void CDeTeCtMFCDlg::OnBnClickedFrame()
{
	// TODO: ajoutez ici le code de votre gestionnaire de notification de contrôle
}
