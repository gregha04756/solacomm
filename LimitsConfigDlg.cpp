#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"

CRITICAL_SECTION g_LimitsConfigCritSect;
const TCHAR* szLimitsConfigQuitEvent = {_T("LimitsConfigQuitEvent")};
const TCHAR* szLimitsConfigMBRWEvent = {_T("LimitsConfigMBRWEvent")};
HANDLE g_hLimitsConfigMBRWEvent;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
HANDLE g_hLimitsConfigQuitEvent;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveConfigPages;
extern "C++" CSolaMBMap* pcSystemConfiguration;
DWORD WINAPI LimitsConfigThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcLimitsConfig;
extern "C++" CSolaPage* pcLimitsConfigPage;
extern "C++" SOLAMULTIVALUE RegisterAccess[];
extern "C++" SOLAMULTIVALUE ModbusExceptionCodes[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" HWND g_hODResetLineWnd;
extern "C++" BOOL UpdatePage(HWND hwndDialog,CSolaPage* lpPage);
extern "C++" INT_PTR CALLBACK UIPercentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);

extern "C++" INT_PTR CALLBACK UINumericDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIMultiDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UITimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK LimitsConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bOkCRC;
	static int nPageIndex;
	static BOOL bStatus;
	BOOL bResult;
	LPNMHDR     lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	int k;
	DWORD dwResult;
	static DWORD dwLimitsConfigThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hLimitsConfigThread;
	static HANDLE hLimitsConfigThreadDup;
	signed short ssValue;
	unsigned short usRegAddr;
	static MBSNDRCVREQ MBSndRcvReq;
	static unsigned char* pchToSnd;
	static unsigned char* pchEndSnd;
	static unsigned char* pchToRcv;
	static unsigned char* pchEndRcv;
	static CSolaMBMap::LPNUMERICUIPARMS lpUIParms;
	static LPMULTIUIPARMS lpMUIParms;
	INT_PTR ipResult;
	CSolaMBMap* lpMap;
	CSolaMBMap::SolaType stType;
	static int nControlIDMin;
	static int nControlIDMax;
	int nResult;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		::InitializeCriticalSection(&g_LimitsConfigCritSect);
		g_hLimitsConfigMBRWEvent = ::CreateEvent(NULL, false, false, szLimitsConfigMBRWEvent);
		bStatus = false;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_LIMCONFIGDLG);
		g_hLimitsConfigQuitEvent = ::CreateEvent(NULL, true, false, szLimitsConfigQuitEvent);
		bResult = ::ResetEvent(g_hLimitsConfigQuitEvent);
		// Start Limits Config update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hConfigChangeEvent;
		pSummaryParms->hQuitEvent = g_hLimitsConfigQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hLimitsConfigThread = ::CreateThread(NULL, 0, LimitsConfigThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwLimitsConfigThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hLimitsConfigThread, GetCurrentProcess(), &hLimitsConfigThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hLimitsConfigThread);
		bResult = UpdatePage(hDlg,pcLimitsConfigPage);
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveConfigPages++;
		g_lpPageDataEvents[nPageIndex].typePage = ConfigPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected )
		{
			bResult = ::PostMessage(hDlg, WM_APPLIMCONFIGUPD, (WPARAM)1, (LPARAM)0);
		}
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPLIMCONFIGUPD;
		nControlIDMin = TXTIDBASE + 0x0fff;
		nControlIDMax = TXTIDBASE;
		for ( i = 0; i < pcLimitsConfigPage->GetSize(); i++ )
		{
			nResult = TXTIDBASE + pcLimitsConfigPage->ItemMap(i)->GetStartRegAddr(pcLimitsConfigPage->ItemIndex(i));
			nControlIDMin = nControlIDMin < nResult ? nControlIDMin : nResult;
			nControlIDMax = nControlIDMax > nResult ? nControlIDMax : nResult;
		}
		Make_Reg_Group_List(pcLimitsConfigPage);
		break;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcLimitsConfigPage);
		bResult = UpdatePage(hDlg,pcLimitsConfigPage);
		break;
	case WM_APPLIMCONFIGUPD:
		bResult = UpdatePage(hDlg,pcLimitsConfigPage);
		if ( bSolaConnected && !LOWORD(wParam))
		{
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		}
		break;
	case WM_APPMBRESPONSE:
		if ( LOWORD(wParam) == 3 )
		{
			::EnterCriticalSection(&gRWDataCritSect);
			bOkCRC =  check_CRC16(chMBRcvBuf, (int)(pchEndRcv-pchToRcv));
			::LeaveCriticalSection(&gRWDataCritSect);
			if ( bOkCRC && ((unsigned char)0x80 & chMBRcvBuf[1]) )
			{
				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
			}
		}
		if ( LOWORD(wParam) == 4 )
		{
			if ( (unsigned char)0x80 & chMBRcvBuf[1] )
			{
				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
			}
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
			int nResult = ::SetBkMode((HDC)wParam,TRANSPARENT);
			return (LRESULT)CreateSolidBrush(0xFFFFFF);
		}
	case WM_COMMAND:
		if ( (HIWORD(wParam) == STN_CLICKED) && (LOWORD(wParam) >= nControlIDMin && LOWORD(wParam) <= nControlIDMax) )
		{
			ipResult = IDCANCEL;
			usRegAddr = (unsigned short)(LOWORD(wParam)-TXTIDBASE);
			stType = CSolaMBMap::Novalue;
			lpMap = NULL;
			for ( i = 0; i < pcLimitsConfigPage->GetSize() && !lpMap; i++ )
			{
				if ( pcLimitsConfigPage->ItemMap(i)->GetStartRegAddr(pcLimitsConfigPage->ItemIndex(i)) == usRegAddr )
				{
					lpMap = pcLimitsConfigPage->ItemMap(i);
				}
			}
			stType = (lpMap == NULL) ? CSolaMBMap::Novalue : lpMap->GetType(usRegAddr);
			switch (stType)
			{
			case CSolaMBMap::TemperatureSetpoint:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Hysteresis:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UINUMERIC), hDlg, UINumericDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Multivalue:
				lpMUIParms = new MULTIUIPARMS;
				lpMUIParms->lpMultiList = lpMap->GetLPMulti(usRegAddr);
				lpMUIParms->nCurSel = lpMap->GetValue(usRegAddr);
				lpMUIParms->nMultiListSize = lpMap->GetMultiListSize(usRegAddr);
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UIMULTI), hDlg, UIMultiDlgProc, (LPARAM)lpMUIParms);
				ssValue = lpMUIParms->nCurSel;
				delete lpMUIParms;
				break;
			case CSolaMBMap::Numericvalue:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = (short)lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UINUMERIC), hDlg, UINumericDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Percentvalue:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = (short)lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIGUIPERCENT), hDlg, UIPercentDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Timevalue:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UITIME), hDlg, UITimeDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			}
			if ( ipResult == IDOK )
			{
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x06;
				*(*MBSndRcvReq.ppchEndSnd)++ = (usRegAddr >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = usRegAddr & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = (ssValue >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = ssValue & 0x00ff;
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
			}
		}
		break;

	case WM_NOTIFY:
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLimitsConfigThread )
				{
					bResult = ::SetEvent(g_hLimitsConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hLimitsConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Limits Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLimitsConfigThread);
					bResult = ::CloseHandle(hLimitsConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hLimitsConfigQuitEvent);
				bResult = ::CloseHandle(g_hLimitsConfigMBRWEvent);
				::DeleteCriticalSection(&g_LimitsConfigCritSect);
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLimitsConfigThread )
				{
					bResult = ::SetEvent(g_hLimitsConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hLimitsConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Limits Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLimitsConfigThread);
					bResult = ::CloseHandle(hLimitsConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hLimitsConfigQuitEvent);
				bResult = ::CloseHandle(g_hLimitsConfigMBRWEvent);
				::DeleteCriticalSection(&g_LimitsConfigCritSect);
			}
			break;
         
		case PSN_SETACTIVE:
			PropSheet_UnChanged(::GetParent(hDlg), hDlg);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

DWORD WINAPI LimitsConfigThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hLimitsConfigQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_lpPageDataEvents[nPage].hEvent, GetCurrentProcess(), &hEvents[1], 0, false, DUPLICATE_SAME_ACCESS);
	lpReadDataCritSect= ((LPSUMMARYTHREADPARMS) lpParam)->pDataCritSect;
	delete ((LPSUMMARYTHREADPARMS) lpParam);

	while ( !g_bQuit && bSuccess && ((dwResult = ::WaitForSingleObject(hEvents[0], 0))==WAIT_TIMEOUT) )
	{
		dwResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE), hEvents, false, INFINITE);
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[0] || dwResult == WAIT_FAILED )
		{
			bResult = ::CloseHandle(hEvents[0]);
			bResult = ::CloseHandle(hEvents[1]);
			return 0;
		}
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[1] )
		{
			bResult = ::PostMessage(hParentWnd, WM_APPLIMCONFIGUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("Limits Config thread PostMessage error"), szTitle, MB_OK);
			}
		}
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
