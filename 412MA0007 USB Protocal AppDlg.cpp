//
// 412MA0007 Test AppDlg.cpp : implementation file
//
// Vuzix USB protocal access modules.
// VuzixUSBProtocal.cpp + .h
//
// This applications demonstrates the windows SendMessage solution.  However the process can be done using a foreground direct polling method and is demonstrated
//   In an alternate workspace.
// Uses the Windows message for detecting the hot-swap of the usb device.
// Uses onDevChange(...) to search and find the USB device.
// MCUAllocateResources(...) to allocate system resources for readfile and writefile workerthreads.
// MCUReleaseResources(...) to free thread resources.
// MCUSendIoPacket(...) to send command packets to the USB device.
// OnNewPacket(...) to process any incomming packets from the USB device.  Dont block this thread.  
//					Note: the polling method does not require a windows message to process incomming packets.
// 
#include "stdafx.h"
#include "412MA0007 USB Protocal App.h"
#include "412MA0007 USB Protocal AppDlg.h"
#include "afxdialogex.h"
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <sys\stat.h>
/* Beginning of Vuzix USB API Code */
#include <dbt.h>
#ifdef __cplusplus
extern "C" {
#endif
#include "VuzixUSBProtocal.h"
#ifdef __cplusplus
}
#endif
#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define WM_NEW_PACKET			(WM_APP + 1)
#define REPEAT_TIME				1000
#define TEST_TIME				10000
#define GETVERSION_TIMER		1

void CMy412MA0002TestAppDlg::OnBnClickedCancel()
{
	// Release USB protocal resources.
	MCUReleaseResources( MCU_CANCELTHRD_TIME );
	OnCancel();
}

LRESULT CMy412MA0002TestAppDlg::OnNewPacket(WPARAM wp, LPARAM lp)
{
PIDCOMMANDPKT	pRcv_pkt;
VERSIONPKT		MCUVersion;
	// Check for errors on processed-packets.
	if (wp != MCU_OK)
		return(TRUE);
	// WARNING: The updatedata(..) call may be delayed?
	UpdateData(TRUE);
	// Set pointer to most recent recieved packet.
	pRcv_pkt = (PIDCOMMANDPKT)lp;
	// Switch on pktcmd and determine tracker packet to process...
	switch (pRcv_pkt->pkt.pktcmd) {
		case GET_VERSION:
			// Add all HW device version parts to version struct...
			memcpy(&MCUVersion, (PVERSIONPKT)pRcv_pkt->pkt.pktdata.payload, sizeof(VERSIONPKT));
			m_DevStatus.Format(_T("Device Attached - %d.%d"), MCUVersion.USB_vmajor, MCUVersion.USB_vminor);
			m_ProgStatus.Format(_T("Ready for Test..."));
			m_DevLight.SetBitmap(m_GrnLight);
			m_LightSensors.SetBitmap(m_YloLight);
			m_LightStereo.SetBitmap(m_YloLight);
			m_ButtonTestSensors.EnableWindow(TRUE);
			// 1.3: Stop trying to get_version as we are now successful.
			KillTimer(GETVERSION_TIMER);
			UTLOut(m_DevStatus.GetBuffer());
			UTLOut(L"\r\n");
		break;
		case TRACKER_RAW_DATA:
			// Mags x,y,z
			int	Magx, Magy, Magz;
			Magx = (int)(char)pRcv_pkt->pkt.pktdata.payload[0] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[1]) << 8);
			Magy = (int)(char)pRcv_pkt->pkt.pktdata.payload[2] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[3]) << 8);
			Magz = (int)(char)pRcv_pkt->pkt.pktdata.payload[4] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[5]) << 8);
			int	Accx, Accy, Accz;
			Accx = (int)(char)pRcv_pkt->pkt.pktdata.payload[6] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[7]) << 8);
			Accy = (int)(char)pRcv_pkt->pkt.pktdata.payload[8] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[9]) << 8);
			Accz = (int)(char)pRcv_pkt->pkt.pktdata.payload[10] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[11]) << 8);
			int	Gyrx, Gyry, Gyrz;
			Gyrx = (int)(char)pRcv_pkt->pkt.pktdata.payload[12] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[13]) << 8);
			Gyry = (int)(char)pRcv_pkt->pkt.pktdata.payload[14] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[15]) << 8);
			Gyrz = (int)(char)pRcv_pkt->pkt.pktdata.payload[16] + ((int)((char)pRcv_pkt->pkt.pktdata.payload[17]) << 8);
			// Show value of tracker stream in progressbar.
			if (m_TrackerOn) {
				m_LightSensors.SetBitmap(m_GrnLight);
				m_csTrackInfo.Format(L"Magnetometers %-07d,%-07d,%-07d\nAccelerometers %-07d,%-07d,%-07d\nGyroscopes %-07d,%-07d,%-07d", Magx, Magy, Magz, Accx, Accy, Accz, Gyrx, Gyry, Gyrz);
			}
			else {
				m_LightSensors.SetBitmap(m_YloLight);
				m_csTrackInfo.Format(L"");
			}
		break;
		case SET_STEREO_MODE:
			// Indicates stereo mode is now transitioned.
			if (m_StereoOn)
				m_LightStereo.SetBitmap(m_GrnLight);
			else
				m_LightStereo.SetBitmap(m_YloLight);
		break;
		case COMMAND_FAILED:
			switch (pRcv_pkt->pkt.pktdata.value) {
			case TRACKER_RAW_DATA:
			case TRACKER_EULER_DATA:
			case TRACKER_QUATS_DATA:
				// Receive failed...
				break;
			default:
				// General command failed.
				break;
			}
		break;
		default:
		// All other received packets…Get received in foreground request.
		break;
	}
	// WARNING: The updatedata(..) call may be delayed?
	UpdateData(FALSE);
	return(TRUE);
}

