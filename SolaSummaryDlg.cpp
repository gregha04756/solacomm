// SolaSummaryDlg.cpp
//

#include "stdafx.h"
#include "SolaComm.h"
#include "NoticeDialog.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "SolaTCPComm.h"
#include "SolaPage.h"
#include "PollingDlg.h"


using namespace std;

// Global Variables:
extern "C++" HINSTANCE g_hInst;								// current instance
static HWND g_h_Dlg;
extern "C++" TCHAR szTitle[];					// The title bar text
extern "C++" TCHAR szWindowClass[];			// the main window class name
extern "C++" LARGE_INTEGER g_liPollTime;
TCHAR szTimeString[260];
const TCHAR* szSummaryQuitEvent = {_T("SummaryQuitEvent")};
const TCHAR* szTimerQuitEvent = {_T("TimerQuitEvent")};
const TCHAR* szReadEvent = {_T("ReadEvent")};
const TCHAR* szStatusChangeEvent = {_T("StatusChangeEvent")};
const TCHAR* szConfigChangeEvent = {_T("ConfigChangeEvent")};
const TCHAR* szTempUnit = {_T("FC")};
const int i_MB_Limit = 254;
const float flSaveDataSecsInc = (float)0.25;
//HANDLE hCOMDup;
CRITICAL_SECTION gCOMCritSect;
CRITICAL_SECTION gRWDataCritSect;
CRITICAL_SECTION gTimeCritSect;
CRITICAL_SECTION gSaveFileCritSect;
extern "C++" CRITICAL_SECTION g_UpdCountCS;
BOOL bSolaConnected;
BOOL bCommThreadActive;
DWORD dwCommThreadID;
extern "C++" HWND g_hPropSheet;
unsigned char SOLAMBAddress;
BOOL g_bQuit;
HANDLE hSaveFile;
HANDLE g_hTimerQuitEvent;
HANDLE g_hReadEvent;
HANDLE g_hStatusChangeEvent;
HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
HANDLE g_hReadQuitEvent;
HANDLE g_hSummaryQuitEvent;
HANDLE g_hStartRTUPollEvent;
static Ctrash81_Modeless_Dlg_DLL::LPCONNTYPEPARMS g_lp_ctp;
static std::list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>* g_p_SDCL;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveTrendPages;
extern "C++" int g_nActiveStatusPages;
extern "C++" int g_nActiveConfigPages;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcModConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcODResetConfig;
extern "C++" CSolaMBMap* pcDHWConfiguration;
extern "C++" CSolaMBMap* pcLLStatus;
extern "C++" CSolaMBMap* pcXLLStatus;
extern "C++" CSolaMBMap* pcLLConfig;
extern "C++" CSolaMBMap* pcXSystemConfig;
extern "C++" CSolaMBMap* pcBurnerControlStatus;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemStatus;
extern "C++" CSolaMBMap* pcSensorStatus;
extern "C++" CSolaMBMap* pcExtendedSensorStatus;
extern "C++" CSolaMBMap* pcDemandModulationStatus;
extern "C++" CSolaMBMap* pcCHStatus;
extern "C++" CSolaMBMap* pcDHWStatus;
extern "C++" CSolaMBMap* pcPumpStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcAlarmCode;
extern "C++" CSolaMBMap* pcStatistics;
extern "C++" CSolaMBMaps* pcTrendMaps;
extern "C++" CSolaMBMaps* pcAllSolaMaps;
extern "C++" CSolaMBMaps* pcSystemIDMaps;
extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerControlStatusValues;
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" CSolaAlert* pcAlertLog;
extern "C++" CSolaPage* pcSummaryPage;
extern "C++" CSolaPage* pcSystemIDPage;
unsigned char chMBSndBuf[64];
unsigned char chMBRcvBuf[64];
std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
SOLAIDRESPONSE g_SolaID;
extern "C++" int nUpdCount;
MBConnType mbctMBConn;
extern "C++" enum TCP_Gateway_Type g_TCP_gw_selection;
CSolaTCPComm* lpSolaTCPComm;


// Forward declarations of functions included in this code module:
void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam);
LRESULT CALLBACK SolaSummaryDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK MBServerIPDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" DWORD WINAPI TimerThread(LPVOID lpParam);
DWORD WINAPI SummaryThread(LPVOID lParam);
extern "C++" DWORD WINAPI CommThread(LPVOID lParam);
extern "C++" DWORD WINAPI RTU_Serial_Gateway_Fn(LPVOID lpParam);
DWORD OpenCOMPort(HWND hParentWnd, int nCommPort, HANDLE &hCOM);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" DWORD ShowWarning(void);
extern "C++" DWORD Init_TCP_GW_Combo(HWND hwnd_pd);
float TempVal(BOOL units, short temp);
short ssTempVal (BOOL units, short temp);
float HystVal(BOOL units, short temp);
signed short SolaTempVal(BOOL units, short temp);
signed short SolaHystVal (BOOL units, short temp );
sctf RTU_Comm_Thread_Fn;
static void Scan_Done_Handler(void *p_v);
static void Dlg_End_Callback(void* p_v);
DWORD Update_Sola_Dev_Coords_UI(HWND &hwnd_dlg,std::list<CSola_Auto_ID_DLL::SOLADEVICECOORDS> *lp_sdcl);
DWORD Update_Modbus_Address_List(HWND &hwnd_dlg,std::list<CSola_Auto_ID_DLL::SOLADEVICECOORDS> *lp_sdcl,std::wstring *p_wstr_in);
DWORD Activate_COM_Port_Selection(HWND &hwnd_dlg,HANDLE &h_COM);
void Testing_Dlg_Callback(HWND hwnd_ow);
extern "C++" BOOL OnExitCleanup(void);

