#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "SolaPage.h"


const TCHAR* szDigitalIOQuitEvent = {_T("DigitalIOQuitEvent")};
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hReadEvent;
//extern "C++" HANDLE g_hStatusChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
HANDLE g_hDigitalIOQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" CSolaMBMap* pcSystemStatus;
DWORD WINAPI DigitalIOThread(LPVOID lParam);
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveTrendPages;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" CSolaMultiValue* pcDigitalIOCodes;
extern "C++" CSolaMBMaps* pcAnnuncMaps;
extern "C++" CSolaPage* pc_Digital_IO_Page;
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);

LRESULT CALLBACK DigitalIODlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	static int nPageIndex;
	LPPSHNOTIFY lppsn;
	LPNMHDR lpnmhdr;
	static BOOL bStatus;
	static BOOL bSuccess;
	BOOL bResult;
	int i;
	int j;
	int nResult;
	unsigned short usBit;
	DWORD dwResult;
	static DWORD dwDigitalIOThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hDigitalIOThread;
	static HANDLE hDigitalIOThreadDup;
	PVOID pvTemp;
	HDC hdc;
	HGDIOBJ hOldBrush;
	COLORREF OldBrushColor;
	RECT arect;
	TCHAR szTemp[64];
	char* pchAnnuncStr;
	LPCSTR pchName;
	int cbNameLen;
	SOLAANNUNCUNION Annunciator;
	unsigned short usResult;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		bSuccess = true;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_DIGITALIODLG);
		if ( bSuccess )
		{
			// Create Summary Quit event
			g_hDigitalIOQuitEvent = ::CreateEvent(NULL, true, false, szDigitalIOQuitEvent);
			bResult = ::ResetEvent(g_hDigitalIOQuitEvent);
			// Start Digital I/O thread
			pSummaryParms = new SUMMARYTHREADPARMS;
			pSummaryParms->hParentWnd = hDlg;
			pSummaryParms->pDataCritSect = &gRWDataCritSect;
			pSummaryParms->hReadEvent = g_hReadEvent;
			pSummaryParms->hQuitEvent = g_hDigitalIOQuitEvent;
			pSummaryParms->nPg = nPageIndex;
			hDigitalIOThread = ::CreateThread(NULL, 0, DigitalIOThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwDigitalIOThreadID);
			bResult = ::DuplicateHandle(::GetCurrentProcess(), hDigitalIOThread, GetCurrentProcess(), &hDigitalIOThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
			dwResult = ::ResumeThread(hDigitalIOThread);
		}
		usBit = 1;
		for ( i = 0; i < 16; i++ )
		{
			bResult = ::SetDlgItemText(hDlg, LBLIDBASE+usBit, pcDigitalIOCodes->GetMultiString(i));
			usBit <<= 1;
		}
		usBit = 1;
		for ( i = 1; i < pcAnnuncMaps->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText(hDlg, LBLIDBASE+0x4000+usBit, pcAnnuncMaps->GetLPMap(i)->GetParmName((int)0));
			usBit <<= 1;
			if ( !bSolaConnected )
			{
				if ( pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->pchStr != NULL )
				{
					pvTemp = ::SecureZeroMemory((PVOID)pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->pchStr,
						(size_t)pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->cbStrLen);
				}
			}
		}
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveTrendPages++;
		g_lpPageDataEvents[nPageIndex].typePage = TrendPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		Make_Reg_Group_List(pc_Digital_IO_Page);
		break;
	case WM_CHILDACTIVATE:
		Make_Reg_Group_List(pc_Digital_IO_Page);
	case WM_APPTRENDUPD:
		usBit = 1;
		for ( i = 1; i <= pcDigitalIOCodes->GetSize(); i++ )
		{
			hdc = GetDC(GetDlgItem(hDlg,i));
			bResult = ::GetClientRect(GetDlgItem(hDlg,i),&arect);
			hOldBrush = SelectObject(hdc,GetStockObject(DC_BRUSH));
			if ( usBit & (pcSystemStatus->GetValue((int)2)) )
			{
				OldBrushColor = SetDCBrushColor(hdc,RGB(0x00,0xff,0x00));
			}
			else
			{
				OldBrushColor = SetDCBrushColor(hdc,RGB(0xff,0x00,0x00));
			}
			bResult = ::Rectangle(hdc, arect.left, arect.top, arect.right, arect.bottom);
			hOldBrush = SelectObject(hdc,hOldBrush);
			OldBrushColor = SetDCBrushColor(hdc,OldBrushColor);
			nResult = ::ReleaseDC(GetDlgItem(hDlg,i),hdc);
			usBit <<= 1;
		}
// Annunciators
		usBit = 1;
		for ( i = 1; i < pcAnnuncMaps->GetSize(); i++ )
		{
			hdc = GetDC(GetDlgItem(hDlg,i+0x4100));
			bResult = ::GetClientRect(GetDlgItem(hDlg,i+0x4100),&arect);
			hOldBrush = SelectObject(hdc,GetStockObject(DC_BRUSH));
			if ( usBit & (pcSystemStatus->GetValue((int)3)) )
			{
				OldBrushColor = SetDCBrushColor(hdc,RGB(0x00,0xff,0x00));
			}
			else
			{
				OldBrushColor = SetDCBrushColor(hdc,RGB(0xff,0x00,0x00));
			}
			bResult = ::Rectangle(hdc, arect.left, arect.top, arect.right, arect.bottom);
			hOldBrush = SelectObject(hdc,hOldBrush);
			OldBrushColor = SetDCBrushColor(hdc,OldBrushColor);
			nResult = ::ReleaseDC(GetDlgItem(hDlg,i+0x4100),hdc);
			pchAnnuncStr = (char*)pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->pchStr;
			usResult = 0;
			cbNameLen = 0;
			pchName = NULL;
			if ( pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->cbStrLen == sizeof(SOLAANNUNCRECORD) )
			{
				for ( j = 0; j < sizeof(SOLAANNUNCRECORD); j++ )
				{
					Annunciator.cc[j] = pchAnnuncStr[j];
				}
				usResult = ((Annunciator.aa.usLocation & 0x00ff)*256)+((Annunciator.aa.usLocation>>8)&0x00ff);
				Annunciator.aa.usLocation = usResult;
				pchName = reinterpret_cast<LPCSTR>(Annunciator.aa.uchName);
				cbNameLen = sizeof(Annunciator.aa.uchName);
			}
			if ( pcAnnuncMaps->GetLPMap(i)->GetLPMap(0)->cbStrLen == sizeof(SOLAANNUNCSHORT) )
			{
				for ( j = 0; j < sizeof(SOLAANNUNCRECORD); j++ )
				{
					Annunciator.cc[j] = pchAnnuncStr[j];
				}
				usResult = 4;
				pchName = reinterpret_cast<LPCSTR>(Annunciator.as.uchName);
				cbNameLen = sizeof(Annunciator.as.uchName);
			}
			if ( usResult > 0 && cbNameLen > 0 && pchName != NULL )
			{
				nResult = ::MultiByteToWideChar(CP_ACP,
					MB_PRECOMPOSED,
					(LPCSTR)pchName,
					cbNameLen,
					szTemp,
					sizeof(szTemp)/sizeof(TCHAR));
					bResult = ::SetDlgItemText(hDlg, LBLIDBASE+0x4000+usBit, szTemp);
			}
			usBit <<= 1;
		}
		bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		break;
	case WM_DRAWITEM:
		break;
	case WM_COMMAND:
		break;

	case WM_NOTIFY:
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hDigitalIOThread )
				{
					bResult = ::SetEvent(g_hDigitalIOQuitEvent);
					dwResult = ::WaitForSingleObject(hDigitalIOThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Digital I/O update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hDigitalIOThread);
					bResult = ::CloseHandle(hDigitalIOThreadDup);
					bResult = ::CloseHandle(g_hDigitalIOQuitEvent);
				}
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hDigitalIOThread )
				{
					bResult = ::SetEvent(g_hDigitalIOQuitEvent);
					dwResult = ::WaitForSingleObject(hDigitalIOThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Digital I/O update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hDigitalIOThread);
					bResult = ::CloseHandle(hDigitalIOThreadDup);
					bResult = ::CloseHandle(g_hDigitalIOQuitEvent);
				}
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

