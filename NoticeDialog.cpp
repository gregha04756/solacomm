#include "StdAfx.h"
#include "Resource.h"
#include "NoticeDialog.h"

CNoticeDialog::CNoticeDialog(void)
{
	this->m_szNoticeQuitEvent = NULL;
	this->m_hwndOwner = NULL;
	this->m_hwndNoticeDlg = NULL;
	this->m_hNoticeQuitEvent = NULL;
	this->m_hNoticeThread = NULL;
	this->m_hdupNoticeThread = NULL;
	this->m_hInst = (HINSTANCE) NULL;
	this->m_szTitle = NULL;
	this->m_dwNoticeThreadID = NULL;
	this->m_szNoticeTxt = NULL;
	this->m_hNoticeDlgRdyEvent = NULL;
}

CNoticeDialog::CNoticeDialog(HWND hOwner, HINSTANCE hInst,TCHAR* szTitle):m_hwndOwner(hOwner), m_hInst(hInst), m_szTitle(szTitle)
{
	this->m_szNoticeQuitEvent = _T("NoticeQuitEvent");
	this->m_szNoticeDlgRdyEvent = _T("NoticeDlgRdyEvent");
	this->m_hwndNoticeDlg = NULL;
	this->m_hNoticeQuitEvent = NULL;
	this->m_hNoticeThread = NULL;
	this->m_hdupNoticeThread = NULL;
	this->m_dwNoticeThreadID = NULL;
	this->m_szNoticeTxt = NULL;
	this->m_hNoticeDlgRdyEvent = NULL;
//	BOOL bResult = this->Start();
}

CNoticeDialog::~CNoticeDialog(void)
{
	BOOL bResult;
	if ( m_hNoticeThread )
	{
		Stop();
	}
	if ( this->m_hNoticeQuitEvent )
	{
		bResult = ::CloseHandle(m_hNoticeQuitEvent);
		m_hNoticeQuitEvent = NULL;
	}
	if ( this->m_hwndNoticeDlg )
	{
//		bResult = ::CloseHandle(m_hwndNoticeDlg);
		m_hwndNoticeDlg = NULL;
	}
	if ( m_szNoticeTxt )
	{
		delete m_szNoticeTxt;
		m_szNoticeTxt = NULL;
	}
}

INT_PTR CALLBACK CNoticeDialog::NoticeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	static CNoticeDialog* pnd;
	BOOL bResult;

	switch (message)
	{
	case WM_INITDIALOG:
		pnd = (CNoticeDialog*)lParam;
		bResult = ::SetEvent(pnd->m_hNoticeDlgRdyEvent);
		if ( pnd->m_szNoticeTxt )
		{
			::SetDlgItemText(hDlg,IDC_TXTNOTICE,pnd->m_szNoticeTxt);
		}
		return (INT_PTR)TRUE;

//	case WM_TIMER:
	case WM_APPENDNOTICETHREAD:
		bResult = ::DestroyWindow(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			bResult = ::DestroyWindow(hDlg);
			return (INT_PTR)TRUE;
		}

	case WM_DESTROY:
//		pnd->m_hwndNoticeDlg = NULL;
		::PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI CNoticeDialog::NoticeThreadProc(LPVOID lParam)
{
	MSG message;
	BOOL bResult;

	CNoticeDialog* pnd = (CNoticeDialog*)lParam;
	pnd->m_hwndNoticeDlg = NULL;
	
	if ( !::IsWindow(pnd->m_hwndNoticeDlg) && ::IsWindow(pnd->m_hwndOwner) ) 
	{ 
		pnd->m_hwndNoticeDlg = ::CreateDialogParam(pnd->m_hInst, MAKEINTRESOURCE(IDD_NOTICEBOX), pnd->m_hwndOwner, NoticeDlgProc,(LPARAM)pnd); 
		bResult = ::ShowWindow(pnd->m_hwndNoticeDlg, SW_SHOW); 

	while ((bResult = GetMessage(&message, NULL, 0, 0)) != 0) 
	{ 
		if (bResult == -1)
		{
        // Handle the error and possibly exit
			break;
		}
		else if (!IsWindow(pnd->m_hwndNoticeDlg) || !IsDialogMessage(pnd->m_hwndNoticeDlg,&message)) 
		{ 
			TranslateMessage(&message); 
			DispatchMessage(&message); 
		} 
	}
	}

	return 0;
}

DWORD WINAPI CNoticeDialog::SetTxtThreadProc(LPVOID lParam)
{
	BOOL bResult;
	HANDLE hEvents[2];

	CNoticeDialog* pnd = (CNoticeDialog*)lParam;

	bResult = ::DuplicateHandle(::GetCurrentProcess(),pnd->m_hNoticeQuitEvent,::GetCurrentProcess(),&hEvents[0],DUPLICATE_SAME_ACCESS,true,0);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),pnd->m_hNoticeDlgRdyEvent,::GetCurrentProcess(),&hEvents[1],DUPLICATE_SAME_ACCESS,true,0);

