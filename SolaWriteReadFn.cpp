#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "SolaMultiValue.h"

using namespace std;

HANDLE g_h15Timer;
HANDLE g_h35Timer;
HANDLE g_hPollTimer;
LARGE_INTEGER g_liPollTime;

HANDLE g_hWEvents[NUMRWEVENTS];
HANDLE g_hREvents[NUMRWEVENTS];
HANDLE g_hPollEvents[NUMPOLLEVENTS];

extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" HANDLE g_h1SecTimer;
extern "C++" unsigned char* g_pMBRequest;
extern "C++" unsigned char* g_pMBResponse;
extern "C++" OVERLAPPED g_oWOverlap;
extern "C++" OVERLAPPED g_oROverlap;
extern "C++" HANDLE g_hWriteCompletedEvent;
extern "C++" HANDLE g_hReadCompletedEvent;
extern "C++" DWORD g_dwTotalSent;
extern "C++" DWORD g_dwTotalRcvd;
extern "C++" DWORD g_dwTotalCRCErrors;

extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg,unsigned short usDataLen);
extern "C++" int check_CRC16(unsigned char *buf,int buflen);

RTUCommResult MBWrite(HANDLE& hCOM,unsigned char* mbmsg,int cbmsg);
RTUCommResult MBRead(HANDLE& hCOM,unsigned char* reply,int cbreply,LPDWORD lpreplylen);

RTUCommResult MBWriteRead(HWND& hwnd,HANDLE& hCOM,unsigned char* mbmsg,int cbmsg,unsigned char* reply,int cbreply,LPDWORD lpreplylen)
{
	RTUCommResult rcr;
	unsigned short usCRC16;
	int i;

	if ( g_pMBRequest )
	{
		PVOID lpvTemp = ::SecureZeroMemory((PVOID)g_pMBRequest,512);
	}
	for ( i = 0; (i < cbmsg) && (i < 512); i++ )
	{
		*(g_pMBRequest+i) = mbmsg[i];
	}
	usCRC16 = calc_CRC16(g_pMBRequest,(unsigned short)i);
	g_pMBRequest[i++] = usCRC16 & 0x00ff;
	g_pMBRequest[i++] = (usCRC16 >> 8) & 0x00ff;
	if ( (rcr = MBWrite(hCOM,g_pMBRequest,i)) != NoError )
	{
		return rcr;
	}
	rcr =  MBRead(hCOM,reply,cbreply,lpreplylen);
	return rcr;
}


RTUCommResult MBWrite(HANDLE& hCOM,unsigned char* mbmsg,int cbmsg)
{
	RTUCommResult rcr = HardError;
	COMSTAT COMStatus;
	BOOL bResult;
	BOOL bSuccess;
	BOOL bIOPending = false;
	DWORD dwResult;
	DWORD dwBytesWritten;
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000LL;

	if ( 0LL != g_liPollTime.QuadPart )
	{
		::EnterCriticalSection(&gCOMCritSect);
		bSuccess = ::SetWaitableTimer(g_hPollTimer,&g_liPollTime,0,NULL,NULL,0);
		::LeaveCriticalSection(&gCOMCritSect);
		if ( !bSuccess )
		{
//			::DebugBreak();
			return (rcr = HardError);
		}
		dwResult = ::WaitForMultipleObjects(sizeof(g_hPollEvents)/sizeof(HANDLE),g_hPollEvents,false,INFINITE);
		if ( WAIT_FAILED == dwResult)
		{
//			::DebugBreak();
			return (rcr = HardError);
		}
		if ( 0 == (dwResult-WAIT_OBJECT_0) )
		{
//			::DebugBreak();
			return (rcr = NoError);
		}
	}

	bIOPending = false;
	g_oWOverlap.Internal = 0;
	g_oWOverlap.InternalHigh = 0;
	g_oWOverlap.Offset = 0;
	g_oWOverlap.OffsetHigh = 0;
	bSuccess = ::SetWaitableTimer(g_h1SecTimer,&liDueTime,0,NULL,NULL,0);
	bResult = ::WriteFile(hCOM,mbmsg,cbmsg,NULL,&g_oWOverlap);
	switch (bResult)
	{
	case 0:
		dwResult = ::GetLastError();
		switch (dwResult)
		{
		case ERROR_IO_PENDING:
			bIOPending = true;
			break;
		default:
			bResult = ::ClearCommError(hCOM,&dwResult,&COMStatus);
			break;
		}
		break;
	default:
		break;
	}
	if ( bIOPending )
	{
		dwResult = ::WaitForMultipleObjects(sizeof(g_hWEvents)/sizeof(HANDLE),g_hWEvents,false,INFINITE);
	}
	if ( WAIT_FAILED == dwResult )
	{
		rcr = HardError;
	}
	if  ( 1 == (dwResult - WAIT_OBJECT_0) )
	{
		bResult = ::ResetEvent(g_hWriteCompletedEvent);
		bResult = ::GetOverlappedResult(hCOM,&g_oWOverlap,&dwBytesWritten,false);
		rcr = NoError;
		::EnterCriticalSection(&gCOMCritSect);
		g_dwTotalSent += dwBytesWritten;
		::LeaveCriticalSection(&gCOMCritSect);
	}
	if ( HardError == rcr )
	{
//		::DebugBreak();
	}
	return rcr;
}

