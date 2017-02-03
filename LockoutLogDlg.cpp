#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaLockout.h"
#include "SolaMultiValue.h"
//#include "LockoutRecordDisplay.h"
//#include "SolaLockoutDesc.h"

using namespace std;

const TCHAR* szLockoutLogQuitEvent = {_T("LockoutLogQuitEvent")};
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hStatusChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
HANDLE g_hLockoutLogQuitEvent;
extern "C++" TCHAR szTitle[];
DWORD WINAPI LockoutLogThread(LPVOID lParam);
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveStatusPages;
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" CRITICAL_SECTION g_UpdCountCS;
extern "C++" int nUpdCount;
extern "C++" CSolaMultiValue* pcSolaLockoutDesc;
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" CSolaMBMap* pcBurnerControlStatus;
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" unsigned char SOLAMBAddress;
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" INT_PTR CALLBACK LockoutRecordDisplayDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);

LRESULT CALLBACK LockoutLogDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	LRESULT lRes;
	HRESULT hRes;
	INT_PTR ipResult;
	static int nPageIndex;
	LPPSHNOTIFY lppsn;
	LPNMHDR lpnmhdr;
	static BOOL bStatus;
	static BOOL bSuccess;
	static BOOL bOkCRC;
	BOOL bResult;
	int i;
	unsigned short usBit;
	DWORD dwResult;
	static DWORD dwLockoutLogThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hLockoutLogThread;
	static HANDLE hLockoutLogThreadDup;
	TCHAR szLockoutRecord[MAX_LOADSTRING];
	TCHAR szUpdCount[MAX_LOADSTRING];
	unsigned long ulCycle;
//	static CLockoutRecordDisplay* clrd;
	unsigned short usRegAddr;
	const U16 usResetLockout = 1;
	static MBSNDRCVREQ MBSndRcvReq;
	static unsigned char* pchToSnd;
	static unsigned char* pchEndSnd;
	static unsigned char* pchToRcv;
	static unsigned char* pchEndRcv;
//	static CSolaMultiValue* pSolaLockoutDesc;
	static LPLOCKOUTRECORDDSPPARMS lpParms;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		bSuccess = true;
//		try
//		{
//			clrd = (CLockoutRecordDisplay*)new CLockoutRecordDisplay(hDlg,g_hInst,szTitle,pcSolaLockoutDesc);
//		}
//		catch (std::bad_alloc &ba)
//		{
//			ReportError(ba.what());
//		}
		lpParms = NULL;
		hRes = ::StringCchPrintf(szUpdCount, sizeof(szUpdCount)/sizeof(TCHAR), _T("%d"), nUpdCount);
		bResult = ::SetDlgItemText(hDlg, IDC_UPDCOUNT, szUpdCount);
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_LOCKOUTLOGDLG);
		if ( bSuccess )
		{
			// Create Lockout Log Quit event
			g_hLockoutLogQuitEvent = ::CreateEvent(NULL, true, false, szLockoutLogQuitEvent);
			bResult = ::ResetEvent(g_hLockoutLogQuitEvent);
			// Start Digital I/O thread
			pSummaryParms = new SUMMARYTHREADPARMS;
			pSummaryParms->hParentWnd = hDlg;
			pSummaryParms->pDataCritSect = &gRWDataCritSect;
			pSummaryParms->hReadEvent = g_hStatusChangeEvent;
			pSummaryParms->hQuitEvent = g_hLockoutLogQuitEvent;
			pSummaryParms->nPg = nPageIndex;
			hLockoutLogThread = ::CreateThread(NULL, 0, LockoutLogThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwLockoutLogThreadID);
			bResult = ::DuplicateHandle(::GetCurrentProcess(), hLockoutLogThread, GetCurrentProcess(), &hLockoutLogThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
			dwResult = ::ResumeThread(hLockoutLogThread);
		}
		usBit = 1;
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveStatusPages++;
		g_lpPageDataEvents[nPageIndex].typePage = StatusPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected && (pcBurnerControlStatus->GetValue((unsigned short)0x0022) != 0) )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),true);
		}
		else
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),false);
		}
		break;
	case WM_CHILDACTIVATE:
		if ( bSolaConnected )
		{
			lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_LOCKOUTLIST), LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
			for ( i = 0; !g_bQuit && i < pcLockoutLog->GetSize(); i++ )
			{
				if ( pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode != 0 )
				{
					usBit = pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode;
					ulCycle = pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.ulCycle;
					hRes = ::StringCchPrintf(szLockoutRecord, sizeof(szLockoutRecord)/sizeof(TCHAR), _T("%03d                   %ld                %s"),
						pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode,
						pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.ulCycle,
						pcSolaLockoutDesc->GetMultiString(pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode));
//						LockoutDescriptions[pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode].szLockoutText);
					lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_LOCKOUTLIST), LB_ADDSTRING, (WPARAM) 0, (LPARAM) szLockoutRecord);
				}
			}
		}
		if ( bSolaConnected && (pcBurnerControlStatus->GetValue((unsigned short)0x0022) != 0) )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),true);
		}
		else
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),false);
		}
		break;
	case WM_APPLOCKOUTUPD:
		::EnterCriticalSection(&g_UpdCountCS);
		nUpdCount--;
		hRes = ::StringCchPrintf(szUpdCount, sizeof(szUpdCount)/sizeof(TCHAR), _T("%d"), nUpdCount);
		bResult = ::SetDlgItemText(hDlg, IDC_UPDCOUNT, szUpdCount);
		::LeaveCriticalSection(&g_UpdCountCS);
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_LOCKOUTLIST), LB_RESETCONTENT, (WPARAM) 0, (LPARAM) 0);
		for ( i = 0; !g_bQuit && i < pcLockoutLog->GetSize(); i++ )
		{
			if ( pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode != 0 )
			{
				usBit = pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode;
				ulCycle = pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.ulCycle;
				hRes = ::StringCchPrintf(szLockoutRecord, sizeof(szLockoutRecord)/sizeof(TCHAR), _T("%03d                   %ld                %s"),
					pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode,
					pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.ulCycle,
						pcSolaLockoutDesc->GetMultiString(pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode));
//					LockoutDescriptions[pcLockoutLog->GetLPMap(i)->pLockoutUnion->slr.usLockoutCode].szLockoutText);
				lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_LOCKOUTLIST), LB_ADDSTRING, (WPARAM) 0, (LPARAM) szLockoutRecord);
			}
		}
		if ( bSolaConnected )
		{
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		}
		if ( bSolaConnected && (pcBurnerControlStatus->GetValue((unsigned short)0x0022) != 0) )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),true);
		}
		else
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNCLRLOCKOUT),false);
		}
		break;
	case WM_DRAWITEM:
		break;
   // on any command notification, tell the property sheet to enable the Apply button
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDC_LOCKOUTLIST && HIWORD(wParam) == LBN_SELCHANGE )
		{
			lRes = ::SendMessage((HWND)lParam,LB_GETCURSEL,(WPARAM)0,(LPARAM)0);
//			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%ld"),lRes);
			if ( lRes != LB_ERR )
			{
//				if ( !clrd->LockoutRecordDisplayIsActive() )
//				{
//					i = ::MessageBox(::GetParent(hDlg),szTemp,szTitle,MB_OK);
//					bResult = clrd->Start(pcLockoutLog,(int)lRes);
				lpParms = (LPLOCKOUTRECORDDSPPARMS)new LockoutRecordDspParms;
				lpParms->nIndex = (int)lRes;
					ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_LOCKOUTRECORDDLG),hDlg,LockoutRecordDisplayDlgProc,(LPARAM)lpParms); 