// Message handler for Summary page dialog.
LRESULT CALLBACK SolaSummaryDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	PVOID p_v;
	LPNMHDR lpnmhdr;
	LPPSHNOTIFY lppsn;
	HWND hTempWnd;
	HRESULT hRes;
	BOOL bResult;
	static BOOL b_COM_parm;
	static HANDLE hCOM;
	static HWND hMBAddrSpin;
	static HWND hSaveDataSecsSpin;
	char chSaveBuf[2048];
	int i;
	int iNdx = 0;
	int hh;
	int mm;
	int ss;
	unsigned int uiResult;
	static int nCommPort;
	static int nPageIndex;
	static float nSaveDataSecs;
	static float nSaveDataCntr;
	int nRes;
	static TCHAR szCommPort[5];
	TCHAR sz_Comm_Port_Sel[8];
	wstring wstr_cps;
	wstring wstr_fs;
	TCHAR szErrMsg[260];
	static BOOL bSuccess;
	static BOOL bStatus;
	static DWORD dwBytesRead;
	static DWORD dwBytesWritten;
	static HANDLE hTimerThread;
	static HANDLE hTimerThreadDup;
	static HANDLE hCommThread;
	static HANDLE hCommThreadDup;
	static HANDLE hSummaryThread;
	static HANDLE hSummaryThreadDup;
	static HWND hRWWnd;
	static DWORD dwTimerThreadID;
	static DWORD dwSummaryThreadID;
	DWORD dwResult;
	int nResult;
	size_t nLen;
	LPTimerThreadParms pTimerParms;
	LPRWTHREADPARMS pRWParms;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static OPENFILENAME ofnSaveFile;
	static TCHAR szFilter[MAX_PATH];
	static TCHAR szSaveFileName[MAX_PATH];
	static TCHAR szDefaultExt[5];
	static TCHAR szSOLAMBAddr[5];
	SYSTEMTIME st;
	LRESULT lRes;
	static int nHeartBeat;
	TCHAR szTemp[MAX_LOADSTRING];
	TCHAR szSaveBuf[1024];
	SYSTEMTIME LocalTime;
	LRESULT lResult;
	static unsigned short usRegAcc;
	static DWORD dwIPAddress;
	static u_long ulIPAddress;
	static WSADATA wsWSAData;
	static char chHostName[MAX_PATH];
	static struct addrinfo *result;
	static struct addrinfo *ptr;
	static struct addrinfo hints;
	static struct sockaddr_in *sockaddr_ipv4;
	static CPollingDlg* lpPollingDlg;
	static char* pchMBServer;
	static char chMBServer[32];
	static in_addr addrMBServer;
	static char chPort[6];
	static TCHAR szPort[6];
	static TCHAR szIPAddress[20];
	float flDecimal2;
	const TCHAR szTxHB[] = {_T("|/-\\")};
	const TCHAR szRxHB[] = {_T("|\\-/")};
	static int nHBCntr;
	std::wstring mbcb_txt;
	std::wstring wstr_in;
	static CSola_Auto_ID_DLL *p_sid;
	static Ctrash81_Modeless_Dlg_DLL *p_aidd;
	static std::list<wstring> *lp_ipwsl;
	std::list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>::iterator sdc_it;
	static CSola_Testing_DLL* p_stdlg;
	static int nStatisticsStart;
	static BOOL bSaveDataSecsUpd;
	RECT rectSaveDataSecsEdit;
	RECT rectDialog;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		bSaveDataSecsUpd = FALSE;
		nStatisticsStart = pcSystemIDPage->GetSize() - pcStatistics->GetSize() - 1;
		g_h_Dlg = hDlg;
		p_sid = NULL;
		p_aidd = NULL;
		g_p_SDCL = NULL;
		g_lp_ctp = NULL;
		try
		{
			lp_ipwsl = reinterpret_cast<list<wstring>*>(new list<wstring>);
		}
		catch (std::bad_alloc &ba)
		{
			ReportError(ba.what());
			return true;
		}
		g_liPollTime.QuadPart = 0LL;
		nSaveDataSecs = nSaveDataCntr = 1.0;
		nHBCntr = nHeartBeat = 0;
		bSuccess = true;
		bStatus = false;
		bCommThreadActive = false;
		bSolaConnected = false;
		g_bQuit = false;
		mbctMBConn = RTU_Serial_Direct;
		RTU_Comm_Thread_Fn = (sctf)CommThread;
		g_hTimerQuitEvent = NULL;
		g_hReadEvent = NULL;
		g_hStatusChangeEvent = NULL;
		g_hConfigChangeEvent = NULL;
		hTimerThread = NULL;
		hTimerThreadDup = NULL;
		hCommThread = NULL;
		hCommThreadDup = NULL;
		dwCommThreadID = 0;
		usRegAcc = 0;
		lpSolaTCPComm = NULL;
		lpPollingDlg = NULL;
		p_stdlg = NULL;
		p_v = ::SecureZeroMemory((PVOID)&g_SolaID,sizeof(g_SolaID));
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_SOLASUMMARYDLG);

		::InitializeCriticalSection(&gTimeCritSect);
		::InitializeCriticalSection(&gCOMCritSect);
		::InitializeCriticalSection(&gRWDataCritSect);
		::InitializeCriticalSection(&gSaveFileCritSect);
		if ( bSuccess )
		{
			// Start timer thread
			g_hTimerQuitEvent = ::CreateEvent(NULL, true, false, szTimerQuitEvent);
			bResult = ::ResetEvent(g_hTimerQuitEvent);
			// Start timer thread
			pTimerParms = new TimerThreadParms;
			pTimerParms->hParentWnd = hDlg;
			pTimerParms->hQuitEvent = g_hTimerQuitEvent;
			pTimerParms->cchTimeString = sizeof(szTimeString)/sizeof(TCHAR);
			hTimerThread = ::CreateThread(NULL,0,TimerThread,(LPVOID)pTimerParms,CREATE_SUSPENDED,&dwTimerThreadID);
			bResult = ::DuplicateHandle(::GetCurrentProcess(),hTimerThread,GetCurrentProcess(),&hTimerThreadDup,0,false,DUPLICATE_SAME_ACCESS);
			dwResult = ::ResumeThread(hTimerThread);
		}
		if ( bSuccess )
		{
			// Create Summary Quit event
			g_hSummaryQuitEvent = ::CreateEvent(NULL, true, false, szSummaryQuitEvent);
			bResult = ::ResetEvent(g_hSummaryQuitEvent);
			// Create Read event
			g_hReadEvent = ::CreateEvent(NULL, true, false, szReadEvent);
			g_hStatusChangeEvent = ::CreateEvent(NULL, true, false, szStatusChangeEvent);
			g_hConfigChangeEvent = ::CreateEvent(NULL, true, false, szConfigChangeEvent);
			bResult = ::ResetEvent(g_hReadEvent);
			bResult = ::ResetEvent(g_hStatusChangeEvent);
			bResult = ::ResetEvent(g_hConfigChangeEvent);
			// Start Summary page update thread
			pSummaryParms = new SUMMARYTHREADPARMS;
			pSummaryParms->hParentWnd = hDlg;
			pSummaryParms->pDataCritSect = &gRWDataCritSect;
			pSummaryParms->hReadEvent = g_hReadEvent;
			pSummaryParms->hQuitEvent = g_hSummaryQuitEvent;
			pSummaryParms->nPg = nPageIndex;

			hSummaryThread = ::CreateThread(NULL, 0, SummaryThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwSummaryThreadID);
			bResult = ::DuplicateHandle(::GetCurrentProcess(), hSummaryThread, GetCurrentProcess(), &hSummaryThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
			dwResult = ::ResumeThread(hSummaryThread);
		}
		g_hStartRTUPollEvent = ::CreateEvent(NULL,false,false,NULL);
		bSuccess = (g_hStartRTUPollEvent != NULL);
		nRes = 0;
		b_COM_parm = false;
		if (NULL != lParam)
		{
			LPPROPSHEETPAGE lp_psp = (LPPROPSHEETPAGE)lParam;
			LPCOMPORTPARMS lp_cpp = (LPCOMPORTPARMS)lp_psp->lParam;
			if (NULL != lp_cpp)
			{
				nRes = lp_cpp->i_COM_port - 1;
				b_COM_parm = true;
				delete lp_cpp;
				lp_cpp = NULL;
			}
		}
		nCommPort = 0;
		hCOM = NULL;
		for ( nCommPort = 1; nCommPort <= MAXCOMPORTS; nCommPort++ )
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("COM%d"), nCommPort);
			lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),CB_ADDSTRING,(WPARAM)0,(LPARAM)szTemp);
		}
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),CB_SETCURSEL,(WPARAM)nRes,(LPARAM)0);
		dwResult = Init_TCP_GW_Combo(hDlg);
		bResult = ::CheckRadioButton(hDlg,IDC_BTNRTUSD,IDC_BTNTCP,IDC_BTNRTUSD);
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),CB_SHOWDROPDOWN,(WPARAM)false,(LPARAM)0);
		hSaveFile = NULL;
		hRes = ::StringCchPrintf( szCommPort, sizeof(szCommPort)/sizeof(TCHAR), _T("    ") );
		SOLAMBAddress = 0x01;
		hRes = ::StringCchPrintf(szSOLAMBAddr, sizeof(szSOLAMBAddr)/sizeof(TCHAR), _T("%d"), SOLAMBAddress);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNCONNECT), false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNPOLLDLG), false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_CHKSAVEDATA), false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_SAVEDATASECINTER), false);
		nResult = ::LoadString(g_hInst, IDS_NAMEFILTER, szFilter, sizeof(szFilter)/sizeof(TCHAR));
		nResult = ::LoadString(g_hInst, IDS_DEFAULTEXT, szDefaultExt, sizeof(szDefaultExt)/sizeof(TCHAR));
		for (i = 1; i <= i_MB_Limit; i++)
		{
			mbcb_txt.assign(to_wstring((ULONGLONG)i));
			lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_ADDSTRING,(WPARAM)0,(LPARAM)mbcb_txt.c_str());
			lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
		}

		/* bResult = ::SetDlgItemInt(hDlg, IDC_SAVEDATASECSEDIT, nSaveDataSecs, false); */
		hRes = StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%g"), nSaveDataSecs);
		bResult = SetWindowText(GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), szTemp);
		bResult = GetWindowRect(hDlg, &rectDialog);
		bResult = GetWindowRect(GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), &rectSaveDataSecsEdit);
		hSaveDataSecsSpin =  ::CreateWindowEx(	WS_EX_RIGHTSCROLLBAR,
										UPDOWN_CLASS,
										_T("SaveDataSecsSpin"),
										WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_WRAP,
			360 ,
			300,
										12,
										20,
										hDlg,
										NULL,
										g_hInst,
										NULL);
