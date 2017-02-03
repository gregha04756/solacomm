#include "stdafx.h"
#include "solacomm.h"

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];

INT_PTR CALLBACK MBServerIPDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	LRESULT lRes;
	static LPMBSERVERIPPARMS lpParms;
	static HWND hwndIPAddress;
	static unsigned long ulIPAddress;

	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (LPMBSERVERIPPARMS)lParam;
		ulIPAddress = lpParms->ulIPAddress;
		hwndIPAddress = ::CreateWindow(WC_IPADDRESS,szTitle,WS_CHILD|WS_VISIBLE,
			30,30,175,30,hDlg,NULL,g_hInst,NULL);
		if ( hwndIPAddress )
		{
			lRes = ::SendMessage(hwndIPAddress,IPM_SETADDRESS,(WPARAM)0,(LPARAM)ulIPAddress);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			lRes = ::SendMessage(hwndIPAddress,IPM_GETADDRESS,(WPARAM)0,(LPARAM)&ulIPAddress);
			lpParms->ulIPAddress = ulIPAddress;
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDCANCEL )
		{
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}
