#include "stdafx.h"
#include "solacomm.h"
//#include "resource.h"

extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bCommThreadActive;

DWORD WINAPI ConnUpdThread( LPVOID lpParam )
{
	DWORD dwElapsed = 0;
	MSG mess;
	HRESULT hRes;
	BOOL bResult;
	BOOL bSuccess = true;
	BOOL* lpbStatus;
	int i = 0;
	int j;
	TCHAR szTemp[MAX_LOADSTRING];

	LPCONNUPDTHREADPARMS pConnUpdParms = (LPCONNUPDTHREADPARMS) lpParam;
	HWND hParentWnd = pConnUpdParms->hParentWnd;
	UINT uID = pConnUpdParms->uID;
	lpbStatus = pConnUpdParms->lpbStatus;
	delete pConnUpdParms;

	while ( bSuccess && !(*lpbStatus) && dwElapsed < 10000 )
	{
		if ( i == 0 )
		{
			j = ::LoadString(g_hInst, IDS_CONNECTING, szTemp, sizeof(szTemp)/sizeof(TCHAR));
		}
		bResult = ::SetWindowText(::GetDlgItem(hParentWnd, uID), szTemp);
//		bResult = ::SetDlgItemText(hParentWnd, uID, szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%s%c"), szTemp, '.');
		i = ( (i < 10) ? ++i : 0 );
		::Sleep(100);
		dwElapsed += 100;
	}
	hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T(""));
	bResult = ::SetWindowText(::GetDlgItem(hParentWnd, uID), szTemp);

	return 0;
}