/*		lRes = ::SendMessage(hSaveDataSecsSpin, UDM_SETBUDDY, (WPARAM)(HWND) ::GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), 0); */
		lRes = ::SendMessage(hSaveDataSecsSpin, UDM_SETRANGE32, 0, 3600);
		for ( i = 0; i < pcSummaryPage->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText(hDlg,
				LBLIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),
				pcSummaryPage->ItemLabel(i));
		}
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveTrendPages++;
		g_lpPageDataEvents[nPageIndex].typePage = TrendPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPTRENDUPD;

		sockaddr_ipv4 = NULL; /* Get this workstations IP address */
		result = NULL;
		ptr = NULL;
		nResult = ::WSAStartup(MAKEWORD(2,2),&wsWSAData);
		if ( nResult != 0 )
		{
			p_v = ::SecureZeroMemory((PVOID)szErrMsg,sizeof(szErrMsg));
			switch (nResult)
			{
			case WSASYSNOTREADY:
				hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%d WSASYSNOTREADY"),nResult);
				break;
			case WSAVERNOTSUPPORTED:
				hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%d WSAVERNOTSUPPORTED"),nResult);
				break;
			case WSAEINPROGRESS:
				hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%d WSAEINPROGRESS"),nResult);
				break;
			case WSAEPROCLIM:
				hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%d WSAEPROCLIM"),nResult);
				break;
			case WSAEFAULT:
				hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%d WSAEFAULT"),nResult);
				break;
			}
			::MessageBox(hDlg,szErrMsg,szTitle,MB_OK);
			nResult = ::WSACleanup();
			return (INT_PTR)TRUE;
		}
		nResult = ::gethostname(chHostName,sizeof(chHostName));
		p_v = SecureZeroMemory( &hints, sizeof(hints) );
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		hints.ai_protocol = IPPROTO_TCP;
		dwResult = getaddrinfo(chHostName,NULL,&hints,&result);
		if ( dwResult != 0 )
		{
			hRes = ::StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("getaddrinfo() failed with error %ld"),dwResult);
			::MessageBox(hDlg,szErrMsg,szTitle,MB_OK);
			WSACleanup();
			return (INT_PTR)TRUE;
		}
		for ( ptr = result; ptr != NULL; ptr = ptr->ai_next)
		{
			if ( ptr->ai_family == AF_INET )
			{
                sockaddr_ipv4 = (struct sockaddr_in *) ptr->ai_addr;
				dwIPAddress = sockaddr_ipv4->sin_addr.S_un.S_addr;
				ulIPAddress = ::htonl((u_long)dwIPAddress);
			}
		}
		addrMBServer.S_un.S_addr = ::ntohl(ulIPAddress);
		pchMBServer = ::inet_ntoa(addrMBServer);
		for ( i = 0; pchMBServer[i] != '\0'; i++ )
		{
			continue;
		}
		for ( i = 0; pchMBServer[i] != '\0' && i < sizeof(chMBServer); i++ )
		{
			chMBServer[i] = pchMBServer[i];
		}
		p_v = ::SecureZeroMemory((PVOID)szIPAddress,sizeof(szIPAddress));
		if ( i < sizeof(chMBServer) )
		{
			chMBServer[i] = '\0';
			nResult = MultiByteToWideChar(CP_ACP,
								MB_PRECOMPOSED,
								chMBServer,
								-1,
								szIPAddress,
								sizeof(szIPAddress)/sizeof(TCHAR));
		}
		bResult = ::SetDlgItemText(hDlg,IDC_BTNIPADDRESS,szIPAddress);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNIPADDRESS),false);
		bResult = ::SetDlgItemText(hDlg,IDC_EDITPORT,DEFAULTMBPORT);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_EDITPORT),false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_TCPGWCOMBO),false);
		Make_Reg_Group_List(pcSummaryPage);
		wstr_fs.assign(pcTrendStatus->GetParmName((unsigned short)0x000a));
		bResult = SetWindowText(GetDlgItem(hDlg,IDC_BTNTESTING),wstr_fs.c_str());

		return true;

	case WM_DEVICECHANGE:
		i = 0;
		return true;

	case WM_NOTIFY:
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);
		hTempWnd = ::GetDlgItem(lpnmhdr->hwndFrom, IDOK);
		hTempWnd = lpnmhdr->hwndFrom;

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				g_bQuit = true;
				::EnterCriticalSection(&gSaveFileCritSect);
				if ( hSaveFile != NULL )
				{
					bResult = ::FlushFileBuffers( hSaveFile );
					bResult = ::CloseHandle( hSaveFile );
					hSaveFile = NULL;
				}
				::LeaveCriticalSection(&gSaveFileCritSect);
				if ( hSummaryThread )
				{
					bResult = ::SetEvent(g_hSummaryQuitEvent);
					dwResult = ::WaitForSingleObject(hSummaryThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for summary update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hSummaryThread);
					bResult = ::CloseHandle(hSummaryThreadDup);
				}
				if ( lpSolaTCPComm )
				{
					delete lpSolaTCPComm;
					lpSolaTCPComm = NULL;
				}
				if ( hCommThread )
				{
					bResult = ::SetEvent(g_hReadQuitEvent);
					dwResult = ::WaitForSingleObject(hCommThread, 10000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for read/write thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hCommThreadDup);
					bResult = ::CloseHandle(hCommThread);
					bSolaConnected = false;
				}
				bResult = ::CloseHandle( g_hReadQuitEvent );
				bResult = ::CloseHandle( g_hReadEvent );
				bResult = ::CloseHandle( g_hStatusChangeEvent );
				bResult = ::CloseHandle( g_hConfigChangeEvent );
				// Shut down timer thread
				bResult = ::SetEvent( g_hTimerQuitEvent );
				dwResult = ::WaitForSingleObject( hTimerThread, 5000 );
				switch (dwResult)
				{
				case WAIT_TIMEOUT:
					::MessageBox( hDlg, _T("Wait for timer thread completion timed out"), szTitle, MB_OK );
					break;
				case WAIT_OBJECT_0:
					break;
				}
				bResult = ::CloseHandle(g_hTimerQuitEvent);
				bResult = ::CloseHandle(g_hStartRTUPollEvent);
				bResult = ::CloseHandle(hTimerThreadDup);
				bResult = ::CloseHandle(hTimerThread);
				if ( hCOM != NULL )
				{
					bResult = ::CloseHandle( hCOM );
				}
				for ( i = 0; i < NUMPROPPAGES; i++ )
				{
					bResult = ::CloseHandle(g_hPageUpdEvents[i]);
					bResult = ::CloseHandle(g_lpPageDataEvents[i].hEvent);
				}
				if (!(NULL == g_hPageUpdEvents))
				{
					delete[] g_hPageUpdEvents;
					g_hPageUpdEvents = NULL;
				}
				if (!(NULL == g_lpPageDataEvents))
				{
					delete[] g_lpPageDataEvents;
					g_lpPageDataEvents = NULL;
				}
				::DeleteCriticalSection(&gCOMCritSect);
				::DeleteCriticalSection(&gRWDataCritSect);
				::DeleteCriticalSection(&gTimeCritSect);
				::DeleteCriticalSection(&gSaveFileCritSect);
				::DeleteCriticalSection(&g_UpdCountCS);
			}
			if ( !lppsn->lParam )	// lParam FALSE if Apply button
			{
				i ^= i;
			}
			if (!(NULL == lp_ipwsl))
			{
				delete lp_ipwsl;
				lp_ipwsl = NULL;
			}
			bResult = OnExitCleanup();
			return true;
		case PSN_RESET:   //sent when Cancel button pressed
			g_bQuit = true;
			::EnterCriticalSection(&gSaveFileCritSect);
			if ( hSaveFile != NULL )
			{
				bResult = ::FlushFileBuffers( hSaveFile );
				bResult = ::CloseHandle( hSaveFile );
				hSaveFile = NULL;
			}
			::LeaveCriticalSection(&gSaveFileCritSect);
			if ( hSummaryThread )
			{
				bResult = ::SetEvent(g_hSummaryQuitEvent);
				dwResult = ::WaitForSingleObject(hSummaryThread, 5000);
				switch (dwResult)
				{
				case WAIT_TIMEOUT:
					::MessageBox( hDlg, _T("Wait for summary update thread completion timed out"), szTitle, MB_OK );
					break;
				case WAIT_OBJECT_0:
					break;
				}
				bResult = ::CloseHandle(hSummaryThread);
				bResult = ::CloseHandle(hSummaryThreadDup);
			}
			if ( lpSolaTCPComm )
			{
				delete lpSolaTCPComm;
				lpSolaTCPComm = NULL;
			}
			if ( hCommThread )
			{
				bResult = ::SetEvent(g_hReadQuitEvent);
				dwResult = ::WaitForSingleObject(hCommThread, 10000);
				switch (dwResult)
				{
				case WAIT_TIMEOUT:
					::MessageBox( hDlg, _T("Wait for read/write thread completion timed out"), szTitle, MB_OK );
					break;
				case WAIT_OBJECT_0:
					break;
				}
				bResult = ::CloseHandle(hCommThreadDup);
				bResult = ::CloseHandle(hCommThread);
				bSolaConnected = false;
			}
			bResult = ::CloseHandle( g_hReadQuitEvent );
			bResult = ::CloseHandle( g_hReadEvent );
			bResult = ::CloseHandle( g_hStatusChangeEvent );
			bResult = ::CloseHandle( g_hConfigChangeEvent );
			// Shut down timer thread
			bResult = ::SetEvent( g_hTimerQuitEvent );
			bResult = ::CloseHandle( hTimerThreadDup );
			dwResult = ::WaitForSingleObject( hTimerThread, 5000 );
			switch (dwResult)
			{
			case WAIT_TIMEOUT:
				::MessageBox( hDlg, _T("Wait for timer thread completion timed out"), szTitle, MB_OK );
				break;
			case WAIT_OBJECT_0:
				break;
			}
			bResult = ::CloseHandle(g_hTimerQuitEvent);
			bResult = ::CloseHandle(g_hStartRTUPollEvent);
			if ( hCOM != NULL )
			{
				bResult = ::CloseHandle(hCOM);
			}
			if (!(NULL == g_hPageUpdEvents))
			{
				for (i = 0; i < NUMPROPPAGES; i++)
				{
					bResult = ::CloseHandle(g_hPageUpdEvents[i]);
					bResult = ::CloseHandle(g_lpPageDataEvents[i].hEvent);
				}
				delete[] g_hPageUpdEvents;
				g_hPageUpdEvents = NULL;
			}
			if (!(NULL == g_lpPageDataEvents))
			{
				delete[] g_lpPageDataEvents;
				g_lpPageDataEvents = NULL;
			}

			::DeleteCriticalSection(&gCOMCritSect);
			::DeleteCriticalSection(&gRWDataCritSect);
			::DeleteCriticalSection(&gTimeCritSect);
			::DeleteCriticalSection(&gSaveFileCritSect);
			::DeleteCriticalSection(&g_UpdCountCS);
			if (!(NULL == lp_ipwsl))
			{
				delete lp_ipwsl;
				lp_ipwsl = NULL;
			}
			break;
         
		case PSN_SETACTIVE:
			PropSheet_UnChanged(::GetParent(hDlg), hDlg);
			break;

		case NM_RELEASEDCAPTURE:
			if (hSaveDataSecsSpin == lpnmhdr->hwndFrom)
			{
				int ii = 0;
			}
			break;

		default:
			break;
		}
		return true;

	case WM_APPTIMER:
		::EnterCriticalSection(&gTimeCritSect);
		bResult = ::SetDlgItemText(hDlg, IDC_TXTTIME, (TCHAR*) lParam);
		::LeaveCriticalSection(&gTimeCritSect);
		nHeartBeat = ~nHeartBeat;
		nHBCntr = ((nHBCntr < (sizeof(szTxHB)/sizeof(TCHAR))-2) ? ++nHBCntr : 0);
		if ( bSolaConnected  )
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%c"),szTxHB[nHBCntr]);
			lResult = ::SetDlgItemText(hDlg,IDC_TXTTXHB,szTemp);
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%c"),szRxHB[nHBCntr]);
			lResult = ::SetDlgItemText(hDlg,IDC_TXTRXHB,szTemp);
			if (NULL == lpPollingDlg)
			{
				bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLDLG),true);
			}
		}	

		nSaveDataCntr = (hSaveFile ? --nSaveDataCntr : nSaveDataSecs);
		return true;

	case WM_APPTIMER5:
		return true;

	case WM_APPREADOK:
		if (bSolaConnected && (mbctMBConn == RTU_Serial_Direct) || (mbctMBConn == RTU_Serial_Gateway))
		{
			i = MultiByteToWideChar(
				CP_ACP,
				MB_PRECOMPOSED,
				pcSystemIDBurnerName->GetLPMap(0)->pchStr,
				-1,
				szTitle,
				MAX_LOADSTRING);
		}
		if (bSolaConnected && (mbctMBConn == TCP) && (TCP_Lantronix != g_TCP_gw_selection))
		{
			i = MultiByteToWideChar(
				CP_ACP,
				MB_PRECOMPOSED,
				pc_Dup_Burner_Name->GetLPMap(0)->pchStr,
				-1,
				szTitle,
				MAX_LOADSTRING);
		}
		if (bSolaConnected && (mbctMBConn == TCP) && (TCP_Lantronix == g_TCP_gw_selection))
		{
			i = MultiByteToWideChar(
				CP_ACP,
				MB_PRECOMPOSED,
				pcSystemIDBurnerName->GetLPMap(0)->pchStr,
				-1,
				szTitle,
				MAX_LOADSTRING);
		}
		bResult = ::SetWindowText(::GetParent(hDlg), szTitle);
		return true;
	case WM_APPREADERR:
		hRes = ::StringCchCat(szTitle, MAX_LOADSTRING, _T(" comm error"));
		bResult = ::SetWindowText(::GetParent(hDlg), szTitle);
		return true;
	case WM_APPDATAUPDSTART:
		hRes = ::StringCchCat(szTitle, MAX_LOADSTRING, _T(" retrieving data..."));
		bResult = ::SetWindowText(::GetParent(hDlg), szTitle);
		return true;
	case WM_APPSOLAPOLLABORT:
		nResult = ::MessageBox(hDlg,_T("Communications error, polling aborted"),szTitle,MB_OK);
		return true;
	case WM_APPQUITPOLLINGDLG:
		if ( lpPollingDlg )
		{
			if ( ::IsWindow(lpPollingDlg->GetHWNDPollingDlg()) )
			{
				delete lpPollingDlg;
				lpPollingDlg = NULL;
			}
		}
