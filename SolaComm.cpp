// SolaComm.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"

using namespace std;

// Global Variables:
HINSTANCE g_hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
const TCHAR* szReadQuitEvent = {_T("ReadQuitEvent")};
CRITICAL_SECTION g_UpdCountCS;
HWND g_hPropSheet;
HANDLE* g_hPageUpdEvents;
HANDLE g_h1SecTimer;
extern "C++" HANDLE g_hPollTimer;
LPPAGEDATAEVENT g_lpPageDataEvents;
int g_nActivePages;
int g_nActiveTrendPages;
int g_nActiveStatusPages;
int g_nActiveConfigPages;
PAGEUPDATE g_PageUpdates[NUMPROPPAGES];
int nUpdCount;
unsigned char* g_pMBRequest;
unsigned char* g_pMBResponse;
OVERLAPPED g_oWOverlap;
OVERLAPPED g_oROverlap;
extern "C++" HANDLE g_hWEvents[];
extern "C++" HANDLE g_hREvents[];
extern "C++" HANDLE g_hPollEvents[];
HANDLE g_hWriteCompletedEvent;
HANDLE g_hReadCompletedEvent;
HANDLE g_hReSyncReqEvent;
DWORD g_dwTotalSent;
DWORD g_dwTotalRcvd;
DWORD g_dwTotalCRCErrors;
DWORD g_dwConnectTime;
double g_dErrorRate;
std::list<CSolaMBMap*> *p_Reg_Group_List;
extern "C++" HANDLE g_hReadQuitEvent;
enum TCP_Gateway_Type g_TCP_gw_selection;

// Forward declarations of functions included in this code module:
void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam);
extern "C++" LRESULT CALLBACK SolaSummaryDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK SystemIDDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK CHConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK ModConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK DigitalIODlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK AlertLogDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK LockoutLogDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK ODResetCfgDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK LLStatusDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK DHWConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK BCConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK PumpConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK LimitsConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK LLConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
extern LRESULT CALLBACK SaveRestoreDlgProc(HWND hDlg,UINT uMessage,WPARAM wParam,LPARAM lParam);
extern INT_PTR CALLBACK MBServerIPDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" BOOL InitSolaDatabase();
extern "C++" DWORD ShowWarning(void);

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	BOOL bWarn;
	BOOL bSuccess = true;
	BOOL bResult;
	int i;
	int nResult;
	DWORD dwResult;
	TCHAR* lpErrMsg;
	const TCHAR* szNoWarn = _T("nowarn");
	const int nWarnCnt = 6;
	TCHAR* pszCommandLine;
	TCHAR** ppszArgv;
	int nNumArgs;
	size_t nCmdLineLen;
	HRESULT hRes;
	HLOCAL hLcl;
	PVOID lpvTemp;

	PROPSHEETPAGE psp[NUMPROPPAGES];
	PROPSHEETHEADER psh;

	TCHAR szSummaryPage[MAX_LOADSTRING];
	TCHAR szDigitalIOPage[MAX_LOADSTRING];
	TCHAR szAlertLogPage[MAX_LOADSTRING];
	TCHAR szSystemIDPage[MAX_LOADSTRING];
	TCHAR szCHConfigPage[MAX_LOADSTRING];
	TCHAR szModConfigPage[MAX_LOADSTRING];
	TCHAR szODResetCfgPage[MAX_LOADSTRING];
	TCHAR szDHWConfigPage[MAX_LOADSTRING];
	TCHAR szPumpConfigPage[MAX_LOADSTRING];
	TCHAR szLimitsConfigPage[MAX_LOADSTRING];
	TCHAR szBCConfigPage[MAX_LOADSTRING];
	TCHAR szLLStatusPage[MAX_LOADSTRING];
	TCHAR szLLConfigPage[MAX_LOADSTRING];
	TCHAR szLockoutLogPage[MAX_LOADSTRING];
	TCHAR szSaveRestorePage[MAX_LOADSTRING];
	TCHAR szErrMsg[MAX_LOADSTRING];
	std::wstring ws_COM_arg;
	LPCOMPORTPARMS p_COM_port_parm;
	g_hInst = hInstance;
	nResult = ::LoadString(g_hInst,IDS_APP_TITLE,szTitle,sizeof(szTitle)/sizeof(TCHAR));

