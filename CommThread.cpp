#include "stdafx.h"
#include "solacomm.h"
//#include "resource.h"

extern HINSTANCE g_hInst;
extern CRITICAL_SECTION gCOMCritSect;
extern CRITICAL_SECTION gRWDataCritSect;
extern unsigned short calc_CRC16( unsigned char* puchMsg, unsigned short usDataLen );
extern int check_CRC16(unsigned char *buf, int buflen);


DWORD WINAPI CommThread( LPVOID lpParam )
{
	MSG mess;
	HRESULT hRes;
	BOOL bResult;
	BOOL bSuccess = true;
	int i;
	int j;
	DWORD dwRegListSize;
	DWORD dwWaitRes;
	DWORD dwBytesRead;
	DWORD dwBytesWritten;
	HANDLE hEvents[2];
	HANDLE hCOM;
	HWND hParentWnd;
	HANDLE hQuitEvent;
	HANDLE hReadEvent;
	LPSOLAMBMAP lpSolaRegList;
	LPREADPARMS pReadParms;
	LPREADHRPARMS pReadHRParms;
	LPCRITICAL_SECTION lpCOMCritSect;
	LPCRITICAL_SECTION lpReadDataCritSect;
	unsigned char SolaMBAddress;
	unsigned short usCRC16;
	unsigned char MBMsg[512];
	union {	unsigned char SOLAResponse[512]; HOLDINGREG hr; SOLAIDRESPONSE SolaID; };

	LPRWTHREADPARMS pThreadParms = (LPRWTHREADPARMS) lpParam;
	hCOM = pThreadParms->hCOM;
	hParentWnd = pThreadParms->hParentDlg;
	hEvents[0] = hQuitEvent = pThreadParms->hQuitEvent;
	hEvents[1] = hReadEvent = pThreadParms->hReadEvent;
	lpCOMCritSect = pThreadParms->pCommCritSect;
	lpReadDataCritSect= pThreadParms->pDataCritSect;
	SolaMBAddress = pThreadParms->uchSolaAddr;
	delete pThreadParms;

	while ( bSuccess && ((bResult = ::GetMessage(&mess, NULL, 0, 0)) != 0) )
	{
		bSuccess = !( bResult == -1 );
		if ( bSuccess && mess.message == WM_APPREADHRLIST )
		{
			pReadParms = (LPREADPARMS) mess.lParam;
			::EnterCriticalSection( lpCOMCritSect );
			::EnterCriticalSection( lpReadDataCritSect );
			lpSolaRegList = pReadParms->lpSolaRegList;
			dwRegListSize = pReadParms->dwRegListSize;
			for ( j = 0; (j < dwRegListSize) && bSuccess; j++ )
			{
				if ( bSuccess )
				{
					MBMsg[0] = (lpSolaRegList+j)->uchDevAddr;
					MBMsg[1] = (lpSolaRegList+j)->uchFuncCode;
					MBMsg[2] = ((lpSolaRegList+j)->usStartRegAddr >> 8) & 0x00ff;
					MBMsg[3] = (lpSolaRegList+j)->usStartRegAddr & 0x00ff;
					MBMsg[4] = ((lpSolaRegList+j)->usRegCount >> 8) & 0x00ff;
					MBMsg[5] = (lpSolaRegList+j)->usRegCount & 0x00ff;
					usCRC16 = calc_CRC16( MBMsg, 6 );
					MBMsg[6] = usCRC16 & 0x00ff;
					MBMsg[7] = (usCRC16 >> 8) & 0x00ff;
					bSuccess = ::WriteFile( hCOM, MBMsg, 8, &dwBytesWritten, NULL );
				}
				if ( bSuccess )
				{
					bSuccess = ::ReadFile( hCOM, (LPVOID) &SOLAResponse[0], sizeof(SOLAResponse)/sizeof(unsigned char), &dwBytesRead, NULL );
				}
//			assert( (dwBytesRead > 0) && (dwBytesRead <= sizeof(SOLAResponse)/sizeof(unsigned char)) );
				if ( bSuccess && dwBytesRead && check_CRC16( SOLAResponse, dwBytesRead ) )
				{
					(lpSolaRegList+j)->sValue = (hr.uchValueHi*256) + hr.uchValueLo;
				}
			}
			::LeaveCriticalSection( lpReadDataCritSect );
			::LeaveCriticalSection( lpCOMCritSect );
			::PostMessage( pReadParms->hParentWnd, pReadParms->uMsg, 0, 0 );
		}
		if ( bSuccess && mess.message == WM_APPREADHRSNGL )
		{
			pReadHRParms = (LPREADHRPARMS) mess.lParam;
			::EnterCriticalSection( lpCOMCritSect );
			::EnterCriticalSection( lpReadDataCritSect );
			lpSolaRegList = pReadHRParms->lpSolaReg;
			MBMsg[0] = lpSolaRegList->uchDevAddr;
			MBMsg[1] = lpSolaRegList->uchFuncCode;
			MBMsg[2] = (lpSolaRegList->usStartRegAddr >> 8) & 0x00ff;
			MBMsg[3] = lpSolaRegList->usStartRegAddr & 0x00ff;
			MBMsg[4] = (lpSolaRegList->usRegCount >> 8) & 0x00ff;
			MBMsg[5] = lpSolaRegList->usRegCount & 0x00ff;
			usCRC16 = calc_CRC16( MBMsg, 6 );
			MBMsg[6] = usCRC16 & 0x00ff;
			MBMsg[7] = (usCRC16 >> 8) & 0x00ff;
			bSuccess = ::WriteFile( hCOM, MBMsg, 8, &dwBytesWritten, NULL );
			if ( bSuccess )
			{
				bSuccess = ::ReadFile( hCOM, (LPVOID) pReadHRParms->lpuchSolaResp, pReadHRParms->nRespBuffSize, &dwBytesRead, NULL );
			}
			::LeaveCriticalSection( lpReadDataCritSect );
			::LeaveCriticalSection( lpCOMCritSect );
			::PostMessage( pReadHRParms->hParentWnd, pReadHRParms->uMsg, 0, 0 );
		}
	}
	bResult = ::CloseHandle( hReadEvent );
	bResult = ::CloseHandle( hQuitEvent );
	bResult = ::CloseHandle( hCOM );

	return 0;
}