#if 0
		if ( NULL != hCOM )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLDLG),true);
		}
#endif
		return true;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcSummaryPage);
		if (b_COM_parm)
		{
			dwResult = Activate_COM_Port_Selection(hDlg, hCOM);
		}
		return true;
	case WM_APPTRENDUPD:
			i = MultiByteToWideChar(CP_ACP,
									MB_PRECOMPOSED,
									g_SolaID.BurnerName,
									-1,
									szTitle,
									MAX_LOADSTRING);
			::GetLocalTime(&LocalTime);
			hRes = ::StringCchPrintf(szSaveBuf, sizeof(szSaveBuf)/sizeof(TCHAR),_T("%d-%02d-%02d_%02d:%02d:%02d,"), LocalTime.wYear, LocalTime.wMonth, LocalTime.wDay, LocalTime.wHour, LocalTime.wMinute, LocalTime.wSecond);
			p_v = ::SecureZeroMemory((PVOID)szTemp,sizeof(szTemp));
			for ( i = 0; !g_bQuit && i < pcSummaryPage->GetSize(); i++ )
			{
				switch (pcSummaryPage->ItemMap(i)->GetType(pcSummaryPage->ItemIndex(i)))
				{
				case CSolaMBMap::Temperature:
					if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
					{
						nResult = ::LoadString(g_hInst,IDS_UNCONFIGURED,szTemp,sizeof(szTemp)/sizeof(TCHAR));
					}
					else
					{
						if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) > 1300 )
						{
							hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T(""));
						}
						else
						{
							hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.2f"),
								TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
							if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
							{
								hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.1f"),
									TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
							}
						}
					}
					lResult = ::SetDlgItemText(hDlg,TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),szTemp);
					break;
				case CSolaMBMap::ODTemperature:
					/* Determine OD temperature source */
					nRes = pcExtendedSensorStatus->GetValue((int)1);
					switch (nRes)
					{
					case 0: /* None */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSNONE,szTemp,sizeof(szTemp)/sizeof(TCHAR));
						break;
					case 1: /* Normal */
						if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
						{
							nResult = ::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
						}
						else
						{
							if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) > 1300 )
							{
								hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T(""));
							}
							else
							{
								hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.2f"),
									TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
								if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
								{
									hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.1f"),
										TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
								}
							}
						}
						break;
					case 2: /* Open */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSOPEN, szTemp, sizeof(szTemp) / sizeof(TCHAR));
						break;
					case 3: /* Shorted */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSSHORTED, szTemp, sizeof(szTemp) / sizeof(TCHAR));
						break;
					case 4: /* Outside High range */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSOUTSIDEHIGHRANGE, szTemp, sizeof(szTemp) / sizeof(TCHAR));
						break;
					case 5: /* Outside Low range */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSOUTSIDELOWRANGE, szTemp, sizeof(szTemp) / sizeof(TCHAR));
						break;
					case 6: /* Not reliable */
						nResult = ::LoadString(g_hInst, IDS_SENSORSTATUSNOTRELIABLE, szTemp, sizeof(szTemp) / sizeof(TCHAR));
						break;
					default:
						break;
					}
					lResult = ::SetDlgItemText(hDlg,TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),szTemp);
					break;
				case CSolaMBMap::TemperatureSetpoint:
					if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
					{
						nResult = ::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
					}
					else
					{
						hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.2f"),
							TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
						if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
						{
							hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%-.2f"),
								TempVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
						}
					}
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				case CSolaMBMap::Hysteresis:
					hRes = ::StringCchPrintf(szTemp,
						sizeof(szTemp)/sizeof(TCHAR),
						_T("%-.2f"),
						HystVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("%-.1f"),
							HystVal(pcSystemConfiguration->GetValue((int)0),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))));
					}
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				case CSolaMBMap::Numericvalue:
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)));
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				case CSolaMBMap::Decimal1pl:
					flDecimal2 = (float)pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%-6.1f"), flDecimal2/10.0);
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				case CSolaMBMap::Decimal2pl:
					flDecimal2 = (float)pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%-6.2f"), flDecimal2/100.0);
					if (0x000a == pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i)))
					{
						wstr_fs.assign(pcTrendStatus->GetParmName((unsigned short)0x000a));
						wstr_fs.append(_T("         "));
						wstr_fs.append(szTemp);
						bResult = SetWindowText(GetDlgItem(hDlg,IDC_BTNTESTING),wstr_fs.c_str());
						if (NULL != p_stdlg)
						{
							p_stdlg->Set_Values(pcTrendStatus->GetValue((int)4),
								pcTrendStatus->GetValue((int)1),
								pcTrendStatus->GetValue((int)2),
								pcTrendStatus->GetValue((int)3));
						}
					}
					else
					{
						lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					}
					break;
				case CSolaMBMap::Fanspeed:
					{
						if ( OUTSIDELOWRANGE == (unsigned short)pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) )
						{
							hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("LOW"));
							lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
						}
						else
						{
							hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)));
							lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
						}
					}
					break;
				case CSolaMBMap::SensorMultivalue:
					if ( pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL )
					{
						_ASSERT(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL);
						if ( pcSummaryPage->ItemMap(i)->GetMultiItemValue(pcSummaryPage->ItemIndex(i),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))) != 1 )
						{
							int usA = pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i));
							int nNdx = pcSummaryPage->ItemIndex(i);
							lResult = ::SetDlgItemText(hDlg,
								TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(nNdx)),
								pcSummaryPage->ItemMap(i)->GetMultiValueItem(nNdx,pcSummaryPage->ItemMap(i)->GetValue(nNdx)) );
							hRes = ::StringCchPrintf(szTemp,
								sizeof(szTemp)/sizeof(TCHAR),
								_T("%s"),
								pcSummaryPage->ItemMap(i)->GetMultiValueItem(nNdx,pcSummaryPage->ItemMap(i)->GetValue(nNdx)));
						}
					}
					break;
				case CSolaMBMap::Multivalue:
					if ( pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL )
					{
						_ASSERT(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL);
						int nV = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
						int nA = pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i));
						LPSOLAMULTIVALUE lpM = pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i));
						int nS = pcSummaryPage->ItemMap(i)->GetMultiListSize(pcSummaryPage->ItemIndex(i));
						TCHAR* szS = pcSummaryPage->ItemMap(i)->GetMultiValueItem(pcSummaryPage->ItemIndex(i),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)));
					if (i == 8)
					{
						i = i;
					}
						if ( szS )
						{
							lResult = ::SetDlgItemText(hDlg,
								TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),
								szS );
						hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%s"),szS);
						}
						else
						{
							lResult = ::SetDlgItemText(hDlg,
								TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),
								szS );
							hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%s"),_T("Unknown status"));
						}
					}
					break;
				case CSolaMBMap::Alarmcode:
					{
						uint16_t ar = 0;
						U16 ac = 0;
						ac = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
						ac = 0;
						ar = pcBurnerControlStatus->GetValue((unsigned short)0x0023);
						ar = 0;
					}
					break;
				case CSolaMBMap::Lockoutcode:
					if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) == 0 )
					{
						if ( pcBurnerControlStatus->GetValue((unsigned short)0x0028) == (short) 0 )
						{
							lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), _T(""));
						}
					}
					if ( (pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) != 0) &&
						(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL) )
					{
						_ASSERT(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL);
						nResult = (signed short)pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
						TCHAR* szS = pcSummaryPage->ItemMap(i)->GetMultiValueItem(pcSummaryPage->ItemIndex(i),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)));
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("LOCKOUT %d: %s"),
							nResult,
							szS);
						lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
						hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),szS);
					}
					break;
				case CSolaMBMap::Holdcode:
					if ( pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) == 0 )
					{
						if ( pcBurnerControlStatus->GetValue((unsigned short)0x0022) == (short) 0 )
						{
							lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), _T(""));
						}
					}
					if ( (pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)) != 0) &&
						(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL) )
					{
						_ASSERT(pcSummaryPage->ItemMap(i)->GetLPMulti(pcSummaryPage->ItemIndex(i)) != NULL);
						nResult = (signed short)pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i));
						TCHAR* szS = pcSummaryPage->ItemMap(i)->GetMultiValueItem(pcSummaryPage->ItemIndex(i),pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i)));
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("HOLD %d: %s"),nResult,
							szS);
						lResult = ::SetDlgItemText(hDlg,TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))),szTemp);
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("%s"),
							szS);
					}
					break;
				case CSolaMBMap::Seconds:
					hh = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))/3600;
					mm = (pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))-(hh*3600))/60;
					ss = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))-(hh*3600)-(mm*60);
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				case CSolaMBMap::Timevalue:
					hh = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))/3600;
					mm = (pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))-(hh*3600))/60;
					ss = pcSummaryPage->ItemMap(i)->GetValue(pcSummaryPage->ItemIndex(i))-(hh*3600)-(mm*60);
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
					lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcSummaryPage->ItemMap(i)->GetStartRegAddr(pcSummaryPage->ItemIndex(i))), szTemp);
					break;
				default:
					break;
				}
				::EnterCriticalSection(&gSaveFileCritSect);
				if ( hSaveFile != NULL )
				{
/*					hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),pcSummaryPage->ItemLabel(i));
					hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T("=")); */
					hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),szTemp);
					hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T(","));
				}
				::LeaveCriticalSection(&gSaveFileCritSect);
			}
