#include "stdafx.h"
#include "solacomm.h"

extern "C++" CRITICAL_SECTION gTimeCritSect;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" TCHAR szTimeString[32];
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD g_dwConnectTime;
extern "C++" double g_dErrorRate;
extern "C++" DWORD g_dwTotalCRCErrors;


DWORD WINAPI TimerThread(LPVOID lpParam)
{
	BOOL b_r = TRUE;
	HRESULT hRes;
	int nCtr = 0;
	HWND hParentWnd;
	HANDLE hEventHandle;
	DWORD dwWaitResult;
	DWORD dw1SecCounter = g_dw1SecTimerInterval;
	SYSTEMTIME timestruct;
	int cchTimeString;
	double dCRCErrors;

	LPTimerThreadParms pParms = (LPTimerThreadParms) lpParam;
	hParentWnd = pParms->hParentWnd;
	hEventHandle = pParms->hQuitEvent;
	cchTimeString = pParms->cchTimeString;
	delete pParms;

	while ( (dwWaitResult = ::WaitForSingleObject(hEventHandle, g_dwTimerInterval)) == WAIT_TIMEOUT )
	{
		b_r = ::PostMessage(hParentWnd, WM_APPDATASAVETIMER, 0, (LPARAM)szTimeString);
		if (0 == dw1SecCounter)
		{
			::GetLocalTime(&timestruct);
			::EnterCriticalSection(&gCOMCritSect);
			dCRCErrors = (double)g_dwTotalCRCErrors;
			::LeaveCriticalSection(&gCOMCritSect);
			::EnterCriticalSection(&gTimeCritSect);
			hRes = ::StringCchPrintf(szTimeString,
				cchTimeString,
				_T("%d/%02d/%02d %02d:%02d:%02d"),
				timestruct.wYear,
				timestruct.wMonth,
				timestruct.wDay,
				timestruct.wHour,
				timestruct.wMinute,
				timestruct.wSecond);
			b_r = ::PostMessage(hParentWnd, WM_APPTIMER1, 0, (LPARAM)szTimeString);
			g_dwConnectTime += bSolaConnected;
			g_dErrorRate = (((double)g_dwConnectTime / 60.0L) > 0.0L) ? (dCRCErrors * 60.0L) / (double)g_dwConnectTime : 0.0L;
			::LeaveCriticalSection(&gTimeCritSect);
			if (++nCtr >= 5)
			{
				nCtr = 0;
				b_r = ::PostMessage(hParentWnd, WM_APPTIMER5, 0, 0);
			}
		}
		dw1SecCounter = 0 < dw1SecCounter ? dw1SecCounter-g_dwTimerInterval : g_dw1SecTimerInterval;
	}
	return 0;
}