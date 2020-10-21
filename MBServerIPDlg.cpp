#include "stdafx.h"
#include "solacomm.h"

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];

INT_PTR CALLBACK MBServerIPDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	DWORD dw_err;
	BOOL bResult;
	LRESULT lRes;
	UINT ui_res;
	int i_hn_length = 0;;
	PVOID p_v = NULL;
	static LPMBSERVERIPPARMS lpParms;
	static HWND hwndIPAddress;
	static HWND hwnd_Host_Name_Edit;
	static unsigned long ulIPAddress;
	static TCHAR sz_Host_Name[MAX_PATH];

	switch (message)
	{
	case WM_INITDIALOG:
		p_v = SecureZeroMemory((PVOID)sz_Host_Name, sizeof(sz_Host_Name));
		lpParms = (LPMBSERVERIPPARMS)lParam;
		ulIPAddress = lpParms->ulIPAddress;
		hwndIPAddress = ::CreateWindow(WC_IPADDRESS,szTitle,WS_CHILD|WS_VISIBLE| WS_TABSTOP,
			30,125,175,25,hDlg,NULL,g_hInst,NULL);
		if ( hwndIPAddress )
		{
			lRes = ::SendMessage(hwndIPAddress,IPM_SETADDRESS,(WPARAM)0,(LPARAM)ulIPAddress);
		}
		hwnd_Host_Name_Edit = ::CreateWindowEx(WS_EX_CLIENTEDGE, WC_EDIT, NULL, WS_CHILD | WS_VISIBLE | WS_BORDER | WS_TABSTOP,
			30, 50, 300, 20, hDlg, NULL, g_hInst, NULL);;
		if (NULL != hwnd_Host_Name_Edit)
		{
			ui_res = SetWindowText(hwnd_Host_Name_Edit, sz_Host_Name);
		}

		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			i_hn_length = 0;
			if (NULL != hwnd_Host_Name_Edit)
			{
				i_hn_length = GetWindowTextLength(hwnd_Host_Name_Edit);
			}
			if (0 != i_hn_length)
			{
				ui_res = GetWindowText(hwnd_Host_Name_Edit, sz_Host_Name, sizeof(sz_Host_Name) / sizeof(TCHAR));
				if (ERROR_SUCCESS != (dw_err = GetLastError()))
				{
					ReportError(dw_err);
					return (INT_PTR)TRUE;
				}
			}
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
