#include "stdafx.h"
#include "Resource.h"
#include "PollingDlg.h"
#include <cassert>

CPollingDlg::CPollingDlg(void)
{
	this->m_szPollingDlgQuitEvent = NULL;
	this->m_szPollingDlgRdyEvent = NULL;
	this->m_hPollingDlgQuitEvent = NULL;
	this->m_hwndOwner = NULL;
	this->m_hwndPollingDlg = NULL;
	this->m_hPollingDlgThread = NULL;
	this->m_hdupPollingDlgThread = NULL;
	this->m_hInst = (HINSTANCE) NULL;
	this->m_szTitle = NULL;
	this->m_dwPollingDlgThreadID = NULL;
	this->m_hPollingDlgRdyEvent = NULL;
}

CPollingDlg::CPollingDlg(HWND hOwner,HINSTANCE hInst,TCHAR* szTitle):m_hwndOwner(hOwner),m_hInst(hInst),m_szTitle(szTitle)
{
	BOOL bSuccess = true;
	BOOL bResult;
	DWORD dwResult;
	int nResult;
	this->m_szPollingDlgQuitEvent = _T("PollingDlgQuitEvent");
	this->m_szPollingDlgRdyEvent = _T("PollingDlgRdyEvent");
	this->m_hPollingDlgQuitEvent = NULL;
	this->m_hPollingDlgRdyEvent = NULL;
	this->m_hwndPollingDlg = NULL;
	this->m_hPollingDlgThread = NULL;
	this->m_hdupPollingDlgThread = NULL;
	this->m_dwPollingDlgThreadID = 0L;
	if ( m_hPollingDlgThread == NULL )
	{
		m_hPollingDlgQuitEvent = ::CreateEvent(NULL,true,false,m_szPollingDlgQuitEvent);
		assert(NULL != m_hPollingDlgQuitEvent);
		bResult = ::ResetEvent(m_hPollingDlgQuitEvent);
		m_hPollingDlgThread = ::CreateThread(NULL,0,PollingDlgThreadProc,(LPVOID)this,CREATE_SUSPENDED,&m_dwPollingDlgThreadID);
		dwResult = ::GetLastError();
		if ( m_hPollingDlgThread == NULL )
		{
			nResult = ::MessageBox(this->m_hwndOwner,_T("Error starting PollingDlg thread"),m_szTitle,MB_OK);
			ReportError(dwResult);
		}
		else
		{
			bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hPollingDlgThread,::GetCurrentProcess(),&m_hdupPollingDlgThread,DUPLICATE_SAME_ACCESS,true,0);
			dwResult = ::ResumeThread(m_hPollingDlgThread);
		}
	}
}

CPollingDlg::~CPollingDlg(void)
{
	BOOL bResult;
	if ( this->m_hPollingDlgThread )
	{
		Stop();
	}
	if ( this->m_hPollingDlgQuitEvent )
	{
		bResult = ::CloseHandle(m_hPollingDlgQuitEvent);
		m_hPollingDlgQuitEvent = NULL;
	}
}

