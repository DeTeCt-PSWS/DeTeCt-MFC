
// DeTeCt-MFCDlg.h : header file
//

#pragma once
#include "afxwin.h"
#include <string>
#include <iostream>
#include <stdio.h>
#include <vector>
#include "afxcmn.h"
#include "resource.h"

//Test WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
#include "WndResizer.h"



const CString filter = _T("Video/image (*.ser,*.avi,*.wmv,*.m4v,*.as3,*.png,*.jpg,*.jpeg,*.jp2,*.tif,*.tiff,*.fit,*.fits,*.bmp,*.dib,*.p?m,*.sr,*.ras)|*.avi;*.ser;*.wmv;*.as3;*.bmp;*.jpg;*.jpeg;*.jp2;*.dib;*.png;*.p?m;*.sr;*.ras;*.tif;*.tiff;*.fit;*.fits;*.m4v||");

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
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	static CListBox impactDetectionLog;
	static CProgressCtrl progressBar;
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnFileOpen32771();
	afx_msg void OnHelpExit();
	afx_msg void OnSettingsPreferences();
	afx_msg void OnFileExit();
	afx_msg void OnLbnSelchangeList1();
	static CListBox* getLog() {
		return &impactDetectionLog;
	}
	static CProgressCtrl* getProgress() {
		return &progressBar;
	}
	std::vector<std::string> file_list = {};
	std::vector<std::string> acquisition_file_list = {};
	afx_msg void OnFileOpenfile();
	std::string scan_folder_path = {};
	std::wstring max_mean_folder_path = {};
	afx_msg void OnGetMinMaxInfo(MINMAXINFO* lpMMI);
	afx_msg void OnBnClickedFrame();

//Test WndResizer project resize (https://www.codeproject.com/articles/125068/mfc-c-helper-class-for-window-resizing)
private:
	CWndResizer m_resizer;
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
	enum { IDD = IDD_PREFERENCES };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	virtual BOOL OnInitDialog();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	CEdit meanValue;
	CEdit impactFrameNum;
	CEdit impactMinTime;
	CEdit impactRadius;
	CEdit impactBrightThresh;
	CEdit roiSizeFactor;
	CEdit roiSecFactor;
	CEdit roiMedianBufSize;
	CEdit histScale;
	CButton applyMask;
	CButton detailedADUdtc;
	CButton saveIntFramesADUdtc;
	CButton showROI;
	CButton showTrack;
	CButton showDif;
	CButton showRef;
	CButton showMask;
	CButton showThresh;
	CButton showSmooth;
	CButton showHist;
	CButton showResult;
	CButton datesOnly;
	CButton ignoreIncorrectFrames;
	CButton useFilter;
	CComboBox filterSelect;
	CEdit minimumFrames;
	afx_msg void OnBnClickedCheck15();
	afx_msg void OnBnClickedButton1();
	CSpinButtonCtrl meanValueSpin;
	CSpinButtonCtrl minTimeSpin;
	CSpinButtonCtrl radiusSpin;
	CSpinButtonCtrl brightThreshSpin;
	CSpinButtonCtrl sizeFactSpin;
	CSpinButtonCtrl secFactSpin;
	CSpinButtonCtrl medianBufSpin;
	CSpinButtonCtrl nframeSpin;
	CSpinButtonCtrl minFrameSpin;
	CSpinButtonCtrl histoSpin;
	afx_msg void OnDeltaposSpin1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin2(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin14(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin12(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnDeltaposSpin13(NMHDR *pNMHDR, LRESULT *pResult);
	CComboBox debayeringCode;

	afx_msg void OnCbnSelchangeCombo2();
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
	CStatic progressInfo;
};