RTUCommResult MBRead(HANDLE& hCOM,unsigned char* reply,int cbreply,LPDWORD lpreplylen)
{
	RTUCommResult rcr = HardError;
	COMSTAT COMStatus;
	BOOL bSuccess = true;
	BOOL bResult = false;
	BOOL bReadResult = false;
	BOOL bClearCommResult = false;
	BOOL bIOPending = false;
	DWORD dwResult = 0L;
	DWORD dwReadErrResult = ::GetLastError();
	DWORD dwClearCommResult = 0L;
	DWORD dwClearCommErrResult = 0L;
	DWORD dwWaitResult = 0L;
	int i;
	DWORD dwBytesRead = 0L;
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000LL;

	*lpreplylen = (DWORD)0;
	bSuccess = ::SetWaitableTimer(g_h1SecTimer,&liDueTime,0,NULL,NULL,0);
	if  ( bSuccess )
	{
		g_oROverlap.Internal = 0;
		g_oROverlap.InternalHigh = 0;
		g_oROverlap.Offset = 0;
		g_oROverlap.OffsetHigh = 0;
		bReadResult = ::ReadFile(hCOM,g_pMBResponse,512,NULL,&g_oROverlap);
		dwReadErrResult = ::GetLastError();
		switch (bReadResult)
		{
		case 0:
			switch (dwReadErrResult)
			{
			case ERROR_IO_PENDING:
				bIOPending = true;
				break;
			default:
				bSuccess = bClearCommResult = ::ClearCommError(hCOM,&dwClearCommResult,&COMStatus);
				dwClearCommErrResult = ::GetLastError();
				break;
			}
			break;
		default:
			break;
		}
	}
	if ( bSuccess && bIOPending )
	{
		dwWaitResult = ::WaitForMultipleObjects(sizeof(g_hREvents)/sizeof(HANDLE),g_hREvents,false,INFINITE);
		if ( WAIT_FAILED == dwWaitResult )
		{
			rcr = HardError;
		}
		if ( 1 == (dwWaitResult - WAIT_OBJECT_0) )
		{
			bResult = ::ResetEvent(g_hReadCompletedEvent);
			bResult = ::GetOverlappedResult(hCOM,&g_oROverlap,&dwBytesRead,false);
			if ( bResult )
			{
				if ( dwBytesRead )
				{
					*lpreplylen = dwBytesRead;
					rcr = (check_CRC16(g_pMBResponse,(int)dwBytesRead) ? NoError : CRCError);
					for ( i = 0; (rcr == NoError) && (i < dwBytesRead) && (i < cbreply); i++ )
					{
						*(reply+i) = *(g_pMBResponse+i);
					}
					::EnterCriticalSection(&gCOMCritSect);
					g_dwTotalRcvd += (dwBytesRead * (rcr == NoError));
					g_dwTotalCRCErrors += (rcr == CRCError);
					::LeaveCriticalSection(&gCOMCritSect);
				}
				else
				{
					rcr = Timeout;
				}
			}
		}
		if ( 2 == (dwWaitResult - WAIT_OBJECT_0) )
		{
			rcr = Timeout;
		}
	}
	if ( bSuccess && !bIOPending )
	{
		bResult = ::ResetEvent(g_hReadCompletedEvent);
		bResult = ::GetOverlappedResult(hCOM,&g_oROverlap,&dwBytesRead,false);
		if ( bResult )
		{
			if ( dwBytesRead )
			{
				*lpreplylen = dwBytesRead;
				rcr = (check_CRC16(g_pMBResponse,(int)dwBytesRead) ? NoError : CRCError);
				for ( i = 0; (rcr == NoError) && (i < dwBytesRead) && (i < cbreply); i++ )
				{
					*(reply+i) = *(g_pMBResponse+i);
				}
				::EnterCriticalSection(&gCOMCritSect);
				g_dwTotalRcvd += (dwBytesRead * (rcr == NoError));
				g_dwTotalCRCErrors += (rcr == CRCError);
				::LeaveCriticalSection(&gCOMCritSect);
			}
			else
			{
				rcr = Timeout;
			}
		}
	}
	if ( HardError == rcr )
	{
//		::DebugBreak();
	}
	return rcr;
}