INT_PTR CALLBACK CPollingDlg::PollingDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	PVOID p_v;
	static CPollingDlg* pnd;
	BOOL bResult;
	int i;
	int ndx;
	UINT uiResult;
	static UINT_PTR uipRefreshTimer;
	const UINT_PTR uipRTID = 1;
	const UINT uiTimes[] = {0,50,100,250,500,750,1000,1500,2000};
	TCHAR szTemp[32];
	LRESULT lRes;
	HRESULT hRes;
	LARGE_INTEGER lipt;
	DWORD hh;
	int mm;
	int ss;
	double dErrRate;
	double dSentRate;
	double dRecvRate;
	TCHAR strDataSent[32];
	TCHAR strDataRecv[32];

	switch (message)
	{
	case WM_INITDIALOG:
		pnd = (CPollingDlg*)lParam;
		for ( i = 0; i < sizeof(uiTimes)/sizeof(UINT); i++ )
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),uiTimes[i]);
			lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_POLLINTERVALCOMBO),CB_ADDSTRING,(WPARAM)0,(LPARAM)szTemp);
		}
		::EnterCriticalSection(&gCOMCritSect);
		lipt.QuadPart = g_liPollTime.QuadPart;
		::LeaveCriticalSection(&gCOMCritSect);
		uiResult = (UINT)(lipt.QuadPart/(-10000LL));
		ndx = 0;
		for ( i = 0; uiResult != uiTimes[i] && i < sizeof(uiTimes)/sizeof(UINT); i++ )
		{
		}
		ndx = (i < sizeof(uiTimes)/sizeof(UINT)) ? i : sizeof(uiTimes)/sizeof(UINT);
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_POLLINTERVALCOMBO),CB_SETCURSEL,(WPARAM)ndx,(LPARAM)0);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLINTERVALAPPLY),false);
		uipRefreshTimer = ::SetTimer(hDlg,uipRTID, pollInterval,NULL);
		::EnterCriticalSection(&gCOMCritSect);
		bResult = ::SetDlgItemInt(hDlg,IDC_POLLTOTALSENT,g_dwTotalSent,false);
		bResult = ::SetDlgItemInt(hDlg,IDC_POLLTOTALRCVD,g_dwTotalRcvd,false);
		bResult = ::SetDlgItemInt(hDlg,IDC_POLLTOTALERRORS,g_dwTotalCRCErrors,false);
		::LeaveCriticalSection(&gCOMCritSect);
		hh = 0L;
		mm = 0;
		ss = 0;
		::EnterCriticalSection(&gTimeCritSect);
		hh = g_dwConnectTime/3600;
		mm = (g_dwConnectTime-(hh*3600))/60;
		ss = g_dwConnectTime-(hh*3600)-(mm*60);
		dErrRate = g_dErrorRate;
		::LeaveCriticalSection(&gTimeCritSect);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%02d:%02d:%02d"),hh,mm,ss);
		bResult = ::SetDlgItemText(hDlg,IDC_POLLCONNECTTIME,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%6.1f/min"),dErrRate);
		bResult = ::SetDlgItemText(hDlg,IDC_POLLERRORRATE,szTemp);
		return (INT_PTR)TRUE;
	case WM_TIMER:
		p_v = SecureZeroMemory((PVOID)strDataSent, sizeof(strDataSent));
		p_v = SecureZeroMemory((PVOID)strDataRecv, sizeof(strDataRecv));
		::EnterCriticalSection(&gCOMCritSect);
		dSentRate = (double)g_dwTotalSent / (double)g_dwConnectTime;
		dRecvRate = (double)g_dwTotalRcvd / (double)g_dwConnectTime;
		hRes = ::StringCchPrintf(strDataSent, sizeof(strDataSent) / sizeof(TCHAR), _T("%ld %g/s"), g_dwTotalSent, dSentRate);
		bResult = ::SetDlgItemText(hDlg, IDC_POLLTOTALSENT, strDataSent);
		hRes = ::StringCchPrintf(strDataRecv, sizeof(strDataRecv) / sizeof(TCHAR), _T("%ld %g/s"), g_dwTotalRcvd, dRecvRate);
		bResult = ::SetDlgItemText(hDlg, IDC_POLLTOTALRCVD, strDataRecv);
		bResult = ::SetDlgItemInt(hDlg,IDC_POLLTOTALERRORS,g_dwTotalCRCErrors,false);
		::LeaveCriticalSection(&gCOMCritSect);
		::EnterCriticalSection(&gTimeCritSect);
		hh = g_dwConnectTime/3600;
		mm = (g_dwConnectTime-(hh*3600))/60;
		ss = g_dwConnectTime-(hh*3600)-(mm*60);
		dErrRate = g_dErrorRate;
		::LeaveCriticalSection(&gTimeCritSect);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%02d:%02d:%02d"),hh,mm,ss);
		bResult = ::SetDlgItemText(hDlg,IDC_POLLCONNECTTIME,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%6.1f/min"),dErrRate);
		bResult = ::SetDlgItemText(hDlg,IDC_POLLERRORRATE,szTemp);
		return (INT_PTR)TRUE;
	case WM_APPENDPOLLINGDLGTHREAD:
		if ( uipRefreshTimer )
		{
			bResult = ::KillTimer(hDlg,uipRTID);
		}
		bResult = ::DestroyWindow(hDlg);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDCANCEL )
		{
			bResult = ::PostMessage(pnd->m_hwndOwner,WM_APPQUITPOLLINGDLG,(WPARAM)0,(LPARAM)0);
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDOK )
		{
			i = (int)::SendDlgItemMessage(hDlg,IDC_POLLINTERVALCOMBO,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
			lRes = ::SendDlgItemMessage(hDlg,IDC_POLLINTERVALCOMBO,CB_GETLBTEXT,(WPARAM)i,(LPARAM)szTemp);
			i = ::_wtoi(szTemp);
			::EnterCriticalSection(&gCOMCritSect);
			g_liPollTime.QuadPart = (-10000LL)*i;
			::LeaveCriticalSection(&gCOMCritSect);
			bResult = ::PostMessage(pnd->m_hwndOwner,WM_APPQUITPOLLINGDLG,(WPARAM)0,(LPARAM)0);
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDC_POLLINTERVALCOMBO && HIWORD(wParam) == CBN_SELCHANGE )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLINTERVALAPPLY),true);
		}
		if ( LOWORD(wParam) == IDC_BTNPOLLINTERVALAPPLY )
		{
			i = (int)::SendDlgItemMessage(hDlg,IDC_POLLINTERVALCOMBO,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
			lRes = ::SendDlgItemMessage(hDlg,IDC_POLLINTERVALCOMBO,CB_GETLBTEXT,(WPARAM)i,(LPARAM)szTemp);
			i = ::_wtoi(szTemp);
			::EnterCriticalSection(&gCOMCritSect);
			g_liPollTime.QuadPart = (-10000LL)*i;
			::LeaveCriticalSection(&gCOMCritSect);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNPOLLINTERVALAPPLY),false);
			return (INT_PTR)TRUE;
		}
		if ( bSolaConnected && LOWORD(wParam) == IDC_BTNPOLLRESYNC )
		{
			bResult = ::SetEvent(g_hReSyncReqEvent);
		}
		break;
	case WM_DESTROY:
		::PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI CPollingDlg::PollingDlgThreadProc(LPVOID lParam)
{
	MSG message;
	BOOL bResult;

	CPollingDlg* pnd = (CPollingDlg*)lParam;
	pnd->m_hwndPollingDlg = NULL;
	
	if ( !::IsWindow(pnd->m_hwndPollingDlg) && ::IsWindow(pnd->m_hwndOwner) ) 
	{ 
		pnd->m_hwndPollingDlg = ::CreateDialogParam(pnd->m_hInst,MAKEINTRESOURCE(IDD_POLLINGDLG),pnd->m_hwndOwner,PollingDlgProc,(LPARAM)pnd); 
		bResult = ::ShowWindow(pnd->m_hwndPollingDlg,SW_SHOW); 

		while ((bResult = GetMessage(&message, NULL, 0, 0)) != 0) 
		{ 
			if (bResult == -1)
			{
	     // Handle the error and possibly exit
				break;
			}
			else if (!IsWindow(pnd->m_hwndPollingDlg) || !IsDialogMessage(pnd->m_hwndPollingDlg,&message)) 
			{ 
				TranslateMessage(&message); 
				DispatchMessage(&message); 
			} 
		}
	}
	return 0;
}

BOOL CPollingDlg::Stop()
{
	BOOL bResult;
	int nResult;

	if ( this->m_hPollingDlgThread )
	{
		bResult = ::SetEvent(this->m_hPollingDlgQuitEvent);
		if ( ::IsWindow(this->m_hwndPollingDlg) )
		{
			bResult = ::PostMessage(m_hwndPollingDlg,WM_APPENDPOLLINGDLGTHREAD,(WPARAM)0,(LPARAM)0);
			if ( ::WaitForSingleObject(m_hPollingDlgThread,5000) == WAIT_TIMEOUT )
			{
				nResult = ::MessageBox(this->m_hwndOwner,_T("Error stopping PollingDlg thread"),m_szTitle,MB_OK);
			}
		}
		bResult = ::CloseHandle(m_hPollingDlgThread);
		bResult = ::CloseHandle(m_hdupPollingDlgThread);
		m_hPollingDlgThread = NULL;
		m_hdupPollingDlgThread = NULL;
	}
	if ( m_hPollingDlgQuitEvent )
	{
		bResult = ::CloseHandle(m_hPollingDlgQuitEvent);
		m_hPollingDlgQuitEvent = NULL;
	}
	return true;
}
