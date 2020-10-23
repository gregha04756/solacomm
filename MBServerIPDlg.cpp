
#include "stdafx.h"
#include "solacomm.h"

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];


DWORD get_binary_IP_from_FQDN(TCHAR *fqdn_hostname,unsigned long *result_IP)
{
	const char service_name[] = "0";
	unsigned long ul_host_IP;
	int iResult;
	INT iRetval;
	char char_FQDN_hostname[MAX_PATH];
	PVOID p_v;
	HRESULT hres_r;
	DWORD dwRetval = ERROR_SUCCESS;
	int i_cchWideChar;
	int i = 1;

	ADDRINFOA *result = NULL;
	ADDRINFOA *ptr = NULL;
	ADDRINFOA hints;

	//    struct sockaddr_in6 *sockaddr_ipv6;
	LPSOCKADDR sockaddr_ip;
	struct sockaddr_in *sa_in;

	wchar_t ipstringbuffer[46];
	DWORD ipbufferlength = 46;

	hres_r = StringCchLength(fqdn_hostname, (size_t)MAX_PATH, (size_t*)&i_cchWideChar);
	p_v = SecureZeroMemory((PVOID)char_FQDN_hostname, sizeof(char_FQDN_hostname));
	iResult = WideCharToMultiByte(
		(UINT)CP_ACP,
		(DWORD)0,
		(LPCWCH)fqdn_hostname,
		(int)i_cchWideChar,
		(LPSTR)char_FQDN_hostname,
		(int)sizeof(char_FQDN_hostname)/sizeof(char),
		(LPCCH)NULL,
		(LPBOOL)NULL
	);
	//--------------------------------
	// Setup the hints address info structure
	// which is passed to the getaddrinfo() function
	p_v = SecureZeroMemory((PVOID)&hints, (size_t)sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;

	//--------------------------------
	// Call GetAddrinfoW(). If the call succeeds,
	// the result variable will hold a linked list
	// of addrinfow structures containing response
	// information
	dwRetval = getaddrinfo((PCSTR)char_FQDN_hostname, (PCSTR)service_name, &hints, &result);
	if (dwRetval != 0)
	{
		return dwRetval;
	}

	// Retrieve each address and print out the hex bytes
	for (ptr = result; ptr != NULL; ptr = ptr->ai_next) {

		switch (ptr->ai_family)
		{
		case AF_UNSPEC:
			break;
		case AF_INET:
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			sa_in = (struct sockaddr_in*)ptr->ai_addr;
			ul_host_IP = (unsigned long)sa_in->sin_addr.S_un.S_addr;
			*result_IP = (unsigned long)htonl(ul_host_IP);

			freeaddrinfo((PADDRINFOA)result);
			return dwRetval;
#if 0
			SecureZeroMemory((PVOID)s, (size_t)INET_ADDRSTRLEN);
			inet_ntop(AF_INET, &(sa_in->sin_addr), s, INET_ADDRSTRLEN);
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
				ipstringbuffer, &ipbufferlength);
			if (iRetval)
			{
				wprintf(L"WSAAddressToString failed with %u\n", WSAGetLastError());
			}
			else
			{
				wprintf(L"\tIPv4 address %ws\n", ipstringbuffer);
			}
#endif
			break;
		case AF_INET6:
			// the InetNtop function is available on Windows Vista and later
			// sockaddr_ipv6 = (struct sockaddr_in6 *) ptr->ai_addr;
			// printf("\tIPv6 address %s\n",
			//    InetNtop(AF_INET6, &sockaddr_ipv6->sin6_addr, ipstringbuffer, 46) );

			// We use WSAAddressToString since it is supported on Windows XP and later
			sockaddr_ip = (LPSOCKADDR)ptr->ai_addr;
			// The buffer length is changed by each call to WSAAddresstoString
			// So we need to set it for each iteration through the loop for safety
			ipbufferlength = 46;
			iRetval = WSAAddressToString(sockaddr_ip, (DWORD)ptr->ai_addrlen, NULL,
				ipstringbuffer, &ipbufferlength);
			break;
		default:
			break;
		}
	}

	freeaddrinfo((PADDRINFOA)result);


	return dwRetval;
}

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
			30, 50, 300, 25, hDlg, NULL, g_hInst, NULL);;
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
				dw_err = get_binary_IP_from_FQDN(sz_Host_Name, &ulIPAddress);
/*				lRes = ::SendMessage(hwndIPAddress, IPM_SETADDRESS, (WPARAM)0, (LPARAM)&ulIPAddress); */
				lpParms->ulIPAddress = ulIPAddress;
				bResult = EndDialog(hDlg, LOWORD(wParam));
				return (INT_PTR)TRUE;
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