/*			int nStatisticsStart = pcSystemIDPage->GetSize() - pcStatistics->GetSize() - 1; */
			for (iNdx = 0; (hSaveFile != NULL) && (iNdx < pcStatistics->GetSize()); iNdx++)
			{
				unsigned long ulN = pcSystemIDPage->ItemMap(iNdx + nStatisticsStart)->GetU32Value(iNdx);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp) / sizeof(TCHAR), _T("%lu,"), ulN);
				hRes = ::StringCchCat(szSaveBuf, sizeof(szSaveBuf) / sizeof(TCHAR), szTemp);
			}
			::EnterCriticalSection(&gSaveFileCritSect);
			if ( hSaveFile != NULL && !(nSaveDataCntr > 0) )
			{
				hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T("\r\n"));
				hRes = ::StringCchLength(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),&nLen);
				nRes = ::WideCharToMultiByte(CP_ACP,0,szSaveBuf,nLen,chSaveBuf,sizeof(chSaveBuf),NULL,NULL);
				bResult = ::WriteFile(hSaveFile,(LPCVOID)chSaveBuf,(DWORD)(nLen*sizeof(char)),&dwBytesWritten,NULL);
				if ( !bResult )
				{
					bResult = ::CloseHandle(hSaveFile);
					hSaveFile = NULL;
					::MessageBox(hDlg,_T("Error writing data to file"),szTitle,MB_OK);
					bResult = ::CheckDlgButton(hDlg,IDC_CHKSAVEDATA,false);
				}
				GetWindowText(GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), &szTemp[0], sizeof(szTemp) / sizeof(TCHAR));
				/*				nSaveDataCntr = nSaveDataSecs = ::GetDlgItemInt(hDlg, IDC_SAVEDATASECSEDIT, NULL, false); */
				nSaveDataCntr = nSaveDataSecs = (float)_wtof(szTemp);

			}
			::LeaveCriticalSection(&gSaveFileCritSect);
			if ( usRegAcc != pcTrendStatus->GetValue((int)13) )
			{
				usRegAcc = pcTrendStatus->GetValue((int)13);
				for ( i = 0; i < NUMPROPPAGES; i++ )
				{
					if ( i != nPageIndex && g_PageUpdates[i].hPage )
					{
						bResult = ::PostMessage(g_PageUpdates[i].hPage, g_PageUpdates[i].nMsg, (WPARAM)1, (LPARAM)0);
					}
				}
			}
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		return true;
	case WM_APPSOLACONNECTING:
		bResult = ::SetWindowText(::GetParent(hDlg),_T("Attempting connection..."));
		return true;
	case WM_APPSOLAIDUPD:
		if ( (LPSOLAIDRESPONSE)lParam != NULL )
		{
			if ((mbctMBConn == RTU_Serial_Direct) || (mbctMBConn == RTU_Serial_Gateway))
			{
				if ( bSolaConnected && (g_SolaID.SolaID != 0) )
				{
					i = MultiByteToWideChar(CP_ACP,
										MB_PRECOMPOSED,
										g_SolaID.BurnerName,
										-1,
										szTitle,
										MAX_LOADSTRING);
				}
			}
			if ( mbctMBConn == TCP && lpSolaTCPComm )
			{
				hRes = ::StringCchPrintf(szTitle,MAX_LOADSTRING,_T("%s"),lpSolaTCPComm->GetBurnerName());
				bSolaConnected = lpSolaTCPComm->IsSolaConnected();
			}
			if ( bSolaConnected )
			{
				bResult = ::SetWindowText(::GetParent(hDlg), szTitle);
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_CHKSAVEDATA ), true);
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_SAVEDATASECSEDIT ), true);
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_SAVEDATASECINTER ), true);
			}
			else
			{
				bResult = ::SetWindowText(::GetParent(hDlg),_T("SOLA not connected..."));
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_CHKSAVEDATA ),false);
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_SAVEDATASECSEDIT ),false);
				bResult = ::EnableWindow(::GetDlgItem( hDlg, IDC_SAVEDATASECINTER ),false);
			}
		}
		return true;
	case WM_APPAIDSCANDONECALLBACK:
		if (NULL != lParam)
		{
			if (NULL != g_p_SDCL)
			{
				delete g_p_SDCL;
				g_p_SDCL = NULL;
			}
			g_p_SDCL = reinterpret_cast<list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>*>(new list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>);
			if (NULL != p_aidd->Get_SID()->Get_SDC_List())
			{
				if (!(p_aidd->Get_SID()->Get_SDC_List()->empty()))
				{
					for (sdc_it = p_aidd->Get_SID()->Get_SDC_List()->begin();sdc_it != p_aidd->Get_SID()->Get_SDC_List()->end();sdc_it++)
					{
						g_p_SDCL->push_back(*sdc_it);
					}
					dwResult = Update_Sola_Dev_Coords_UI(hDlg,g_p_SDCL);
					if ((RTU_Serial_Direct == mbctMBConn) || (RTU_Serial_Gateway == mbctMBConn))
					{
						dwResult = Activate_COM_Port_Selection(hDlg,hCOM);
					}
				}
			}
		}
		return true;
	case WM_APPDLGENDING:
		if (NULL != p_aidd)
		{
			delete p_aidd;
			p_aidd = NULL;
		}
		return true;
	case WM_APPTESTINGDLGENDING:
		if (NULL != p_stdlg)
		{
			delete p_stdlg;
			p_stdlg = NULL;
		}
		bResult = EnableWindow(GetDlgItem(hDlg,IDC_BTNTESTING),true);
		return true;
	case WM_COMMAND:
		if ( !bSolaConnected && LOWORD(wParam) == IDC_BTNRTUSD )
		{
			bResult = ::CheckRadioButton( hDlg, IDC_BTNRTUSD, IDC_BTNTCP, LOWORD(wParam) );
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNIPADDRESS),false);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_EDITPORT),false);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_TCPGWCOMBO),false);
#if 0
			if ( NULL != hCOM )
			{
				bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLDLG),true);
			}
#endif
			mbctMBConn = RTU_Serial_Direct;
			RTU_Comm_Thread_Fn = (sctf)CommThread;
		}
		if ( !bSolaConnected && LOWORD(wParam) == IDC_BTNRTUSG )
		{
			bResult = ::CheckRadioButton( hDlg, IDC_BTNRTUSD, IDC_BTNTCP, LOWORD(wParam) );
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNIPADDRESS),false);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_EDITPORT),false);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_TCPGWCOMBO),false);
#if 0
			if ( NULL != hCOM )
			{
				bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLDLG),true);
			}