LRESULT CMy412MA0002TestAppDlg::OnDevChange(WPARAM wp, LPARAM lp)
{
TCHAR			Err_Str[MAX_PATH];
unsigned long	iwrStatus;
unsigned int	DeviceId;
	UpdateData(TRUE);
	switch (wp) {
		case DBT_DEVNODES_CHANGED:
			// Attempt to detect the transition of plugged or unplugged USB Tracker interface.
			iwrStatus = MCUConnectWithDevice(lp, &DeviceId);
			if (iwrStatus == MCU_CONNECTED) {
				swprintf_s(Err_Str, _countof(Err_Str), L"MCU Interface is online");
				m_ProgStatus.Format(Err_Str);
				// Set signal for Polling process to startup the USB Device.
				m_DevStatus.Format(_T("%d Attached. Getting version..."), DeviceId);
				UTLOut(L"\r\n");
				UTLOut(m_DevStatus.GetBuffer());
				UTLOut(L"\r\n");
				SetTimer(GETVERSION_TIMER, REPEAT_TIME, NULL);
				m_csTrackInfo.Format(_T("Tracker Packets"));
				SetTimer(GETVERSION_TIMER, REPEAT_TIME, NULL);
			}
			else
			if (iwrStatus == MCU_DISCONNECTED) {
				// Set signal: Usb Device is not offline.
				// Note: Any MCUCommand will return MCU_OFFLINE status.
				m_DevStatus.Format(_T("Device Not Attached"));
				m_ProgStatus.Format(_T("Waiting for device plug in..."));
				m_csTrackInfo.Format(_T(""));
				m_DevLight.SetBitmap(m_RedLight);
				m_LightSensors.SetBitmap(m_RedLight);
				m_LightStereo.SetBitmap(m_RedLight);
				
				m_ButtonTestSensors.EnableWindow(FALSE);
				UTLOut(m_DevStatus.GetBuffer());
				UTLOut(L"\r\n");
			}
		break;
	}
	// Keep dialog in focus.
	SetFocus();
	UpdateData(FALSE);
	return TRUE;
}

void CMy412MA0002TestAppDlg::OnTimer(UINT nIDEvent)
{
IDCOMMANDPKT	Send_pkt, Rcv_pkt;
unsigned long	ret;
TCHAR			ErrStr[MAX_PATH];
	switch (nIDEvent) {
	case GETVERSION_TIMER:
			// Attempt to acquire MCU version parts...Look in OnNewPacket(...) method for recieve packet processing.
			Send_pkt.pkt.pktcmd = GET_VERSION;
			ret = MCUSendIoPacket(&Send_pkt, MCU_RETRYCOUNT, MCU_RECEIVETIME_MS);
			if (ret != MCU_OK) {
				MCUProcessError(&Send_pkt, &Rcv_pkt, ret, ErrStr, _countof(ErrStr));
				// Now show me the error
				UpdateData(TRUE);
				m_DevLight.SetBitmap(m_RedLight);
				m_ProgStatus.Format(ErrStr);
				UpdateData(FALSE);
				MCUProcessError(&Send_pkt, NULL, ret, ErrStr, _countof(ErrStr));
				UTLOut(ErrStr);
				UTLOut(L"\r\n");
			}
			else {
				if (m_TrackerOn) {
					Send_pkt.pkt.pktcmd = TRACKER_SET_REPORT_MODE;
					Send_pkt.pkt.pktdata.value = eTRACKER_MODE_RAW;
					ret = MCUSendIoPacket(&Send_pkt, MCU_RETRYCOUNT, MCU_RECEIVETIME_MS);
					// Report status of command.
					MCUProcessError(&Send_pkt, &Rcv_pkt, ret, ErrStr, _countof(ErrStr));
					if (ret == MCU_OK)
						m_ProgStatus.Format(_T("Failed to enable tracking."));
				}
				if (m_StereoOn) {
					Send_pkt.pkt.pktcmd = SET_STEREO_MODE;
					Send_pkt.pkt.pktdata.value = eVFORMAT_SXS_HALF;
					ret = MCUSendIoPacket(&Send_pkt, MCU_RETRYCOUNT, MCU_RECEIVETIME_MS);
					// Report status of command.
					MCUProcessError(&Send_pkt, &Rcv_pkt, ret, ErrStr, _countof(ErrStr));
					if (ret == MCU_OK)
						m_ProgStatus.Format(_T("Failed to enable stereoscopy."));
				}
			}
		break;
	}
	CDialog::OnTimer(nIDEvent);
}