#ifdef _DEBUG
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF|_CRTDBG_LEAK_CHECK_DF);
#endif
	bWarn = true;
	pszCommandLine = NULL;
	nNumArgs = 0;
	ppszArgv = NULL;
	g_dwTotalSent = 0L;
	g_dwTotalRcvd = 0L;
	g_dwConnectTime = 0L;
	g_dErrorRate = 0.0L;
	g_dwTotalCRCErrors = 0L;
	g_TCP_gw_selection = TCP_Protonode;
	hRes = ::StringCchLength(::GetCommandLine(),STRSAFE_MAX_CCH,(size_t*)&nCmdLineLen);
	bSuccess = SUCCEEDED(hRes);
	if ( bSuccess )
	{
		pszCommandLine = (TCHAR*)new TCHAR[++nCmdLineLen];
		hRes = ::StringCchCopy(pszCommandLine,nCmdLineLen,::GetCommandLine());
		bSuccess = SUCCEEDED(hRes);
	}
	if ( !bSuccess )
	{
		nResult = ::MessageBox(NULL,_T("Failed getting command line, aborting"),szTitle,MB_OK);
		return 0;
	}
	p_COM_port_parm = NULL;
	ppszArgv = ::CommandLineToArgvW(::GetCommandLine(),&nNumArgs);
	if ( ppszArgv && (1 < nNumArgs) )
	{
		for ( i = 1; i < nNumArgs; i++ )
		{
			ws_COM_arg.assign(ppszArgv[i]);
			nResult = ::wcsncmp(ppszArgv[i],szNoWarn,nWarnCnt);
			bWarn = (std::wstring::npos != ws_COM_arg.find(szNoWarn, 0)) ? false : bWarn;
			if (std::wstring::npos != ws_COM_arg.find(_T("COM"), 0))
			{
				p_COM_port_parm = (LPCOMPORTPARMS)new COMPORTPARMS;
#if _DEBUG
				nResult = ws_COM_arg.length();
				std::wstring ws_temp(ws_COM_arg.substr(3, ws_COM_arg.length() - 3));
#endif
				p_COM_port_parm->i_COM_port = std::stoi(ws_COM_arg.substr(3, ws_COM_arg.length() - 3), (size_t*)&nResult);
			}
		}
	}
	if ( ppszArgv )
	{
		hLcl = ::LocalFree(ppszArgv);
		ppszArgv = NULL;
	}
	if ( pszCommandLine )
	{
		delete[] pszCommandLine;
		pszCommandLine = NULL;
	}
	dwResult = 0;
	if ( bWarn )
	{
		dwResult = ShowWarning();
		if ( !LOWORD(dwResult) )
		{
			nResult = ::MessageBox(NULL,_T("Internal error, exiting program"),szTitle,MB_OK);
			return 0;
		}
	}
	switch HIWORD(dwResult)
	{
	case 0:
		break;
	case 1:
		return 0;
	}
	INITCOMMONCONTROLSEX icce;
	icce.dwSize = sizeof(icce);
	icce.dwICC = ICC_UPDOWN_CLASS | ICC_INTERNET_CLASSES;
	if ( !::InitCommonControlsEx(&icce) )
	{
		dwResult = ::GetLastError();
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						dwResult,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR) &lpErrMsg,
						260,
						NULL );
		HRESULT hRes = StringCchPrintf(szErrMsg,sizeof(szErrMsg)/sizeof(TCHAR),_T("%s exiting, error %d: %s"),szTitle,dwResult,lpErrMsg);
		nResult = ::MessageBox(NULL,(LPWSTR)szErrMsg,szTitle,MB_OK);
		if ( lpErrMsg )
		{
			::LocalFree( lpErrMsg );
			lpErrMsg = NULL;
		}
		return 0;
	}
	try
	{
		bSuccess = InitSolaDatabase();
	}
	catch (std::bad_alloc &ba)
	{
		nResult = ::MessageBox(NULL,_T("Failure initializing data, exiting"),szTitle,MB_OK );
		return 0;
	}
	try
	{
		p_Reg_Group_List = (std::list<CSolaMBMap*>*)new std::list<CSolaMBMap*>;
		p_Reg_Group_List->clear();
	}
	catch(std::exception &e)
	{
		ReportError(e.what());
		return 0;
	}

	if ( NULL == (g_pMBRequest = (unsigned char*)new unsigned char[512]) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_pMBResponse = (unsigned char*)new unsigned char[512]) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_h1SecTimer = ::CreateWaitableTimer(NULL,true,NULL)) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_hPollTimer = ::CreateWaitableTimer(NULL,true,NULL)) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_hReadQuitEvent = ::CreateEvent( NULL,true,false,szReadQuitEvent )) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	bResult = ::ResetEvent(g_hReadQuitEvent);
	if ( NULL == (g_hWriteCompletedEvent = ::CreateEvent(NULL,true,false,NULL)) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_hReadCompletedEvent = ::CreateEvent(NULL,true,false,NULL)) )
	{
		ReportError(::GetLastError());
		return -1;
	}
	if ( NULL == (g_hReSyncReqEvent = ::CreateEvent(NULL,false,false,NULL)) )
	{
		ReportError(::GetLastError());
		return -1;
	}

	lpvTemp = ::SecureZeroMemory((PVOID)&g_oWOverlap,sizeof(OVERLAPPED));
	for ( i = 0; i < NUMRWEVENTS; i++ )
	{
		g_hWEvents[i] = NULL;
	}
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,::GetCurrentProcess(),&g_hWEvents[0],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hWriteCompletedEvent,::GetCurrentProcess(),&g_hWEvents[1],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_h1SecTimer,::GetCurrentProcess(),&g_hWEvents[2],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hWriteCompletedEvent,::GetCurrentProcess(),&(g_oWOverlap.hEvent),0,true,DUPLICATE_SAME_ACCESS);

	lpvTemp = ::SecureZeroMemory((PVOID)&g_oROverlap,sizeof(OVERLAPPED));
	for ( i = 0; i < NUMRWEVENTS; i++ )
	{
		g_hREvents[i] = NULL;
	}
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,::GetCurrentProcess(),&g_hREvents[0],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadCompletedEvent,::GetCurrentProcess(),&g_hREvents[1],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_h1SecTimer,::GetCurrentProcess(),&g_hREvents[2],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadCompletedEvent,::GetCurrentProcess(),&(g_oROverlap.hEvent),0,true,DUPLICATE_SAME_ACCESS);

	for ( i = 0; i < NUMPOLLEVENTS; i++ )
	{
		g_hPollEvents[i] = NULL;
	}
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,::GetCurrentProcess(),&g_hPollEvents[0],0,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hPollTimer,::GetCurrentProcess(),&g_hPollEvents[1],0,true,DUPLICATE_SAME_ACCESS);

	nResult = ::LoadString(g_hInst, IDS_SUMMARYPAGE, szSummaryPage, sizeof(szSummaryPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_DIGITALIOPAGE, szDigitalIOPage, sizeof(szDigitalIOPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_ALERTLOGPAGE, szAlertLogPage, sizeof(szAlertLogPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_SYSTEMIDPAGE, szSystemIDPage, sizeof(szSystemIDPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_CHCONFIGPAGE, szCHConfigPage, sizeof(szCHConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_MODCONFIGPAGE, szModConfigPage, sizeof(szModConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_ODRESETCFGPAGE, szODResetCfgPage, sizeof(szODResetCfgPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_DHWCONFIGPAGE, szDHWConfigPage, sizeof(szDHWConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_PUMPCONFIGPAGE, szPumpConfigPage, sizeof(szPumpConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_LIMCONFIGPAGE, szLimitsConfigPage, sizeof(szLimitsConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_BCCONFIGPAGE, szBCConfigPage, sizeof(szBCConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_LLSTATUSPAGE, szLLStatusPage, sizeof(szLLStatusPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_LLCONFIGPAGE, szLLConfigPage, sizeof(szLLConfigPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_LOCKOUTLOGPAGE, szLockoutLogPage, sizeof(szLockoutLogPage)/sizeof(TCHAR));
	nResult = ::LoadString(g_hInst, IDS_SAVERESTOREPAGE, szSaveRestorePage, sizeof(szSaveRestorePage)/sizeof(TCHAR));
	g_hPageUpdEvents = new HANDLE[sizeof(psp)/sizeof(PROPSHEETPAGE)];
	g_lpPageDataEvents = new PAGEDATAEVENT[sizeof(psp)/sizeof(PROPSHEETPAGE)];
	for ( i = 0; g_hPageUpdEvents != NULL && i < sizeof(psp)/sizeof(PROPSHEETPAGE); i++ )
	{
		g_hPageUpdEvents[i] = ::CreateEvent(NULL, false, false, NULL);
		g_lpPageDataEvents[i].hEvent = ::CreateEvent(NULL, false, false, NULL);
		g_lpPageDataEvents[i].typePage = NoPage;
		g_PageUpdates[i].hPage = NULL;
		g_PageUpdates[i].nMsg = NULL;
	}
	g_nActivePages = 0;
	g_nActiveTrendPages = 0;
	g_nActiveStatusPages = 0;
	g_nActiveConfigPages = 0;
	::InitializeCriticalSection(&g_UpdCountCS);
	nUpdCount = 0;

//Fill out the PROPSHEETPAGE data structure for the Summary page page
	psp[0].dwSize        = sizeof(PROPSHEETPAGE);
	psp[0].dwFlags       = PSP_USETITLE;
	psp[0].hInstance     = g_hInst;
	psp[0].pszTemplate   = MAKEINTRESOURCE(IDD_SOLASUMMARYDLG);
	psp[0].pszIcon       = NULL;
	psp[0].pfnDlgProc    = (DLGPROC)SolaSummaryDlgProc;
	psp[0].pszTitle      = szSummaryPage;
	psp[0].lParam        = (LPARAM)p_COM_port_parm;

//Fill out the PROPSHEETPAGE data structure for the Digital I/O page
	psp[1].dwSize        = sizeof(PROPSHEETPAGE);
	psp[1].dwFlags       = PSP_USETITLE;
	psp[1].hInstance     = g_hInst;
	psp[1].pszTemplate   = MAKEINTRESOURCE(IDD_DIGITALIODLG);
	psp[1].pszIcon       = NULL;
	psp[1].pfnDlgProc    = (DLGPROC)DigitalIODlgProc;
	psp[1].pszTitle      = szDigitalIOPage;
	psp[1].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Alert Log page
	psp[2].dwSize        = sizeof(PROPSHEETPAGE);
	psp[2].dwFlags       = PSP_USETITLE;
	psp[2].hInstance     = g_hInst;
	psp[2].pszTemplate   = MAKEINTRESOURCE(IDD_ALERTLOGDLG);
	psp[2].pszIcon       = NULL;
	psp[2].pfnDlgProc    = (DLGPROC)AlertLogDlgProc;
	psp[2].pszTitle      = szAlertLogPage;
	psp[2].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the System ID Configuration page
	psp[3].dwSize        = sizeof(PROPSHEETPAGE);
	psp[3].dwFlags       = PSP_USETITLE;
	psp[3].hInstance     = g_hInst;
	psp[3].pszTemplate   = MAKEINTRESOURCE(IDD_SYSTEMIDDLG);
	psp[3].pszIcon       = NULL;
	psp[3].pfnDlgProc    = (DLGPROC)SystemIDDlgProc;
	psp[3].pszTitle      = szSystemIDPage;
	psp[3].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the CH Configuration page
	psp[4].dwSize        = sizeof(PROPSHEETPAGE);
	psp[4].dwFlags       = PSP_USETITLE;
	psp[4].hInstance     = g_hInst;
	psp[4].pszTemplate   = MAKEINTRESOURCE(IDD_CHCONFIGDLG);
	psp[4].pszIcon       = NULL;
	psp[4].pfnDlgProc    = (DLGPROC)CHConfigDlgProc;
	psp[4].pszTitle      = szCHConfigPage;
	psp[4].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the OD Reset Config page
	psp[5].dwSize        = sizeof(PROPSHEETPAGE);
	psp[5].dwFlags       = PSP_USETITLE;
	psp[5].hInstance     = g_hInst;
	psp[5].pszTemplate   = MAKEINTRESOURCE(IDD_ODRESETCFGDLG);
	psp[5].pszIcon       = NULL;
	psp[5].pfnDlgProc    = (DLGPROC)ODResetCfgDlgProc;
	psp[5].pszTitle      = szODResetCfgPage;
	psp[5].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the DHW Configuration page
	psp[6].dwSize        = sizeof(PROPSHEETPAGE);
	psp[6].dwFlags       = PSP_USETITLE;
	psp[6].hInstance     = g_hInst;
	psp[6].pszTemplate   = MAKEINTRESOURCE(IDD_DHWCONFIGDLG);
	psp[6].pszIcon       = NULL;
	psp[6].pfnDlgProc    = (DLGPROC)DHWConfigDlgProc;
	psp[6].pszTitle      = szDHWConfigPage;
	psp[6].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the LL Status page
	psp[7].dwSize        = sizeof(PROPSHEETPAGE);
	psp[7].dwFlags       = PSP_USETITLE;
	psp[7].hInstance     = g_hInst;
	psp[7].pszTemplate   = MAKEINTRESOURCE(IDD_LLSTATUSDLG);
	psp[7].pszIcon       = NULL;
	psp[7].pfnDlgProc    = (DLGPROC)LLStatusDlgProc;
	psp[7].pszTitle      = szLLStatusPage;
	psp[7].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the LL Configuration page
	psp[8].dwSize        = sizeof(PROPSHEETPAGE);
	psp[8].dwFlags       = PSP_USETITLE;
	psp[8].hInstance     = g_hInst;
	psp[8].pszTemplate   = MAKEINTRESOURCE(IDD_LLCONFIGDLG);
	psp[8].pszIcon       = NULL;
	psp[8].pfnDlgProc    = (DLGPROC)LLConfigDlgProc;
	psp[8].pszTitle      = szLLConfigPage;
	psp[8].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Lockout Log page
	psp[9].dwSize        = sizeof(PROPSHEETPAGE);
	psp[9].dwFlags       = PSP_USETITLE;
	psp[9].hInstance     = g_hInst;
	psp[9].pszTemplate   = MAKEINTRESOURCE(IDD_LOCKOUTLOGDLG);
	psp[9].pszIcon       = NULL;
	psp[9].pfnDlgProc    = (DLGPROC)LockoutLogDlgProc;
	psp[9].pszTitle      = szLockoutLogPage;
	psp[9].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Modulation Configuration page
	psp[10].dwSize        = sizeof(PROPSHEETPAGE);
	psp[10].dwFlags       = PSP_USETITLE;
	psp[10].hInstance     = g_hInst;
	psp[10].pszTemplate   = MAKEINTRESOURCE(IDD_MODCONFIGDLG);
	psp[10].pszIcon       = NULL;
	psp[10].pfnDlgProc    = (DLGPROC)ModConfigDlgProc;
	psp[10].pszTitle      = szModConfigPage;
	psp[10].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Burner Control Configuration page
	psp[11].dwSize        = sizeof(PROPSHEETPAGE);
	psp[11].dwFlags       = PSP_USETITLE;
	psp[11].hInstance     = g_hInst;
	psp[11].pszTemplate   = MAKEINTRESOURCE(IDD_BCCONFIGDLG);
	psp[11].pszIcon       = NULL;
	psp[11].pfnDlgProc    = (DLGPROC)BCConfigDlgProc;
	psp[11].pszTitle      = szBCConfigPage;
	psp[11].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Limits Configuration page
	psp[12].dwSize        = sizeof(PROPSHEETPAGE);
	psp[12].dwFlags       = PSP_USETITLE;
	psp[12].hInstance     = g_hInst;
	psp[12].pszTemplate   = MAKEINTRESOURCE(IDD_LIMCONFIGDLG);
	psp[12].pszIcon       = NULL;
	psp[12].pfnDlgProc    = (DLGPROC)LimitsConfigDlgProc;
	psp[12].pszTitle      = szLimitsConfigPage;
	psp[12].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Pump Configuration page
	psp[13].dwSize        = sizeof(PROPSHEETPAGE);
	psp[13].dwFlags       = PSP_USETITLE;
	psp[13].hInstance     = g_hInst;
	psp[13].pszTemplate   = MAKEINTRESOURCE(IDD_PUMPCONFIGDLG);
	psp[13].pszIcon       = NULL;
	psp[13].pfnDlgProc    = (DLGPROC)PumpConfigDlgProc;
	psp[13].pszTitle      = szPumpConfigPage;
	psp[13].lParam        = 0;

//Fill out the PROPSHEETPAGE data structure for the Save & Restore page
	psp[14].dwSize        = sizeof(PROPSHEETPAGE);
	psp[14].dwFlags       = PSP_USETITLE;
	psp[14].hInstance     = g_hInst;
	psp[14].pszTemplate   = MAKEINTRESOURCE(IDD_SAVERESTOREDLG);
	psp[14].pszIcon       = NULL;
	psp[14].pfnDlgProc    = (DLGPROC)SaveRestoreDlgProc;
	psp[14].pszTitle      = szSaveRestorePage;
	psp[14].lParam        = 0;

//Fill out the PROPSHEETHEADER
	psh.dwSize           = sizeof(PROPSHEETHEADER);
	psh.dwFlags          = PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK | PSH_NOAPPLYNOW;
//	psh.dwFlags	= PSH_PROPSHEETPAGE | PSH_USEICONID | PSH_USECALLBACK| PSH_MODELESS;
	psh.hwndParent       = NULL;
	psh.hInstance        = g_hInst;
	psh.pszIcon          = MAKEINTRESOURCE(IDI_SOLACOMM);
	psh.pszCaption       = szTitle;
	psh.nPages           = sizeof(psp) / sizeof(PROPSHEETPAGE);
	psh.ppsp             = (LPCPROPSHEETPAGE) &psp;
	psh.pfnCallback      = (PFNPROPSHEETCALLBACK)PropSheetCallback;

//And finally display the modal property sheet
	nResult = (int)PropertySheet(&psh);
	if ( g_hPollTimer )
	{
		bResult = ::CloseHandle(g_hPollTimer);
	}
	if ( NULL != g_hReSyncReqEvent )
	{
		bResult = ::CloseHandle(g_hReSyncReqEvent);
	}
	return nResult;
}

void CALLBACK PropSheetCallback(HWND hwndPropSheet, UINT uMsg, LPARAM lParam)
{
	int i;

	switch(uMsg)
	{
		//called before the dialog is created, hwndPropSheet = NULL, lParam points to dialog resource
	case PSCB_PRECREATE:
		{
			g_hPropSheet = hwndPropSheet;
			LPDLGTEMPLATE  lpTemplate = (LPDLGTEMPLATE)lParam;

			if(!(lpTemplate->style & WS_SYSMENU))
			{
				lpTemplate->style |= WS_SYSMENU;
			}
		}
		break;

   //called after the dialog is created
	case PSCB_INITIALIZED:
		g_hPropSheet = hwndPropSheet;
		break;

	case WM_COMMAND:
		i = 0;
		break;
	}
}