#endif
			mbctMBConn = RTU_Serial_Gateway;
			RTU_Comm_Thread_Fn = (sctf)RTU_Serial_Gateway_Fn;
		}
		if ( !bSolaConnected && LOWORD(wParam) == IDC_BTNIPADDRESS )
		{
			LPMBSERVERIPPARMS lpIPParms = new MBSERVERIPPARMS;
			lpIPParms->ulIPAddress = ulIPAddress;
			INT_PTR ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_MBSERVERIP),hDlg,MBServerIPDlg,(LPARAM)lpIPParms);
			ulIPAddress = lpIPParms->ulIPAddress;
			delete lpIPParms;
			lpIPParms = NULL;
			addrMBServer.S_un.S_addr = ::ntohl(ulIPAddress);
			pchMBServer = ::inet_ntoa(addrMBServer);
			for ( i = 0; pchMBServer[i] != '\0'; i++ )
			{
				continue;
			}
			for ( i = 0; pchMBServer[i] != '\0' && i < sizeof(chMBServer); i++ )
			{
				chMBServer[i] = pchMBServer[i];
			}
			if ( i < sizeof(chMBServer) )
			{
				chMBServer[i] = '\0';
			}
			i = MultiByteToWideChar(CP_ACP,
									MB_PRECOMPOSED,
									chMBServer,
									-1,
									szIPAddress,
									sizeof(szIPAddress)/sizeof(TCHAR));
			bResult = ::SetDlgItemText(hDlg,IDC_BTNIPADDRESS,szIPAddress);
		}
		if ( !bSolaConnected && LOWORD(wParam) == IDC_BTNTCP )
		{
			bResult = ::CheckRadioButton( hDlg, IDC_BTNRTUSD, IDC_BTNTCP, LOWORD(wParam) );
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_COMPORTCOMBO),false);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNIPADDRESS),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_EDITPORT),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_TCPGWCOMBO),true);
			mbctMBConn = TCP;
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCONNECT),true);
#if 0
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLDLG),false);
#endif
		}
		if ( !bSolaConnected && LOWORD(wParam) == IDC_COMPORTCOMBO && HIWORD(wParam) == CBN_SELCHANGE )
		{
			p_v = SecureZeroMemory((PVOID)sz_Comm_Port_Sel,sizeof(sz_Comm_Port_Sel));
			lRes = SendMessage(GetDlgItem(hDlg,IDC_COMPORTCOMBO),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
			lRes = SendMessage(GetDlgItem(hDlg,IDC_COMPORTCOMBO),CB_GETLBTEXT,(WPARAM)lRes,(LPARAM)sz_Comm_Port_Sel);
			wstr_in.assign(sz_Comm_Port_Sel);
			dwResult = Update_Modbus_Address_List(hDlg,g_p_SDCL,&wstr_in);
			dwResult = Activate_COM_Port_Selection(hDlg,hCOM);
			return (INT_PTR)TRUE;
		}
		if ( !bSolaConnected && LOWORD(wParam) == IDC_TCPGWCOMBO && HIWORD(wParam) == CBN_SELCHANGE )
		{
			g_TCP_gw_selection = (enum TCP_Gateway_Type)SendMessage(GetDlgItem(hDlg,IDC_TCPGWCOMBO),CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
		}
		if ( LOWORD(wParam) == IDC_BTNCONNECT )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCONNECT),false);
			if ( bSuccess && (hCOM != NULL) && ((mbctMBConn == RTU_Serial_Direct) || (mbctMBConn == RTU_Serial_Gateway)) )
			{
				lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_GETCURSEL,(WPARAM)0,(WPARAM)0);
				lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_GETLBTEXT,(WPARAM)lRes,(WPARAM)szSOLAMBAddr);
				mbcb_txt.assign(szSOLAMBAddr);
				SOLAMBAddress = (unsigned char)std::stoi(mbcb_txt);
				bResult = ::SetWindowText(::GetParent(hDlg), _T("Attempting connect, please wait..."));
				pRWParms = new RWTHREADPARMS;
				pRWParms->hCOM = hCOM;
				pRWParms->hParentDlg = hDlg;
				pRWParms->hQuitEvent = g_hReadQuitEvent;
				pRWParms->hReadEvent = g_hReadEvent;
				pRWParms->pCommCritSect = &gCOMCritSect;
				pRWParms->pDataCritSect = &gRWDataCritSect;
				pRWParms->uchSolaAddr = SOLAMBAddress;
				bSuccess = ((hCommThread = ::CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)RTU_Comm_Thread_Fn, (LPVOID) pRWParms, CREATE_SUSPENDED, &dwCommThreadID)) != NULL);
				if ( bSuccess )
				{
					bSuccess = ::DuplicateHandle(::GetCurrentProcess(), hCommThread, GetCurrentProcess(), &hCommThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
				}
				if ( bSuccess )
				{
					bSuccess = ((dwResult = ::ResumeThread(hCommThread)) != (DWORD)-1);
				}
				bResult = ::SetEvent(g_hStartRTUPollEvent);
			}
			if ( bSuccess && (mbctMBConn == TCP) )
			{
				bResult = ::SetWindowText(::GetParent(hDlg), _T("Attempting connect, please wait..."));
				p_v = SecureZeroMemory(&hints,sizeof(hints));
				hints.ai_family = AF_UNSPEC;
				hints.ai_socktype = SOCK_STREAM;
				hints.ai_protocol = IPPROTO_TCP;
				addrMBServer.S_un.S_addr = ::ntohl(ulIPAddress);
				pchMBServer = ::inet_ntoa(addrMBServer);
				for ( i = 0; pchMBServer[i] != '\0'; i++ )
				{
					continue;
				}
				for ( i = 0; pchMBServer[i] != '\0' && i < sizeof(chMBServer); i++ )
				{
					chMBServer[i] = pchMBServer[i];
				}
				if ( i < sizeof(chMBServer) )
				{
					chMBServer[i] = '\0';
				}
				nResult = ::GetDlgItemText(hDlg,IDC_EDITPORT,szPort,sizeof(szPort)/sizeof(TCHAR));
				nResult = ::WideCharToMultiByte(CP_ACP,WC_COMPOSITECHECK,szPort,-1,chPort,sizeof(chPort)/sizeof(char),NULL,NULL);
				dwResult = getaddrinfo(chMBServer,chPort,&hints,&result);
				if ( !lpSolaTCPComm )
				{
					lpSolaTCPComm = (CSolaTCPComm*)new CSolaTCPComm(hDlg,g_TCP_gw_selection);
				}
				if ( lpSolaTCPComm && !lpSolaTCPComm->IsConnected() )
				{
					lpSolaTCPComm->CreateSocket(result);
				}
			}
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDC_BTNPOLLDLG )
		{
			if (NULL == lpPollingDlg )
			{
				try
				{
					lpPollingDlg = (CPollingDlg*)new CPollingDlg(hDlg,g_hInst,szTitle);
				}
				catch (std::bad_alloc &ba)
				{
					ReportError(::GetLastError());
				}
				if ( NULL != lpPollingDlg )
				{
					bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNPOLLDLG), false);
				}
			}
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDC_CHKSAVEDATA )
		{
			uiResult = ::IsDlgButtonChecked(hDlg, IDC_CHKSAVEDATA);
			switch (uiResult)
			{
			case BST_CHECKED:
				if ( hSaveFile == NULL )
				{
					p_v = ::SecureZeroMemory(szSaveFileName, sizeof(szSaveFileName));
					nResult = ::LoadString( g_hInst, IDS_DEFAULTNAME, szSaveFileName, sizeof(szSaveFileName)/sizeof(TCHAR) );
					nResult = ::LoadString( g_hInst, IDS_DEFAULTEXT, szDefaultExt, sizeof(szDefaultExt)/sizeof(TCHAR) );
					ofnSaveFile.lStructSize = (DWORD) sizeof(OPENFILENAME);
					ofnSaveFile.hwndOwner = hDlg;
					ofnSaveFile.lpstrFilter = szFilter;
					::GetLocalTime( &st );
					hRes = ::StringCchLength( szSaveFileName, sizeof(szSaveFileName)/sizeof(TCHAR), &nLen);
					hRes = ::StringCchPrintf( &szSaveFileName[nLen], (sizeof(szSaveFileName)/sizeof(TCHAR))-nLen, _T("_%d-%02d-%02d_%02d_%02d"), st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute );
					ofnSaveFile.lpstrFile = szSaveFileName;
					ofnSaveFile.nMaxFile = sizeof(szSaveFileName)/sizeof(TCHAR);
					ofnSaveFile.lpstrFileTitle = NULL;
					ofnSaveFile.lpstrTitle = NULL;
					ofnSaveFile.Flags = OFN_OVERWRITEPROMPT;
					ofnSaveFile.lpstrDefExt = szDefaultExt;
					if ( (bResult = ::GetSaveFileName( &ofnSaveFile )) )
					{
						hRes = ::StringCchLength( szSaveFileName, sizeof(szSaveFileName)/sizeof(TCHAR), &nLen);
						if ( nLen )
						{
							::EnterCriticalSection(&gSaveFileCritSect);
							hSaveFile = ::CreateFile(	szSaveFileName,
														GENERIC_READ | GENERIC_WRITE,
														0,
														NULL,
														CREATE_ALWAYS,
														FILE_ATTRIBUTE_NORMAL,
														NULL );
							::LeaveCriticalSection(&gSaveFileCritSect);
							if ( hSaveFile == INVALID_HANDLE_VALUE )
							{
								::MessageBox( hDlg, _T("File create failed"), szTitle, MB_OK );
								hSaveFile = NULL;
							}
							if ( (NULL != hSaveFile) && (hSaveFile != INVALID_HANDLE_VALUE) )
							{
								p_v = SecureZeroMemory((PVOID)szSaveBuf,sizeof(szSaveBuf));
								hRes = StringCchPrintf(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T("Time,"));
								for ( i = 0; !g_bQuit && i < pcSummaryPage->GetSize(); i++ )
								{
									hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),pcSummaryPage->ItemLabel(i));
									hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T(","));
								}
								for (i = 0; !g_bQuit && i < pcStatistics->GetSize(); i++)
								{
									hRes = ::StringCchCat(szSaveBuf, sizeof(szSaveBuf) / sizeof(TCHAR), pcStatistics->GetParmName(i));
									hRes = ::StringCchCat(szSaveBuf, sizeof(szSaveBuf) / sizeof(TCHAR), _T(","));
								}
								::EnterCriticalSection(&gSaveFileCritSect);
								if ( hSaveFile != NULL )
								{
									hRes = ::StringCchCat(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),_T("\r\n"));
									hRes = ::StringCchLength(szSaveBuf,sizeof(szSaveBuf)/sizeof(TCHAR),&nLen);
									nRes = ::WideCharToMultiByte(CP_ACP,0,szSaveBuf,nLen,chSaveBuf,sizeof(chSaveBuf),NULL,NULL);
									bResult = ::WriteFile(hSaveFile,(LPCVOID)chSaveBuf,(DWORD)(nLen*sizeof(char)),&dwBytesWritten,NULL);
									if ( !bResult )
									{
										bResult = ::CloseHandle(hSaveFile);
										hSaveFile = NULL;
										::MessageBox(hDlg,_T("Error writing data to file"),szTitle,MB_OK);
										bResult = ::CheckDlgButton(hDlg,IDC_CHKSAVEDATA,false);
									}
								}
								::LeaveCriticalSection(&gSaveFileCritSect);
							}
						}
					}
					else
					{
						bResult = ::CheckDlgButton( hDlg, IDC_CHKSAVEDATA, BST_UNCHECKED );
					}
				}
				else
				{
					::EnterCriticalSection(&gSaveFileCritSect);
					bResult = ::FlushFileBuffers( hSaveFile );
					bResult = ::CloseHandle( hSaveFile );
					hSaveFile = NULL;
					::LeaveCriticalSection(&gSaveFileCritSect);
					bResult = ::CheckDlgButton( hDlg, IDC_CHKSAVEDATA, BST_UNCHECKED );
				}
				break;
			case BST_UNCHECKED:
				::EnterCriticalSection(&gSaveFileCritSect);
				if ( hSaveFile != NULL )
				{
					bResult = ::FlushFileBuffers( hSaveFile );
					bResult = ::CloseHandle( hSaveFile );
					hSaveFile = NULL;
				}
				::LeaveCriticalSection(&gSaveFileCritSect);
				break;
			default:
				break;
			}
		}
		if ( LOWORD(wParam) == IDC_CBMBADDR && HIWORD(wParam) == CBN_SELCHANGE )
		{
			lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_GETCURSEL,(WPARAM)0,(WPARAM)0);
			lRes = SendMessage(GetDlgItem(hDlg,IDC_CBMBADDR),CB_GETLBTEXT,(WPARAM)lRes,(WPARAM)szSOLAMBAddr);
			mbcb_txt.assign(szSOLAMBAddr);
			SOLAMBAddress = (unsigned char)std::stoi(mbcb_txt);
		}
		if (LOWORD(wParam) == IDC_BTNAUTOID && HIWORD(wParam) == BN_CLICKED)
		{
			if( NULL == p_aidd)
			{
				try
				{
					p_aidd = reinterpret_cast<Ctrash81_Modeless_Dlg_DLL*>(new Ctrash81_Modeless_Dlg_DLL(hDlg,g_hInst,szTitle));
				}
				catch(std::bad_alloc &ba)
				{
					ReportError(ba.what());
					return true;
				}
				try
				{
					g_lp_ctp = reinterpret_cast<Ctrash81_Modeless_Dlg_DLL::LPCONNTYPEPARMS>(new Ctrash81_Modeless_Dlg_DLL::CONNTYPEPARMS);
				}
				catch (std::bad_alloc &ba)
				{
					ReportError(ba.what());
					return true;
				}
				if (NULL != g_lp_ctp)
				{
					if ((RTU_Serial_Direct == mbctMBConn) || (RTU_Serial_Gateway == mbctMBConn))
					{
						g_lp_ctp->ct = CSola_Auto_ID_DLL::conn_type::RTU;
						g_lp_ctp->p_lipddn = NULL;
					}
					if (TCP == mbctMBConn)
					{
						g_lp_ctp->ct = CSola_Auto_ID_DLL::conn_type::TCP;
						p_v = SecureZeroMemory((PVOID)szIPAddress,sizeof(szIPAddress));
						bResult = GetDlgItemText(hDlg,IDC_BTNIPADDRESS,szIPAddress,sizeof(szIPAddress)/sizeof(TCHAR));
						lp_ipwsl->clear(); /* want list to only have one entry so clear all entries now (2014-09-26)*/
						lp_ipwsl->push_back(wstring(szIPAddress));
						g_lp_ctp->p_lipddn = lp_ipwsl;
					}
					p_aidd->Set_Callback(Dlg_End_Callback);
					p_aidd->Set_Scan_Done_Handler(Scan_Done_Handler);
					bResult = p_aidd->Run(g_lp_ctp);
				}
			}
			return true;
		}
		if ( LOWORD(wParam) == IDC_SAVEDATASECSEDIT && HIWORD(wParam) == EN_UPDATE )
		{
			bSaveDataSecsUpd = !bSaveDataSecsUpd;
			/*nSaveDataCntr = nSaveDataSecs = ::GetDlgItemInt(hDlg, IDC_SAVEDATASECSEDIT, NULL, false); */
			p_v = SecureZeroMemory((PVOID)szTemp, (SIZE_T)sizeof(szTemp));
			nRes = GetWindowText(GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), &szTemp[0], sizeof(szTemp) / sizeof(TCHAR));
			if (!bSaveDataSecsUpd)
			{
				nSaveDataCntr = nSaveDataSecs = (float)_wtof(szTemp) + flSaveDataSecsInc;
				hRes = StringCchPrintf(szTemp, sizeof(szTemp) / sizeof(TCHAR), _T("%g"), nSaveDataSecs);
				bResult = SetWindowText(::GetDlgItem(hDlg, IDC_SAVEDATASECSEDIT), szTemp);
			}
		}



		if (LOWORD(wParam) == IDC_BTNTESTING && HIWORD(wParam) == BN_CLICKED)
		{
/*			dwResult = (DWORD)MessageBox(hDlg,_T("Button clicked"),szTitle,MB_OK);*/
			if (NULL != p_stdlg)
			{
				delete p_stdlg;
				p_stdlg = NULL;
			}
			if (NULL == p_stdlg)
			{
				try
				{
					p_stdlg = (CSola_Testing_DLL*)new CSola_Testing_DLL(hDlg,Testing_Dlg_Callback,g_hInst,szTitle);
				}
				catch (std::exception &se)
				{
					ReportError(se.what());
				}
				if (NULL != p_stdlg)
				{
					bResult = EnableWindow(GetDlgItem(hDlg,IDC_BTNTESTING),false);
/*					p_stdlg->Set_Parent_Callback(Testing_Dlg_Callback);*/
					p_stdlg->Run();
				}
			}
		}





		return true;
	default:
		return false;
	}
	return FALSE;
}