DWORD WINAPI DigitalIOThread(LPVOID lpParam)
{
	BOOL bResult;
	BOOL bSuccess = true;
	int i;
	int nPage;
	DWORD dwResult;
	HANDLE hEvents[2];
	HWND hParentWnd;
	LPCRITICAL_SECTION lpReadDataCritSect;

	hParentWnd = ((LPSUMMARYTHREADPARMS) lpParam)->hParentWnd;
	nPage = ((LPSUMMARYTHREADPARMS) lpParam)->nPg;
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hDigitalIOQuitEvent,GetCurrentProcess(),&hEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_lpPageDataEvents[nPage].hEvent,GetCurrentProcess(),&hEvents[1],0,false,DUPLICATE_SAME_ACCESS);
	lpReadDataCritSect = ((LPSUMMARYTHREADPARMS)lpParam)->pDataCritSect;
	delete ((LPSUMMARYTHREADPARMS)lpParam);

	while ( !g_bQuit && bSuccess && ((dwResult = ::WaitForSingleObject(hEvents[0],0))==WAIT_TIMEOUT) )
	{
		dwResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,INFINITE);
		if ( dwResult == WAIT_FAILED )
		{
			break;
		}
		if ( dwResult - WAIT_OBJECT_0 < sizeof(hEvents)/sizeof(HANDLE) )
		{
//			if ( hEvents[dwResult-WAIT_OBJECT_0] == g_hDigitalIOQuitEvent )
			if ( 0 == dwResult-WAIT_OBJECT_0 )
			{
				break;
			}
			if ( 1 == dwResult-WAIT_OBJECT_0 )
			{
				bResult = ::PostMessage(hParentWnd,WM_APPTRENDUPD,(WPARAM)0,(LPARAM)0);
				if ( !bResult )
				{
					::MessageBox(hParentWnd, _T("Digital I/O update thread PostMessage error"), szTitle, MB_OK);
				}
			}
		}
	}
	for ( i = 0; i < sizeof(hEvents)/sizeof(HANDLE); i++ )
	{
		bResult = ::CloseHandle(hEvents[i]);
	}
	return 0;
}