//				}
			}
			else
			{
				dwResult = ::GetLastError();
				if ( ERROR_SUCCESS == dwResult )
				{
					::MessageBox(hDlg,_T("Ambiguous selection, please re-try"),szTitle,MB_OK);
				}
				else
				{
					ReportError(dwResult);
				}
			}
		}
		if ( bSolaConnected && LOWORD(wParam) == IDC_BTNCLRLOCKOUT && HIWORD(wParam) == BN_CLICKED )
		{
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x06;
				usRegAddr = 0x101F;
				*(*MBSndRcvReq.ppchEndSnd)++ = (usRegAddr >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = usRegAddr & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = (usResetLockout >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = usResetLockout & 0x00ff;
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
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
				i = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
			}
		}
		break;
	case WM_NOTIFY:
//		lpnmhdr = (NMHDR FAR *)lParam;
//		lpnmhdr = (LPNMHDR)&(lppsn->hdr);
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLockoutLogThread )
				{
					bResult = ::SetEvent(g_hLockoutLogQuitEvent);
					dwResult = ::WaitForSingleObject(hLockoutLogThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Lockout Log update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLockoutLogThreadDup);
					bResult = ::CloseHandle(hLockoutLogThread);
					bResult = ::CloseHandle(g_hLockoutLogQuitEvent);
				}
//				if ( clrd )
//				{
//					delete clrd;
//				}
//				if ( pSolaLockoutDesc )
//				{
//					delete pSolaLockoutDesc;
//				}
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLockoutLogThread )
				{
					bResult = ::SetEvent(g_hLockoutLogQuitEvent);
					dwResult = ::WaitForSingleObject(hLockoutLogThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Lockout Log update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLockoutLogThreadDup);
					bResult = ::CloseHandle(hLockoutLogThread);
					bResult = ::CloseHandle(g_hLockoutLogQuitEvent);
				}
//				if ( clrd )
//				{
//					delete clrd;
//				}
//				if ( pSolaLockoutDesc )
//				{
//					delete pSolaLockoutDesc;
//				}
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

DWORD WINAPI LockoutLogThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hLockoutLogQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
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
			bResult = ::PostMessage(hParentWnd, WM_APPLOCKOUTUPD, (WPARAM) 0, (LPARAM) 0);
			::EnterCriticalSection(&g_UpdCountCS);
			nUpdCount = (bResult) ? ++nUpdCount : nUpdCount;
			::LeaveCriticalSection(&g_UpdCountCS);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("Lockout thread PostMessage error"), szTitle, MB_OK);
			}
		}

	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