DWORD OpenCOMPort(HWND hParentWnd, int nCommPort, HANDLE &hCOM )
{
	DWORD dwResult = 0;
	DCB dcb;
	COMMTIMEOUTS CommTimeouts;
	BOOL bSuccess = true;
	BOOL bResult;
	TCHAR szCommPort[16];
	HRESULT hRes;

	if ( hCOM != NULL )
	{
		bResult = ::CloseHandle( hCOM );
		hCOM = NULL;
	}
	hRes = ::StringCchPrintf(szCommPort,sizeof(szCommPort)/sizeof(TCHAR),_T("COM%d"),nCommPort);
	if ( nCommPort >= 10 )
	{
		hRes = ::StringCchPrintf(szCommPort,sizeof(szCommPort)/sizeof(TCHAR),_T("\\\\.\\COM%d"),nCommPort);
	}
	hCOM = ::CreateFile(szCommPort,GENERIC_READ | GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_FLAG_OVERLAPPED,NULL);
	if ( hCOM == INVALID_HANDLE_VALUE ) 
	{
		ReportError(::GetLastError());
		hCOM = NULL;
		bSuccess = false;
	}

	if ( bSuccess )
	{
		if ( !(bSuccess = GetCommState(hCOM,&dcb))) 
		{
			// Handle the error.
			ReportError(::GetLastError());
		}
	}
	if ( bSuccess )
	{
		// Fill in DCB: 38,400 bps, 8 data bits, no parity, and 1 stop bit.
		dcb.fBinary = true;
		dcb.fAbortOnError = false;
		dcb.BaudRate = CBR_38400;     // set the baud rate
		dcb.ByteSize = 8;             // data size, xmit, and rcv
		dcb.Parity = NOPARITY;        // no parity bit
		dcb.StopBits = ONESTOPBIT;    // one stop bit
		if ( !(bSuccess = SetCommState(hCOM,&dcb)) ) 
		{
			// Handle the error.
			ReportError(::GetLastError());
		}	
	}
	
	if ( bSuccess )
	{
		if ( !(bSuccess = GetCommTimeouts(hCOM,&CommTimeouts)) )
		{
			// Handle the error.
			ReportError(::GetLastError());
		}
	}
	if ( bSuccess )
	{
		CommTimeouts.ReadIntervalTimeout = 25;           // 1.9ms max. interval between incoming chars.
		CommTimeouts.ReadTotalTimeoutConstant = 500;
		CommTimeouts.ReadTotalTimeoutMultiplier = 0;      // #chars to read does not add to timeout amount
//		CommTimeouts.ReadIntervalTimeout = MAXDWORD;
//		CommTimeouts.ReadTotalTimeoutConstant = 1;
//		CommTimeouts.ReadTotalTimeoutMultiplier = MAXDWORD;
		CommTimeouts.WriteTotalTimeoutConstant = 2000;
		CommTimeouts.WriteTotalTimeoutMultiplier = 60;    // 60ms per char sent
		if ( !(bSuccess = ::SetCommTimeouts( hCOM,&CommTimeouts )) ) 
		{
			// Handle the error.
			ReportError(::GetLastError());
		}	
	}
	return dwResult;
}