/* Vuzix Code Additions END */
CMy412MA0002TestAppDlg::CMy412MA0002TestAppDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMy412MA0002TestAppDlg::IDD, pParent)
	, m_DevStatus(_T("Device Disconnected"))
	, m_ProgStatus(_T("Waiting for device plug in..."))
	, m_csTrackInfo(_T("Tracker Reports..."))
	
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CMy412MA0002TestAppDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_DEV_STATUS, m_DevStatus);
	
	DDX_Text(pDX, IDC_PROm_STATUS, m_ProgStatus);
	DDX_Text(pDX, IDC_TRACKER_STATUS, m_csTrackInfo);
	DDX_Control(pDX, IDC_STATIC_DEV_STAT, m_DevLight);
	DDX_Control(pDX, IDC_STATIC_STAT6, m_LightSensors);
	DDX_Control(pDX, IDC_STATIC_STAT7, m_LightStereo);
	DDX_Control(pDX, IDC_BUTTON_TEST6, m_ButtonTestSensors);
}

BEGIN_MESSAGE_MAP(CMy412MA0002TestAppDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_TIMER()
	ON_MESSAGE(WM_DEVICECHANGE, OnDevChange)
	ON_MESSAGE(WM_NEW_PACKET, OnNewPacket)

	ON_BN_CLICKED(IDCANCEL,			&CMy412MA0002TestAppDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_TEST6, &CMy412MA0002TestAppDlg::OnBnClickedToggleTracker)
	ON_BN_CLICKED(IDC_BUTTON_STEREO, &CMy412MA0002TestAppDlg::OnBnClickedButtonStereo)
END_MESSAGE_MAP()
//
// Debug string to file or message box.
//
void		CMy412MA0002TestAppDlg::UTLOut(TCHAR *strout)
{
	int			fOut, nBytes;
	if (strout == NULL) {
		DeleteFile(m_Logfile);
		return;
	}
	// U8TEXT ... Really...REALLY!
	if (!(_wsopen_s(&fOut, m_Logfile, _O_CREAT | _O_U8TEXT | _O_APPEND | _O_WRONLY, _SH_DENYNO, _S_IREAD | _S_IWRITE))) {
		nBytes = wcslen(strout) * 2;
		if ((_write(fOut, strout, nBytes)) == -1){
			;//DSWOut( L"Write failed on output\n" );
		}
		_close(fOut);
	}
	else
		;//Failed to open
}

BOOL CMy412MA0002TestAppDlg::PreTranslateMessage(MSG* pMsg)
{
	if (pMsg->message == WM_KEYDOWN) {
		switch (pMsg->wParam) {
		case VK_ESCAPE:
			OnBnClickedCancel();
			break;
		}
	}
	return CDialog::PreTranslateMessage(pMsg);
}

// CMy412MA0002TestAppDlg message handlers
BOOL CMy412MA0002TestAppDlg::OnInitDialog()
{
unsigned long	ret;
size_t			i;
	CDialogEx::OnInitDialog();
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	m_RedLight.LoadBitmapW(IDB_BITMAP_RED);
	m_YloLight.LoadBitmapW(IDB_BITMAP_YLO);
	m_GrnLight.LoadBitmapW(IDB_BITMAP_GRN);
	// VUZIX INIT CODE BEGIN
	// Allocate DirectShow onetime setup for device connections.
	// Perform one-time initializations for the DirectShow module to configure device names and such.
	wcscpy_s(m_Logfile, _countof(m_Logfile), __wargv[0]);
	for (i = wcslen(m_Logfile); i; i--) if (m_Logfile[i] == '\\') break;
	wcscpy_s(&m_Logfile[i], _countof(m_Logfile) - i, L"\\USBProtocal.log.txt");
	UTLOut(NULL);
	// Allocate one time resources required by USB protocal.
	ret = MCUAllocateResources(MCU_CANCELTHRD_TIME, MCU_INPUT_QUE_SIZE, m_hWnd, WM_NEW_PACKET);
	if (ret != MCU_OK) {
		// Problem acquiring resources from OS.
		AfxMessageBox(L"MCU Interface Error: Unable to allocate protocal resources", MB_OK, 0);
		// In this application we will force an exit.
		SendMessage(WM_CLOSE, 0, 0);
	}
	else {
		// Read initialization fields from .INI file.
		m_TrackerOn = false;
		m_StereoOn	= false;
		// If you want the device insertion and removal events we need to register...
		// Not if not interested in vid-pid identifier string.
		// Attempt to find and connect with the Vuzix USB device.
		OnDevChange( DBT_DEVNODES_CHANGED, 1L );
		// Init dialog box info text. will happen during hotplug detection logic.
		m_DevStatus.Format(_T(""));
		m_ProgStatus.Format(_T(""));
		m_csTrackInfo.Format(_T(""));
		// Ensure test button has focus for Esc to exit.
		GetDlgItem(IDC_BUTTON_TEST6)->SetFocus();
	}
	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CMy412MA0002TestAppDlg::OnPaint()
{
	if (IsIconic())	{
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
		CDialogEx::OnPaint();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CMy412MA0002TestAppDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

//
// Turn on tracker, and monitor raw data 
//  Determine dynamic ranges of magnetometer,
//  Determine rates of gyros
//  Determine dynamic ranges of accelerometers
//	Pass the test if the ranges and rates are all within proper scales.
//
void CMy412MA0002TestAppDlg::OnBnClickedToggleTracker()
{
IDCOMMANDPKT	Send_pkt, Rcv_pkt;
unsigned long	ret;
TCHAR			ErrStr[MAX_PATH];
	// Attempt to Turn tracking on/off
	m_LightSensors.SetBitmap(m_RedLight);
	m_TrackerOn = !m_TrackerOn;
	Send_pkt.pkt.pktcmd			= TRACKER_SET_REPORT_MODE;
	if ( m_TrackerOn )
		Send_pkt.pkt.pktdata.value = eTRACKER_MODE_RAW;
	else
		Send_pkt.pkt.pktdata.value = eTRACKER_MODE_OFF;
	ret = MCUSendIoPacket(&Send_pkt, MCU_RETRYCOUNT, MCU_RECEIVETIME_MS);
	// Report status of command.
	MCUProcessError(&Send_pkt, &Rcv_pkt, ret, ErrStr, _countof(ErrStr));
	UpdateData(TRUE);
	if( ret == MCU_OK ) {
		if (m_TrackerOn)
			m_ProgStatus.Format(_T("Recieving Tracker Packets."));
		else
			m_ProgStatus.Format(_T("Tracker is off"));
		m_csTrackInfo.Format(_T(""));
		UTLOut(m_ProgStatus.GetBuffer());
		UTLOut(L"\r\n");
	}
	else {
		m_LightSensors.SetBitmap(m_RedLight);
		m_ProgStatus.Format(ErrStr);
		m_csTrackInfo.Format(_T(""));
		UTLOut(m_ProgStatus.GetBuffer());
		UTLOut(L"\r\n");
	}
	UpdateData(FALSE);
}


void CMy412MA0002TestAppDlg::OnBnClickedButtonStereo()
{
IDCOMMANDPKT	Send_pkt, Rcv_pkt;
unsigned long	ret;
TCHAR			ErrStr[MAX_PATH];
	// Attempt to Turn stereoscopy on/off
	m_LightStereo.SetBitmap(m_RedLight);
	m_StereoOn = !m_StereoOn;
	Send_pkt.pkt.pktcmd = SET_STEREO_MODE;
	if (m_StereoOn)
		Send_pkt.pkt.pktdata.value = eVFORMAT_SXS_HALF;
	else
		Send_pkt.pkt.pktdata.value = eVFORMAT_MONO;
	ret = MCUSendIoPacket(&Send_pkt, MCU_RETRYCOUNT, MCU_RECEIVETIME_MS);
	// Report status of command.
	MCUProcessError(&Send_pkt, &Rcv_pkt, ret, ErrStr, _countof(ErrStr));
	UpdateData(TRUE);
	if (ret == MCU_OK) {
		if (m_StereoOn)
			m_ProgStatus.Format(_T("Stereoscopy is on (SxS)."));
		else
			m_ProgStatus.Format(_T("Stereoscopy is off"));
		m_csTrackInfo.Format(_T(""));
		UTLOut(m_ProgStatus.GetBuffer());
		UTLOut(L"\r\n");
	}
	else {
		m_LightStereo.SetBitmap(m_RedLight);
		m_ProgStatus.Format(ErrStr);
		m_csTrackInfo.Format(_T(""));
		UTLOut(m_ProgStatus.GetBuffer());
		UTLOut(L"\r\n");
	}
	UpdateData(FALSE);
}
