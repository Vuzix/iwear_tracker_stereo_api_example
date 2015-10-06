//
// 412MA0002 Test AppDlg.h : header file
//
#pragma once
#include "afxwin.h"
#include "VuzixAPI.h"
// CMy412MA0002TestAppDlg dialog
class CMy412MA0002TestAppDlg : public CDialogEx
{
// Construction
public:
	CMy412MA0002TestAppDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_MY412MA0002TESTAPP_DIALOG };
	protected:
	virtual void	DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
protected:
	HICON m_hIcon;
	virtual BOOL	OnInitDialog();
	afx_msg void	OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg BOOL	PreTranslateMessage(MSG* pMsg);

	DECLARE_MESSAGE_MAP()
	/* Vuzix Additions START */
	LRESULT			OnDevChange(WPARAM wp, LPARAM lp);
	LRESULT			OnNewPacket(WPARAM wp, LPARAM lp);
public:
	afx_msg void	OnTimer(UINT nIDEvent);

	CString			m_DevStatus;
	CString			m_ProgStatus;
	CString			m_csTrackInfo;
	
	CBitmap			m_RedLight;
	CBitmap			m_YloLight;
	CBitmap			m_GrnLight;
	CButton			m_ButtonTestSensors;
	CStatic			m_DevLight;
	CStatic			m_LightSensors;
	CStatic			m_LightStereo;

	TCHAR			m_Logfile[MAX_PATH];

	bool			m_TrackerOn;
	bool			m_StereoOn;

	afx_msg void	OnBnClickedCancel();
	void			UTLOut(TCHAR *strout);
	afx_msg void	OnBnClickedToggleTracker();
	afx_msg void	OnBnClickedButtonStereo();
};
