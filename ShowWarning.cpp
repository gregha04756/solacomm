#include "stdafx.h"

DWORD ShowWarning(void)
{
	BOOL bSuccess = true;
	BOOL bResult = true;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
	HANDLE hReplys[2];
	SECURITY_ATTRIBUTES sa[2];
	PVOID pv;
	DWORD dwWaitResult;
	DWORD dwWaitReply;

	sa[0].bInheritHandle = true;
	sa[0].lpSecurityDescriptor = NULL;
	sa[0].nLength = sizeof(SECURITY_ATTRIBUTES);
	sa[1].bInheritHandle = true;
	sa[1].lpSecurityDescriptor = NULL;
	sa[1].nLength = sizeof(SECURITY_ATTRIBUTES);

	hReplys[0] = ::CreateEvent(&sa[0],true,false,_T("AcceptEvent"));
	hReplys[1] = ::CreateEvent(&sa[1],true,false,_T("RejectEvent"));

	pv = ::SecureZeroMemory((PVOID)&si,sizeof(si));
    si.cb = sizeof(si);
	pv = ::SecureZeroMemory((PVOID)&pi,sizeof(pi) );

    // Start the child process. 
	bSuccess = ::CreateProcess(_T("NTIWarning.exe"),   // No module name (use command line)
        NULL,        // Command line
        NULL,           // Process handle not inheritable
        false,           // Thread handle not inheritable
        true,          // Set handle inheritance to FALSE
        0,              // No creation flags
        NULL,           // Use parent's environment block
        NULL,           // Use parent's starting directory 
        &si,            // Pointer to STARTUPINFO structure
        &pi);           // Pointer to PROCESS_INFORMATION structure

	if ( bSuccess )
	{
		dwWaitReply = ::WaitForMultipleObjects(sizeof(hReplys)/sizeof(HANDLE),
		hReplys,
		false,
		INFINITE);
		if ( WAIT_FAILED == dwWaitReply )
		{
			bSuccess = false;
		}
	}

    // Wait until child process exits.
	if ( bSuccess )
	{
		dwWaitResult = ::WaitForSingleObject(pi.hProcess,INFINITE);
	}
	if ( WAIT_FAILED == dwWaitResult )
	{
		bSuccess = false;
	}

    // Close process and thread handles.
	bSuccess &= ::CloseHandle(hReplys[0]);
	bSuccess &= ::CloseHandle(hReplys[1]);
	bSuccess &= ::CloseHandle(pi.hProcess);
	bSuccess &= ::CloseHandle(pi.hThread);
	return MAKELONG((WORD)bSuccess&0xffffffff,(WORD)(dwWaitReply&0xffffffff));
}
