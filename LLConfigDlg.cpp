#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"

CRITICAL_SECTION g_LLConfigCritSect;
const TCHAR* szLLConfigQuitEvent = {_T("LLConfigQuitEvent")};
const TCHAR* szLLConfigMBRWEvent = {_T("LLConfigMBRWEvent")};
HANDLE g_hLLConfigMBRWEvent;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD dwCommThreadID;
HANDLE g_hLLConfigQuitEvent;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveConfigPages;
extern "C++" CSolaMBMap* pcSystemConfiguration;
DWORD WINAPI LLConfigThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" SOLAMULTIVALUE RegisterAccess[];
extern "C++" SOLAMULTIVALUE ModbusExceptionCodes[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" CSolaMBMap* pcLLConfig;
extern "C++" CSolaMBMap* pcXLLConfig;
extern "C++" CSolaPage* pcLLConfigPage;
extern "C++" BOOL UpdatePage(HWND hwndDialog,CSolaPage* lpPage);
extern "C++" HWND g_hODResetLineWnd;
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);

extern "C++" INT_PTR CALLBACK UINumericDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIMultiDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UITimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIPercentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK LLConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bOkCRC;
	static int nPageIndex;
	static BOOL bStatus;
	BOOL bResult;
	LPNMHDR     lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	int k;
	int nResult;
	DWORD dwResult;
	static DWORD dwLLConfigThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hLLConfigThread;
	static HANDLE hLLConfigThreadDup;
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
	int nLeftScrollAmt;
	int nRightScrollAmt;
	static HWND hwndScrollLeftRightBar;
	RECT crectDlg;
	RECT crectUpdate;
	static int nLastPos;
	static SCROLLINFO si = { 0 };

	switch (uMessage)
	{
	case WM_INITDIALOG:
		::InitializeCriticalSection(&g_LLConfigCritSect);
		g_hLLConfigMBRWEvent = ::CreateEvent(NULL, false, false, szLLConfigMBRWEvent);
		bStatus = false;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_LLCONFIGDLG);
		g_hLLConfigQuitEvent = ::CreateEvent(NULL, true, false, szLLConfigQuitEvent);
		bResult = ::ResetEvent(g_hLLConfigQuitEvent);
		// Start LL Config update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hConfigChangeEvent;
		pSummaryParms->hQuitEvent = g_hLLConfigQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hLLConfigThread = ::CreateThread(NULL, 0, LLConfigThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwLLConfigThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hLLConfigThread, GetCurrentProcess(), &hLLConfigThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hLLConfigThread);
		for ( i = 0; i < pcLLConfig->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText( hDlg, LBLIDBASE+(pcLLConfig->GetStartRegAddr(i)), pcLLConfig->GetParmName(i));
		}
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveConfigPages++;
		g_lpPageDataEvents[nPageIndex].typePage = ConfigPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected )
		{
			bResult = ::PostMessage(hDlg, WM_APPLLCONFIGUPD, (WPARAM)1, (LPARAM)0);
		}
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPLLCONFIGUPD;
		nControlIDMin = TXTIDBASE + 0x0fff;
		nControlIDMax = TXTIDBASE;
		for ( i = 0; i < pcLLConfigPage->GetSize(); i++ )
		{
			nResult = TXTIDBASE + pcLLConfigPage->ItemMap(i)->GetStartRegAddr(pcLLConfigPage->ItemIndex(i));
			nControlIDMin = nControlIDMin < nResult ? nControlIDMin : nResult;
			nControlIDMax = nControlIDMax > nResult ? nControlIDMax : nResult;
		}
		bResult = ::GetClientRect(hDlg,&crectDlg);
		hwndScrollLeftRightBar = ::CreateWindowEx(	WS_EX_LEFT | SBS_HORZ,
												WC_SCROLLBAR,
												NULL,
												WS_CHILD | WS_VISIBLE,
												crectDlg.left,
												0,
												2*(crectDlg.right-crectDlg.left),
												15,
												hDlg,
												NULL,
												g_hInst,
												NULL);
		if ( (dwResult = ::GetLastError() != NOERROR ) )
		{
			ReportError(dwResult);
		}
		if ( hwndScrollLeftRightBar )
		{
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			bResult = ::GetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si);
			si.fMask = SIF_ALL;
			si.nMax = 3;
			si.nMin = 0;
			si.nPage = 1;
			si.nPos = 0;
			nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
		}
		nLastPos = 0;
		Make_Reg_Group_List(pcLLConfigPage);
		break;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcLLConfigPage);
		bResult = UpdatePage(hDlg,pcLLConfigPage);
		if ( ::g_hODResetLineWnd )
		{
			bResult = ::PostMessage(g_hODResetLineWnd,WM_APPODRESETCFGUPD,(WPARAM)0,(LPARAM)0);
		}
		break;
	case WM_APPLLCONFIGUPD:
		bResult = UpdatePage(hDlg,pcLLConfigPage);
		if ( ::g_hODResetLineWnd )
		{
			bResult = ::PostMessage(g_hODResetLineWnd,WM_APPODRESETCFGUPD,(WPARAM)0,(LPARAM)0);
		}
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
	case WM_HSCROLL:
		if ( (HWND)lParam == hwndScrollLeftRightBar )
		{
			switch ( LOWORD(wParam) )
			{
			case SB_THUMBPOSITION:
				if ( HIWORD(wParam) > nLastPos )
				{
					bResult = ::GetWindowRect(hDlg,&crectDlg);
//					nRightScrollAmt = crectDlg.left - crectDlg.right;
					nRightScrollAmt = ((HIWORD(wParam)-nLastPos)*(crectDlg.left - crectDlg.right))/3;
					nResult = ::ScrollWindowEx(hDlg,nRightScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
					si.fMask = SIF_POS;
					si.nPos = HIWORD(wParam);
					if ( si.nPos > si.nMax )
					{
						si.nPos = si.nMax;
					}
					nLastPos = si.nPos;
					nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
					break;
				}
				if ( HIWORD(wParam) < nLastPos )
				{
					bResult = ::GetWindowRect(hDlg,&crectDlg);
//					nLeftScrollAmt = crectDlg.right - crectDlg.left;
					nLeftScrollAmt = ((nLastPos-HIWORD(wParam))*(crectDlg.right - crectDlg.left))/3;
					nResult = ::ScrollWindowEx(hDlg,nLeftScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
					si.fMask = SIF_POS;
					si.nPos = HIWORD(wParam);
					if ( si.nPos < si.nMin )
					{
						si.nPos = si.nMin;
					}
					nLastPos = si.nPos;
					nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				}
				break;
			case SB_PAGELEFT:
				bResult = ::GetWindowRect(hDlg,&crectDlg);
//				nLeftScrollAmt = crectDlg.right - crectDlg.left;
				nLeftScrollAmt = (crectDlg.right - crectDlg.left)/3;
				nResult = ::ScrollWindowEx(hDlg,nLeftScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
				switch (nResult)
				{
				case SIMPLEREGION:
					break;
				case COMPLEXREGION:
					break;
				case NULLREGION:
					break;
				case ERROR:
				default:
					dwResult = ::GetLastError();
					break;
				}
				si.fMask = SIF_POS;
				si.nPos--;
				if ( si.nPos < si.nMin )
				{
					si.nPos = si.nMin;
				}
				nLastPos = si.nPos;
				nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				break;
			case SB_PAGERIGHT:
				bResult = ::GetWindowRect(hDlg,&crectDlg);
//				nRightScrollAmt = crectDlg.left - crectDlg.right;
				nRightScrollAmt = (crectDlg.left - crectDlg.right)/3;
				nResult = ::ScrollWindowEx(hDlg,nRightScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
				switch (nResult)
				{
				case SIMPLEREGION:
					break;
				case COMPLEXREGION:
					break;
				case NULLREGION:
					break;
				case ERROR:
				default:
					dwResult = ::GetLastError();
					break;
				}
				si.fMask = SIF_POS;
				si.nPos++;
				if ( si.nPos > si.nMax )
				{
					si.nPos = si.nMax;
				}
				nLastPos = si.nPos;
				nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				break;
			}
		}
		break;
	case WM_COMMAND:
		if ( (HIWORD(wParam) == STN_CLICKED) && (LOWORD(wParam) >= nControlIDMin && LOWORD(wParam) <= nControlIDMax) )
		{
			ipResult = IDCANCEL;
			usRegAddr = (unsigned short)(LOWORD(wParam)-TXTIDBASE);
			stType = CSolaMBMap::Novalue;
			lpMap = NULL;
			for ( i = 0; i < pcLLConfigPage->GetSize() && !lpMap; i++ )
			{
				if ( pcLLConfigPage->ItemMap(i)->GetStartRegAddr(pcLLConfigPage->ItemIndex(i)) == usRegAddr )
				{
					lpMap = pcLLConfigPage->ItemMap(i);
				}
			}
			stType = (lpMap == NULL) ? CSolaMBMap::Novalue : lpMap->GetType(usRegAddr);
			switch (stType)
			{
			case CSolaMBMap::Temperature:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
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
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Multivalue:
				lpMUIParms = new MULTIUIPARMS;
				lpMUIParms->lpMultiList = lpMap->GetLPMulti(usRegAddr);
				lpMUIParms->nCurSel = lpMap->GetValue(usRegAddr);
				lpMUIParms->nMultiListSize = lpMap->GetMultiListSize(usRegAddr);
				lpMUIParms->szParmName = lpMap->GetParmName(usRegAddr);
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
			case CSolaMBMap::Minutes:
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
				if ( hLLConfigThread )
				{
					bResult = ::SetEvent(g_hLLConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hLLConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for LL Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLLConfigThread);
					bResult = ::CloseHandle(hLLConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hLLConfigQuitEvent);
				bResult = ::CloseHandle(g_hLLConfigMBRWEvent);
				::DeleteCriticalSection(&g_LLConfigCritSect);
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLLConfigThread )
				{
					bResult = ::SetEvent(g_hLLConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hLLConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for LL Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLLConfigThread);
					bResult = ::CloseHandle(hLLConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hLLConfigQuitEvent);
				bResult = ::CloseHandle(g_hLLConfigMBRWEvent);
				::DeleteCriticalSection(&g_LLConfigCritSect);
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

DWORD WINAPI LLConfigThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hLLConfigQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
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
			bResult = ::PostMessage(hParentWnd, WM_APPLLCONFIGUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("LL Config thread PostMessage error"), szTitle, MB_OK);
			}
		}
//		bResult = ::SetEvent(::g_hPageUpdEvents[nPage]);
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
