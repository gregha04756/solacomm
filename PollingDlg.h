#pragma once
#include "stdafx.h"
#include "SolaComm.h"

extern "C++" BOOL bSolaConnected;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gTimeCritSect;
extern "C++" DWORD g_dwTotalSent;
extern "C++" DWORD g_dwTotalRcvd;
extern "C++" DWORD g_dwTotalCRCErrors;
extern "C++" LARGE_INTEGER g_liPollTime;
extern "C++" DWORD g_dwConnectTime;
extern "C++" double g_dErrorRate;
extern "C++" HANDLE g_hReSyncReqEvent;

class CPollingDlg
{
public:
	CPollingDlg(void);
	CPollingDlg(HWND hOwner,HINSTANCE hInst,TCHAR* szTitle);
	~CPollingDlg(void);
	inline HWND GetHWNDPollingDlg(){return m_hwndPollingDlg;};
private:
	static DWORD WINAPI PollingDlgThreadProc(LPVOID lParam);
	static INT_PTR CALLBACK PollingDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
	HWND m_hwndOwner;
	HANDLE m_hPollingDlgQuitEvent;
	TCHAR* m_szPollingDlgQuitEvent;
	TCHAR* m_szPollingDlgRdyEvent;
	HWND m_hwndPollingDlg;
	HANDLE m_hPollingDlgRdyEvent;
	HANDLE m_hPollingDlgThread;
	HANDLE m_hdupPollingDlgThread;
	DWORD m_dwPollingDlgThreadID;
	HINSTANCE m_hInst;
	TCHAR* m_szTitle;
	BOOL Stop();
};

typedef struct _tagPollingDlgThreadParms {LPDWORD pdwTotalSent;
 LPDWORD pdwTotalRcvd;
 LPDWORD pdwTotalCRCErrors;
 LARGE_INTEGER* pliPollTime;
 CPollingDlg* ppd;} POLLINGDLGTHREADPARMS, *LPPOLLINGDLGTHREADPARMS;
