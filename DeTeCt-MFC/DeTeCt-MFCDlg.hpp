
// DeTeCt-MFCDlg.h : header file
//

#pragma once
//CREATEPROCESS_MANIFEST_RESOURCE_ID RT_MANIFEST "YourApp.exe.manifest"
//void InitCommonControls();

#include "stdafx.h"
/*#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdisp.h>        // MFC Automation classes
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls*/
//#include <string>
//#include <iostream>
//#include <stdio.h>
//#include <vector>
#include <afxcmn.h>			// MFC support for Windows Common Controls
#include "resource.h"

#include "dtcgui.hpp"

//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
#include "WndResizer.h"

const CString filter = CString(_T("Video/image (*.ser,*.avi,*.wmv,*.mp4,*.m4v,*.mov,*.")) + _T(AUTOSTAKKERT_EXT) + _T(",*.png,*.jpg,*.jpeg,*.jp2,*.tif,*.tiff,*.fit,*.fits,*.bmp,*.dib,*.p?m,*.sr,*.ras)|*.avi;*.ser;*.wmv;*.mp4;*.m4v;*.mov;*.as3;*.bmp;*.jpg;*.jpeg;*.jp2;*.dib;*.png;*.p?m;*.sr;*.ras;*.tif;*.tiff;*.fit;*.fits||");

// CDeTeCtMFCDlg dialog