/*	DWORD dwResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,INFINITE);

		DWORD dwError = ::GetLastError();
		FormatMessage(	FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
						NULL,
						dwError,
						MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
						(LPWSTR) &lpErrMsg,
						MAX_PATH,
						NULL );
		StringCchPrintf( szErrMsg, sizeof(szErrMsg)/sizeof(TCHAR), _T("%d: %s"), dwError, lpErrMsg );
//		::MessageBox( NULL, (LPWSTR) szErrMsg, _T("Thread"), MB_OK );
		if ( lpErrMsg )
		{
			::LocalFree(lpErrMsg);
			lpErrMsg = NULL;
		}


	if ( dwResult != WAIT_FAILED && (dwResult - WAIT_OBJECT_0) != 0 )
	{
		if ( dwResult - WAIT_OBJECT_0 == 1 )
		{
			bResult = ::SetDlgItemText(pnd->m_hwndNoticeDlg,IDC_TXTNOTICE,pnd->m_szNoticeTxt);
		}
	} */

	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);

	return 0;
}

BOOL CNoticeDialog::Start()
{
	BOOL bSuccess = true;
	BOOL bResult;
	DWORD dwResult;
	int nResult;
	if ( m_hNoticeThread == NULL )
	{
		if ( m_hNoticeDlgRdyEvent )
		{
			bResult = ::CloseHandle(m_hNoticeDlgRdyEvent);
			m_hNoticeDlgRdyEvent = NULL;
		}
		m_hNoticeDlgRdyEvent = ::CreateEvent(NULL,true,false,m_szNoticeDlgRdyEvent);
		m_hNoticeQuitEvent = ::CreateEvent(NULL,true,false,m_szNoticeQuitEvent);
		m_hNoticeThread = ::CreateThread(NULL,0,NoticeThreadProc,(LPVOID)this,CREATE_SUSPENDED,&m_dwNoticeThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hNoticeThread,::GetCurrentProcess(),&m_hdupNoticeThread,DUPLICATE_SAME_ACCESS,true,0);
		dwResult = ::ResumeThread(m_hNoticeThread);
	}
	if ( m_hNoticeThread == NULL )
	{
		nResult = ::MessageBox(this->m_hwndOwner,_T("Error starting Notice thread"),m_szTitle,MB_OK);
	}
	return bSuccess;
}

BOOL CNoticeDialog::Stop()
{
	BOOL bResult;
	int nResult;

	if ( m_hNoticeThread )
	{
		bResult = ::SetEvent(m_hNoticeQuitEvent);
		if ( m_hwndNoticeDlg )
		{
			bResult = DeleteNoticeTxt();
			bResult = ::PostMessage(m_hwndNoticeDlg,WM_APPENDNOTICETHREAD,(WPARAM)0,(LPARAM)0);
			if ( ::WaitForSingleObject(m_hNoticeThread, 5000) == WAIT_TIMEOUT )
			{
				nResult = ::MessageBox(this->m_hwndOwner,_T("Error stopping Notice thread"),m_szTitle,MB_OK);
			}
		}
		bResult = ::CloseHandle(m_hNoticeThread);
		bResult = ::CloseHandle(m_hdupNoticeThread);
		m_hNoticeThread = NULL;
		m_hdupNoticeThread = NULL;
	}
	if ( m_hNoticeQuitEvent )
	{
		bResult = ::CloseHandle(m_hNoticeQuitEvent);
		m_hNoticeQuitEvent = NULL;
	}
	if ( m_hNoticeDlgRdyEvent )
	{
		bResult = ::CloseHandle(m_hNoticeDlgRdyEvent);
		m_hNoticeDlgRdyEvent = NULL;
	}
	return true;
}

int CNoticeDialog::SetNoticeTxt(TCHAR* szTxt)
{
	BOOL bResult;
	int cnt = 0;
	HRESULT hRes;
	if ( m_szNoticeTxt && szTxt )
	{
		delete m_szNoticeTxt;
		m_szNoticeTxt = NULL;
	}
	if ( szTxt )
	{
		while ( szTxt[cnt] != _T('\0') )
		{
			cnt++;
		}
//		cnt = sizeof(szTxt);
//		hRes = ::StringCchLength(szTxt,sizeof(szTxt)/sizeof(TCHAR),(size_t*)&cnt);
	}
	if ( szTxt && cnt )
	{
		m_szNoticeTxt = new TCHAR[cnt+1];
		hRes = ::StringCchPrintf(m_szNoticeTxt,cnt+1,_T("%s"),szTxt);
	}
	if ( m_szNoticeTxt && m_hNoticeThread )
	{
		if ( m_hwndNoticeDlg )
		{
			bResult = ::SetDlgItemText(m_hwndNoticeDlg,IDC_TXTNOTICE,m_szNoticeTxt);
			cnt = ::SetDlgItemText(m_hwndNoticeDlg,IDC_TXTNOTICE,m_szNoticeTxt) ? cnt : 0;
		}
//		else
//		{
//			m_hSetTxtThread = ::CreateThread(NULL,0,SetTxtThreadProc,(LPVOID)this,CREATE_SUSPENDED,&m_dwSetTxtThreadID);
//			bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hSetTxtThread,::GetCurrentProcess(),&m_hdupSetTxtThread,DUPLICATE_SAME_ACCESS,true,0);
//			dwResult = ::ResumeThread(m_hSetTxtThread);
//		}
	}
	return cnt;
}

BOOL CNoticeDialog::DeleteNoticeTxt(void)
{
	BOOL bSuccess = true;
	if ( m_hwndNoticeDlg && m_szNoticeTxt )
	{
		bSuccess = ::SetDlgItemText(m_hwndNoticeDlg,IDC_TXTNOTICE,_T(""));
		delete m_szNoticeTxt;
		m_szNoticeTxt = NULL;
	}
	return bSuccess;
}