#if 0
RTUCommResult MBRead(HANDLE& hCOM,unsigned char* reply,int cbreply,LPDWORD lpreplylen)
{
	RTUCommResult rcr = HardError;
	COMSTAT COMStatus;
	BOOL bSuccess = true;
	BOOL bResult = false;
	BOOL bReadResult = false;
	BOOL bClearCommResult = false;
	BOOL bIOPending = false;
	DWORD dwResult = 0L;
	DWORD dwReadErrResult = ::GetLastError();
	DWORD dwClearCommResult = 0L;
	DWORD dwWaitResult = 0L;
	int i;
	DWORD dwBytesRead = 0L;
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000LL;

	*lpreplylen = (DWORD)0;
	bIOPending = false;
	bSuccess = ::SetWaitableTimer(g_h1SecTimer,&liDueTime,0,NULL,NULL,0);
	if  ( bSuccess )
	{
		g_oROverlap.Internal = 0;
		g_oROverlap.InternalHigh = 0;
		g_oROverlap.Offset = 0;
		g_oROverlap.OffsetHigh = 0;
		bReadResult = ::ReadFile(hCOM,g_pMBResponse,512,NULL,&g_oROverlap);
		switch (bReadResult)
		{
		case 0:
			dwReadErrResult = ::GetLastError();
			switch (dwReadErrResult)
			{
			case ERROR_IO_PENDING:
				bIOPending = true;
				break;
			default:
				bClearCommResult = ::ClearCommError(hCOM,&dwClearCommResult,&COMStatus);
//				bSuccess = false;
				break;
			}
			break;
		default:
			break;
		}
	}
	if ( bSuccess && bIOPending )
	{
		dwWaitResult = ::WaitForMultipleObjects(sizeof(g_hREvents)/sizeof(HANDLE),g_hREvents,false,INFINITE);
		if ( WAIT_FAILED == dwWaitResult )
		{
			rcr = HardError;
		}
		if ( 1 == (dwWaitResult - WAIT_OBJECT_0) )
		{
			bResult = ::ResetEvent(g_hReadCompletedEvent);
			bResult = ::GetOverlappedResult(hCOM,&g_oROverlap,&dwBytesRead,false);
			if ( bResult )
			{
				if ( dwBytesRead )
				{
					*lpreplylen = dwBytesRead;
					rcr = (check_CRC16(g_pMBResponse,(int)dwBytesRead) ? NoError : CRCError);
					for ( i = 0; (rcr == NoError) && (i < dwBytesRead) && (i < cbreply); i++ )
					{
						*(reply+i) = *(g_pMBResponse+i);
					}
					::EnterCriticalSection(&gCOMCritSect);
					g_dwTotalRcvd += (dwBytesRead * (rcr == NoError));
					g_dwTotalCRCErrors += (rcr == CRCError);
					::LeaveCriticalSection(&gCOMCritSect);
				}
				else
				{
					rcr = Timeout;
				}
			}
		}
		if ( 2 == (dwWaitResult - WAIT_OBJECT_0) )
		{
			rcr = Timeout;
		}
	}
	if ( HardError == rcr )
	{
		::DebugBreak();
	}
	return rcr;
}
#endif