/**********************************************************************************************//**
 * @class	CDeTeCtMFCDlg
 *
 * @brief	Main Dialog: variables are controls
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

class CDeTeCtMFCDlg : public CDialog
{
// Construction
public:
	CDeTeCtMFCDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DETECTMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	virtual BOOL EndDialog();
	virtual BOOL OnCheckUpdate();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	static CListBox impactDetectionLog;
	static CProgressCtrl progressBar;
	static CProgressCtrl progressBar_all;
	static CStatic impactNull;
	static CStatic impactLow;
	static CStatic impactHigh;
	static CStatic probability;
	static CStatic duration;
	static CStatic totalProgress;
	static CStatic fileName;
	static CStatic computingTime;
	static CMFCLinkCtrl detectImageslink;
	static CMFCLinkCtrl zipFilelink;
	static CMFCLinkCtrl detectLoglink;
	DECLARE_MESSAGE_MAP()
public:
	static CButton Auto;
	static CButton Exit;
	static CButton Shutdown;
	static CButton AS;
	static CButton dark;
	static CButton acquisitionLog;
	static CButton SER;
	static CButton SERtimestamps;
	static CButton FITS;
	static CButton FileInfo;
	static CStatic acquisitionSW;
	static CButton execAS;
	static CStatic Instance;
	static CStatic MaxInstances;
	CSpinButtonCtrl ValueMaxInstances;
	afx_msg void OnBnClickedOk();
	afx_msg void OnFileOpenFolder();
	afx_msg void OnHelpExit();
	afx_msg void OnHelpTutorial();
	afx_msg void OnHelpChecksForUpdate();
	afx_msg void OnHelpRerunConfigurationUpdates();
	afx_msg void OnHelpHistory();
	afx_msg void OnHelpDocumentation();
	afx_msg void OnHelpProjectResults();
	afx_msg void OnSettingsUser();
	afx_msg void OnSettingsAdvanced();
	afx_msg void OnFileExit();
	afx_msg void OnLbnSelchangeList1();
	afx_msg void EnableZipLink(BOOL enable);
	afx_msg void EnableImagesLink(BOOL enable);
	afx_msg void EnableLogLink(BOOL enable);
	afx_msg void OnBnClickedCheckResultsButton();
	afx_msg void OnDetectImagesClickedOk();
	static CListBox* getLog() {
		return &impactDetectionLog;
	}
	static CProgressCtrl* getProgress() {
		return &progressBar;
	}
	static CProgressCtrl* getProgress_all() {
		return &progressBar_all;
	}
	static CStatic* getimpactNull() {
		return &impactNull;
	}
	static CStatic* getimpactLow() {
		return &impactLow;
	}
	static CStatic* getimpactHigh() {
		return &impactHigh;
	}
	static CStatic* getprobability() {
		return &probability;
	}
	static CStatic* getduration() {
		return &duration;
	}
	static CStatic* gettotalProgress() {
		return &totalProgress;
	}
	static CStatic* getfileName() {
		return &fileName;
	}
	static CStatic* getcomputingTime() {
		return &computingTime;
	}
	static CMFCLinkCtrl* getdetectImageslink() {
		return &detectImageslink;
	}
	static CMFCLinkCtrl* getzipFilelink() {
		return &zipFilelink;
	}
	static CMFCLinkCtrl* getdetectLoglink() {
		return &detectLoglink;
	}
	static CButton* getAuto() {
		return &Auto;
	}
	static CButton* getExit() {
		return &Exit;
	}
	static CButton* getShutdown() {
		return &Shutdown;
	}
	static CButton* getAS() {
		return &AS;
	}
	static CButton* getdark() {
		return &dark;
	}
	static CButton* getacquisitionLog() {
		return &acquisitionLog;
	}
	static CButton* getSER() {
		return &SER;
	}
	static CButton* getSERtimestamps() {
		return &SERtimestamps;
	}
	static CButton* getFITS() {
		return &FITS;
	}
	static CButton* getFileInfo() {
		return &FileInfo;
	}
	static CStatic* getacquisitionSW() {
		return &acquisitionSW;
	}
	static CButton* getexecAS() {
		return &execAS;
	}
	static CStatic* getInstance() {
		return &Instance;
	}
	static CStatic* getMaxInstances() {
		return &MaxInstances;
	}

	AcquisitionFilesList acquisition_files;
	afx_msg void OnFileOpenfile();
	afx_msg void OnFileResetFileList();
	afx_msg void OnFileClearExecutionLog();
	afx_msg void OnFileCleanImpactFiles();
	std::string scan_folder_path = {};
	std::wstring max_mean_folder_path = {};
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedFrame();

//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
private:
	CWndResizer m_resizer;
public:
	afx_msg void OnBnClickedOk3();
	afx_msg void OnStnClickedStaticproba();
	afx_msg void OnBnClickedOk2();
	afx_msg void OnBnClickedCheckAuto();
	afx_msg void OnBnClickedCheckExit();
	afx_msg void OnBnClickedCheckShutdown();
	afx_msg void OnDeltaposSpinInstances(NMHDR *pNMHDR, LRESULT *pResult);
};
#pragma once

// PrefDialog dialog

/**********************************************************************************************//**
 * @class	PrefDialog
 *
 * @brief	Dialog for setting the preferences.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

class PrefDialog : public CDialog
{
	DECLARE_DYNAMIC(PrefDialog)

public:
	PrefDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~PrefDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_ADVANCED };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();

	//Processing Visualisation
	CButton showROI;
	CButton showTrack;
	CButton showDif;
	CButton showRef;
	CButton showMask;
	CButton showThresh;
	CButton showSmooth;
	CButton showHist;
	CButton showResult;
	CButton saveIntFramesADUdtc;
	
	CButton NoZip;
	CButton Debug;
	CButton CleanDir;
	afx_msg void OnBnClickedClean();

	//Impact
	CEdit meanValue;
	CSpinButtonCtrl meanValueSpin;
	afx_msg void OnDeltaposSpin1(NMHDR* pNMHDR, LRESULT* pResult); //Meanvalue
	CEdit impactMinTime;
	CSpinButtonCtrl minTimeSpin;
	afx_msg void OnDeltaposSpin2(NMHDR* pNMHDR, LRESULT* pResult); //Impactm min time
	CEdit impactRadius;
	CSpinButtonCtrl radiusSpin;
	CEdit impactBrightThresh;
	CSpinButtonCtrl brightThreshSpin;
	CButton applyMask;
	
	//ROI
	CEdit roiSizeFactor;
	CSpinButtonCtrl sizeFactSpin;
	afx_msg void OnDeltaposSpin12(NMHDR* pNMHDR, LRESULT* pResult); //roiSizeFactor
	CEdit roiSecFactor;
	CSpinButtonCtrl secFactSpin;
	afx_msg void OnDeltaposSpin13(NMHDR* pNMHDR, LRESULT* pResult); //roiSecFactor
	CEdit roiMedianBufSize;
	CSpinButtonCtrl medianBufSpin;

	//Other processing configuration
	CEdit impactFrameNum;
	CSpinButtonCtrl nframeSpin;
	CEdit minimumFrames;
	CSpinButtonCtrl minFrameSpin;
	CEdit histScale;
	CSpinButtonCtrl histoSpin;
	afx_msg void OnDeltaposSpin14(NMHDR* pNMHDR, LRESULT* pResult); //histscale
	CComboBox debayeringCode;

	afx_msg void OnCbnSelchangeCombo2();
	CButton ignoreIncorrectFrames;
	CButton useFilter;
	CComboBox filterSelect;
	afx_msg void OnBnClickedCheck15(); //use filter
};
#pragma once

// PrefDialog dialog

/**********************************************************************************************//**
 * @class	PrefDialogUser
 *
 * @brief	Dialog for setting the preferences.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

class PrefDialogUser : public CDialog
{
	DECLARE_DYNAMIC(PrefDialogUser)

public:
	PrefDialogUser(CWnd* pParent = NULL);   // standard constructor
	virtual ~PrefDialogUser();

	// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SETTINGS_USER };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedButton1();

	CButton ShowDetectImg;
	CButton ShowMeanImg;
	//Processing actions
	CButton Email;
	//CButton Explorer;
	CButton Noreprocessing;
	CButton detailedADUdtc;

	CButton datesOnly;
	CButton Flat;
};
#pragma once

// SendEmailDlg dialog

/**********************************************************************************************//**
 * @class	SendEmailDlg
 *
 * @brief	Dialog with the notification which ask to send the log file in an email.
 *
 * @author	Jon
 * @date	2017-05-12
 **************************************************************************************************/

class SendEmailDlg : public CDialog
{
	DECLARE_DYNAMIC(SendEmailDlg)

public:
	SendEmailDlg(CWnd* pParent = NULL);   // standard constructor
	SendEmailDlg(CWnd* pParent, std::vector<std::string> logMessages);
	virtual ~SendEmailDlg();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_SENDLOGDIALOG
	};
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	CLinkCtrl logLocation;
	std::vector<std::string> messages;
	static CListBox outputLog;
	static std::vector<LPCTSTR> logMessages;
	static std::vector<LPCTSTR> getLogM() {
		return logMessages;
	}
	static CListBox* getLog() {
		return &outputLog;
	}
	afx_msg void OnStnClickedStatics();
	afx_msg void OnStnClickedStaticf3();
	afx_msg void OnBnClickedMfclink1();

	//WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
private:
	CWndResizer m_resizer;
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnStnClickedStaticf4();
	afx_msg void OnStnClickedStaticf();
 };
#pragma once


// ProgressDialog dialog - Unused

class ProgressDialog : public CDialog
{
	DECLARE_DYNAMIC(ProgressDialog)

public:
	ProgressDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~ProgressDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PROGRESSDIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
public:
	CProgressCtrl dtcProgress;
	CProgressCtrl dtcProgress_all;
	CStatic progressInfo;
};