float TempVal (BOOL units,short temp)
{
	return ((((9.0*((float)temp))/50.0)+32.0)*(units == FAHRENHEITUNITS))+((((float)temp)/10.0)*(units == CELSIUSUNITS));
}

short ssTempVal (BOOL units, short temp)
{
	return ((((9*temp)/50)+32)*(units == FAHRENHEITUNITS))+(((temp)/10)*(units == CELSIUSUNITS));
}

float HystVal (BOOL units,short temp)
{
	if ( units == FAHRENHEITUNITS )
	{
		return (9.0*((float)temp))/50.0;
	}
	return ((float)temp)/10.0;
}

signed short SolaTempVal (BOOL units,short temp)
{
	return (signed short)(10*((((5.0*((float)temp-32.0))/9.0)*(units == FAHRENHEITUNITS)) + ((float)temp*(units == CELSIUSUNITS))));
}

signed short SolaHystVal (BOOL units,short temp)
{
	return (signed short)(10*((((5.0*(float)temp)/9.0)*(units == FAHRENHEITUNITS)) + ((float)temp*(units == CELSIUSUNITS))));
}

DWORD WINAPI SummaryThread(LPVOID lpParam)
{
	BOOL bResult;
	BOOL bSuccess = true;
	int nPage;
	DWORD dwResult;
	HANDLE hEvents[2];
	HWND hParentWnd;
	LPCRITICAL_SECTION lpReadDataCritSect;

	hParentWnd = ((LPSUMMARYTHREADPARMS) lpParam)->hParentWnd;
	nPage = ((LPSUMMARYTHREADPARMS) lpParam)->nPg;
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hSummaryQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_lpPageDataEvents[nPage].hEvent, GetCurrentProcess(), &hEvents[1], 0, false, DUPLICATE_SAME_ACCESS);
	lpReadDataCritSect= ((LPSUMMARYTHREADPARMS) lpParam)->pDataCritSect;
	delete ((LPSUMMARYTHREADPARMS) lpParam);

	while ( !g_bQuit && bSuccess )
	{
		dwResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE), &hEvents[0], false, INFINITE);
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[0] || dwResult == WAIT_FAILED )
		{
			bResult = ::CloseHandle(hEvents[0]);
			bResult = ::CloseHandle(hEvents[1]);
			return 0;
		}
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[1] )
		{
			bResult = ::PostMessage(hParentWnd, WM_APPTRENDUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("Summary update thread PostMessage error"), szTitle, MB_OK);
			}
		}
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}

void Scan_Done_Handler(void *p_v)
{
	BOOL b_r = TRUE;
	b_r = PostMessage(g_h_Dlg,WM_APPAIDSCANDONECALLBACK,(WPARAM)0,(LPARAM)p_v);
}

void Dlg_End_Callback(void* p_v)
{
	if (!(NULL == g_lp_ctp))
	{
		delete g_lp_ctp;
		g_lp_ctp = NULL;
	}
	if (!(NULL == g_p_SDCL))
	{
		delete g_p_SDCL;
		g_p_SDCL = NULL;
	}

	PostMessage(g_h_Dlg,WM_APPDLGENDING,(WPARAM)0,(LPARAM)0);
}

DWORD Update_Sola_Dev_Coords_UI(HWND &hwnd_dlg,list<CSola_Auto_ID_DLL::SOLADEVICECOORDS> *lp_sdcl)
{
	UINT ui_r;
	DWORD dw_rc;
	LRESULT l_r;
	PVOID p_v;
	TCHAR sz_Intfc_Name[20];
	list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>::iterator sdc_it;
	std::wstring wstr_in;

	dw_rc = ERROR_SUCCESS;
	if (NULL == lp_sdcl)
	{
		return dw_rc;
	}
	if (lp_sdcl->empty())
	{
		return dw_rc;
	}
	if ((RTU_Serial_Direct == mbctMBConn) || (RTU_Serial_Gateway == mbctMBConn))
	{
		l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_COMPORTCOMBO),CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
		for (sdc_it = lp_sdcl->begin(); sdc_it != lp_sdcl->end(); sdc_it++)
		{
			l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_COMPORTCOMBO),CB_FINDSTRING,(WPARAM)-1,(LPARAM)sdc_it->Interface_Name.c_str());
			if (CB_ERR == l_r)
			{
				l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_COMPORTCOMBO),CB_ADDSTRING,(WPARAM)0,(LPARAM)sdc_it->Interface_Name.c_str());
			}
		}
		l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_COMPORTCOMBO),CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
		l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_COMPORTCOMBO),CB_GETLBTEXT,(WPARAM)0,(LPARAM)sz_Intfc_Name);
		wstr_in.assign(sz_Intfc_Name);
		dw_rc = Update_Modbus_Address_List(hwnd_dlg,lp_sdcl,&wstr_in);
	}
	if (TCP == mbctMBConn)
	{
		p_v = SecureZeroMemory((PVOID)sz_Intfc_Name,sizeof(sz_Intfc_Name));
		ui_r = GetDlgItemText(hwnd_dlg,IDC_BTNIPADDRESS,sz_Intfc_Name,sizeof(sz_Intfc_Name)/sizeof(TCHAR));
		wstr_in.assign(sz_Intfc_Name);
		dw_rc = Update_Modbus_Address_List(hwnd_dlg,lp_sdcl,&wstr_in);
	}
	return dw_rc;
}

DWORD Update_Modbus_Address_List(HWND &hwnd_dlg,std::list<CSola_Auto_ID_DLL::SOLADEVICECOORDS> *lp_sdcl,std::wstring *p_wstr_in)
{
	LRESULT l_r;
	DWORD dw_rc;
	std::wstring wstr_cps;
	list<CSola_Auto_ID_DLL::SOLADEVICECOORDS>::iterator sdc_it;
	dw_rc = ERROR_SUCCESS;
	if (NULL == lp_sdcl)
	{
		return dw_rc;
	}
	if (lp_sdcl->empty())
	{
		return dw_rc;
	}
	l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_CBMBADDR),CB_RESETCONTENT,(WPARAM)0,(LPARAM)0);
	for (sdc_it = lp_sdcl->begin(); sdc_it != lp_sdcl->end(); sdc_it++)
	{
		if (*p_wstr_in == sdc_it->Interface_Name)
		{
			l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_CBMBADDR),CB_ADDSTRING,(WPARAM)0,(LPARAM)to_wstring((ULONGLONG)sdc_it->ui8_addr).c_str());
		}
	}
	l_r = SendMessage(GetDlgItem(hwnd_dlg,IDC_CBMBADDR),CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
	return dw_rc;
}

DWORD Activate_COM_Port_Selection(HWND &hwnd_dlg,HANDLE &h_COM)
{
	BOOL b_r;
	PVOID p_v;
	LRESULT l_r;
	HRESULT h_r;
	DWORD dw_rc;
	int i_r;
	int i_ncp;
	TCHAR sz_Comm_Port_Sel[8];
	TCHAR sz_Err_Msg[100];
	std::wstring wstr_cps;
	dw_rc = ERROR_SUCCESS;
	if ((NULL != h_COM) && !bSolaConnected)
	{
		b_r = ::CloseHandle(h_COM);
		h_COM = NULL;
	}
	if (!bSolaConnected)
	{
		p_v = SecureZeroMemory((PVOID)sz_Comm_Port_Sel, sizeof(sz_Comm_Port_Sel));
		i_ncp = (int)::SendMessage(::GetDlgItem(hwnd_dlg, IDC_COMPORTCOMBO), CB_GETCURSEL, (WPARAM)0, (LPARAM)0);
		l_r = SendMessage(::GetDlgItem(hwnd_dlg, IDC_COMPORTCOMBO), CB_GETLBTEXT, (WPARAM)i_ncp, (LPARAM)sz_Comm_Port_Sel);
		wstr_cps.assign(sz_Comm_Port_Sel);
		i_ncp = stoi(wstr_cps.erase(0, 3));
		dw_rc = OpenCOMPort(hwnd_dlg, i_ncp, h_COM);
		if (hwnd_dlg == NULL)
		{
			h_r = ::StringCchPrintf(sz_Err_Msg, sizeof(sz_Err_Msg) / sizeof(TCHAR), _T("Can't open COM port %d, pls retry"), i_ncp);
			i_r = ::MessageBox(NULL, sz_Err_Msg, szTitle, MB_OK);
			b_r = ::EnableWindow(::GetDlgItem(hwnd_dlg, IDC_BTNCONNECT), false);
		}
		else
		{
			b_r = ::EnableWindow(::GetDlgItem(hwnd_dlg, IDC_BTNCONNECT), true);
		}
	}
	return dw_rc;
}

void Testing_Dlg_Callback(HWND hwnd_ow)
{
	BOOL b_r = PostMessage(hwnd_ow,WM_APPTESTINGDLGENDING,(WPARAM)0,(LPARAM)0);
}