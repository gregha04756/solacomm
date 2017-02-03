#pragma once

#include "SolaComm.h"
//#define WM_APPENDNOTICETHREAD	WM_APP + 125

class CNoticeDialog
{
public:
	CNoticeDialog(void);
	CNoticeDialog(HWND hOwner, HINSTANCE hInst, TCHAR* szTitle);
	~CNoticeDialog(void);
	inline TCHAR* GetNoticeQuitEventTitle(){return m_szNoticeQuitEvent;};
	BOOL Start(void);
	BOOL Stop(void);
	int SetNoticeTxt(TCHAR* szTxt);
	BOOL DeleteNoticeTxt(void);
private:
	static DWORD WINAPI NoticeThreadProc(LPVOID lParam);
	static DWORD WINAPI SetTxtThreadProc(LPVOID lParam);
	static INT_PTR CALLBACK NoticeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	HWND m_hwndOwner;
	HANDLE m_hNoticeQuitEvent;
	TCHAR* m_szNoticeQuitEvent;
	TCHAR* m_szNoticeTxt;
	TCHAR* m_szNoticeDlgRdyEvent;
	HWND m_hwndNoticeDlg;
	HANDLE m_hNoticeDlgRdyEvent;
	HANDLE m_hNoticeThread;
	HANDLE m_hdupNoticeThread;
	HANDLE m_hSetTxtThread;
	HANDLE m_hdupSetTxtThread;
	DWORD m_dwNoticeThreadID;
	DWORD m_dwSetTxtThreadID;
	HINSTANCE m_hInst;
	TCHAR* m_szTitle;
};
