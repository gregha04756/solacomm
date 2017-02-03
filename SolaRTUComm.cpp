// SolaComm.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "SolaComm.h"
#include "NoticeDialog.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "SolaChangeCode.h"
#include "SolaAlert.h"
#include "SolaLockout.h"
#include "SolaMultiValue.h"

using namespace std;

// Global Variables:
extern "C++" HINSTANCE g_hInst;								// current instance
extern "C++" TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
extern "C++" TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" CRITICAL_SECTION gTimeCritSect;
extern "C++" CRITICAL_SECTION gSaveFileCritSect;
extern "C++" CRITICAL_SECTION g_UpdCountCS;
extern "C++" BOOL bSolaConnected;
extern "C++" BOOL bCommThreadActive;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" DWORD dwCommThreadID;
extern "C++" DWORD g_dwConnectTime;
extern "C++" double g_dErrorRate;
extern "C++" DWORD g_dwTotalCRCErrors;
extern "C++" DWORD g_dwTotalSent;
extern "C++" DWORD g_dwTotalRcvd;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE hSaveFile;
extern "C++" HANDLE g_hTimerQuitEvent;
extern "C++" HANDLE g_hReadEvent;
extern "C++" HANDLE g_hStatusChangeEvent;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" HANDLE g_hSummaryQuitEvent;
extern "C++" HANDLE g_hStartRTUPollEvent;
extern "C++" HANDLE g_hReSyncReqEvent;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveTrendPages;
extern "C++" int g_nActiveStatusPages;
extern "C++" int g_nActiveConfigPages;
extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcXCHConfig;
extern "C++" CSolaMBMap* pcModConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcODResetConfig;
extern "C++" CSolaMBMap* pcDHWConfiguration;
extern "C++" CSolaMBMap* pcLLStatus;
extern "C++" CSolaMBMap* pcXLLStatus;
extern "C++" CSolaMBMap* pcLLConfig;
extern "C++" CSolaMBMap* pcXSystemConfig;
extern "C++" CSolaMBMap* pcBurnerControlStatus;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemStatus;
extern "C++" CSolaMBMap* pcSensorStatus;
extern "C++" CSolaMBMap* pcDemandModulationStatus;
extern "C++" CSolaMBMap* pcCHStatus;
extern "C++" CSolaMBMap* pcDHWStatus;
extern "C++" CSolaMBMap* pcPumpStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcAlarmCode;
extern "C++" CSolaMBMap* pcStatistics;
extern "C++" CSolaMBMaps* pcTrendMaps;
extern "C++" CSolaMBMaps* pcAllSolaMaps;
extern "C++" CSolaMBMaps* pcSystemIDMaps;
extern "C++" CSolaMBMap* pcAnnuncConfigGen;
extern "C++" CSolaMBMap* pcAnnuncConfig1;
extern "C++" CSolaMBMap* pcAnnuncConfig2;
extern "C++" CSolaMBMap* pcAnnuncConfig3;
extern "C++" CSolaMBMap* pcAnnuncConfig4;
extern "C++" CSolaMBMap* pcAnnuncConfig5;
extern "C++" CSolaMBMap* pcAnnuncConfig6;
extern "C++" CSolaMBMap* pcAnnuncConfig7;
extern "C++" CSolaMBMap* pcAnnuncConfig8;
extern "C++" CSolaMBMap* pcPIIAnnuncConfig;
extern "C++" CSolaMBMap* pcLCIAnnuncConfig;
extern "C++" CSolaMBMap* pcILKAnnuncConfig;
extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerControlStatusValues;
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" CSolaAlert* pcAlertLog;
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" int nUpdCount;
extern "C++" MBConnType mbctMBConn;
extern "C++" CSolaChangeCode* pcConfigChangeCodes;
extern "C++" CSolaChangeCode* pcStatusChangeCodes;
extern "C++" SOLAIDRESPONSE g_SolaID;

DWORD dwTicksLastData;
BOOL bCRCError;

// Forward declarations of functions included in this code module:
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg,unsigned short usDataLen);
extern "C++" int check_CRC16(unsigned char *buf,int buflen);
RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMap* lpMap);
RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMaps* lpMaps);
RTUCommResult ReadSolaAlerts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaAlert* lpLog);
RTUCommResult ReadSolaLockouts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaLockout* lpLog);
//RTUCommResult MBWriteRead(HWND& hwnd,HANDLE& hCOM,unsigned char* mbmsg,int cbmsg,unsigned char* reply,int cbreply,LPDWORD replylen);
RTUCommResult GetSolaID(HWND& hwnd,HANDLE& hCOM,unsigned char uc_mb_addr,LPSOLAIDRESPONSE lpSolaID);
RTUCommResult IsBusQuiet(HANDLE& hCOM);

extern "C++" RTUCommResult MBWriteRead(HWND& hwnd,HANDLE& hCOM,unsigned char* mbmsg,int cbmsg,unsigned char* reply,int cbreply,LPDWORD lpreplylen);
extern "C++" RTUCommResult MBRead(HANDLE& hCOM,unsigned char* reply,int cbreply,LPDWORD lpreplylen);
extern "C++" RTUCommResult MBWrite(HANDLE& hCOM,unsigned char* mbmsg,int cbmsg);


DWORD WINAPI CommThread(LPVOID lpParam)
{
	HANDLE hEvents[NUMPROPPAGES+1];
	HANDLE hStartEvents[2];
	BOOL bResult;
	BOOL bSuccess = true;
	BOOL bStatusChange = true;
	BOOL bConfigChange = true;
	int i;
	int j;
	int nResult;
	int nEventCount;
	DWORD dwBytesRead;
	DWORD dwResult;
	HANDLE hCOM;
	HWND hParentWnd;
	HANDLE hQuitEvent;
	HANDLE hReadEvent;
	LPCRITICAL_SECTION lpCOMCritSect;
	LPCRITICAL_SECTION lpReadDataCritSect;
	unsigned char uc_MB_Address;
	unsigned short usBit;
	unsigned char MBMsg[512];
	union {	unsigned char SOLAResponse[512]; SOLAHRRESPONSE hr; SOLAIDRESPONSE SolaID; };
	PVOID lpvTemp;
	RTUCommResult rcr;
	LPSOLAIDRESPONSE lpSolaID = (LPSOLAIDRESPONSE)new SOLAIDRESPONSE;

	bCRCError = false;

	hCOM = ((LPRWTHREADPARMS) lpParam)->hCOM;
	hParentWnd = ((LPRWTHREADPARMS) lpParam)->hParentDlg;
	hQuitEvent = ((LPRWTHREADPARMS) lpParam)->hQuitEvent;
	hReadEvent = ((LPRWTHREADPARMS) lpParam)->hReadEvent;
	lpCOMCritSect = ((LPRWTHREADPARMS) lpParam)->pCommCritSect;
	lpReadDataCritSect= ((LPRWTHREADPARMS) lpParam)->pDataCritSect;
	uc_MB_Address = ((LPRWTHREADPARMS) lpParam)->uchSolaAddr;
	delete ((LPRWTHREADPARMS) lpParam);

	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,GetCurrentProcess(),&hEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	for ( j = 1; !g_bQuit && j < NUMPROPPAGES+1; j++ )
	{
		bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hPageUpdEvents[j-1],GetCurrentProcess(),&hEvents[j],0,false,DUPLICATE_SAME_ACCESS);
	}

	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,GetCurrentProcess(),&hStartEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hStartRTUPollEvent,GetCurrentProcess(),&hStartEvents[1],0,false,DUPLICATE_SAME_ACCESS);
	dwResult = ::WaitForMultipleObjects(2,hStartEvents,false,INFINITE);
	if ( dwResult == WAIT_FAILED || dwResult == WAIT_TIMEOUT )
	{
		nResult = ::MessageBox(hParentWnd,_T("RTU poll thread failed, aborting"),szTitle,MB_OK);
		bResult = ::CloseHandle(hStartEvents[0]);
		bResult = ::CloseHandle(hStartEvents[1]);
		return 0;
	}
	if ( dwResult - WAIT_OBJECT_0 < sizeof(hStartEvents)/sizeof(HANDLE) )
	{
		if ( g_bQuit || (0 == (dwResult - WAIT_OBJECT_0)) )
		{
			bResult = ::CloseHandle(hStartEvents[0]);
			bResult = ::CloseHandle(hStartEvents[1]);
			return 0;
		}
	}

/* Begin main while() loop */
	while ( bSuccess && !g_bQuit && ((dwResult = ::WaitForSingleObject(hEvents[0],0)) == WAIT_TIMEOUT) )
	{
		uc_MB_Address = SOLAMBAddress;
		lpvTemp = ::SecureZeroMemory((PVOID)&g_SolaID,sizeof(g_SolaID));
		bStatusChange = true;
		bConfigChange = true;
		if ( !bSolaConnected )
		{
			bResult = ::PostMessage(hParentWnd,WM_APPSOLACONNECTING,(WPARAM)0,(LPARAM)&g_SolaID);
		}
/*		while ( !g_bQuit && bSuccess && !bSolaConnected )*/
		if ( !g_bQuit && bSuccess && !bSolaConnected )
		{
			bSuccess = (HardError != GetSolaID(hParentWnd,hCOM,uc_MB_Address,&g_SolaID));
			bSolaConnected = (g_SolaID.SolaID != 0);
			::EnterCriticalSection(&gTimeCritSect);
			g_dwConnectTime = 0L;
			::LeaveCriticalSection(&gTimeCritSect);
			::EnterCriticalSection(&gCOMCritSect);
			g_dwTotalCRCErrors = 0L;
			g_dwTotalRcvd = g_dwTotalSent = 0L;
			::LeaveCriticalSection(&gCOMCritSect);
		}

		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bResult = ::PostMessage(hParentWnd,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)&g_SolaID);
			bResult = ::PostMessage(hParentWnd,WM_APPDATAUPDSTART,(WPARAM)0,(LPARAM)0);
		}

/* Get all registers */
		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			dwTicksLastData = ::GetTickCount();
			bSuccess = (HardError != ReadSolaMap(hParentWnd,hCOM,uc_MB_Address,pcAllSolaMaps));
		}

/* Get alert log */
		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bSuccess = (HardError != ReadSolaAlerts(hParentWnd,hCOM,uc_MB_Address,pcAlertLog));
		}

/* Get Lockout history log */
		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bSuccess = (HardError != ReadSolaLockouts(hParentWnd,hCOM,uc_MB_Address,pcLockoutLog));
		}

		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bResult = ::PostMessage(hParentWnd,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)&g_SolaID);
		}

/* Poll until commanded to refresh connection or quit */
		while ( !g_bQuit && bSuccess && bSolaConnected && ((dwResult = ::WaitForSingleObject(hEvents[0],0)) == WAIT_TIMEOUT) )
		{
			::EnterCriticalSection(&gRWDataCritSect);
			nEventCount = g_nActiveTrendPages + (g_nActiveStatusPages*bStatusChange) + (g_nActiveConfigPages*bConfigChange);
			for ( i = 0; !g_bQuit && i < NUMPROPPAGES; i++ )
			{
				if ( g_lpPageDataEvents[i].typePage == TrendPage )
				{
					bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
				}
				if ( bStatusChange && g_lpPageDataEvents[i].typePage == StatusPage )
				{
					bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
				}
				if ( bConfigChange && g_lpPageDataEvents[i].typePage == ConfigPage )
				{
					bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
				}
			}
			::LeaveCriticalSection(&gRWDataCritSect);

/* Always get dynamic trend data */
			if ( bSuccess && !g_bQuit && bSolaConnected )
			{
				rcr = ReadSolaMap(hParentWnd,hCOM,uc_MB_Address,pcTrendMaps);
				bSuccess = (HardError != rcr);
				dwTicksLastData = ((rcr == NoError) ? ::GetTickCount() : dwTicksLastData);
			}

/* Check if there's a status update */
			bStatusChange = false;
			usBit = 1;
			for ( i = 0; !g_bQuit && bSuccess && i < pcStatusChangeCodes->GetSize(); i++ )
			{
				if ( pcStatusChangeCodes->ItemBitMask(i) & pcSystemStatus->GetValue((int)0) )
				{
					bStatusChange = true;
					if ( i == 1 )
					{
						i = i;
					}
					if ( bSuccess && pcStatusChangeCodes->SolaMBMap(i) != NULL )
					{
						bSuccess = (HardError != ReadSolaMap(hParentWnd,hCOM,uc_MB_Address,pcStatusChangeCodes->SolaMBMap(i)));
					}
					if ( bSuccess && pcStatusChangeCodes->SolaAlertLog(i) != NULL )
					{
						bSuccess = (HardError != ReadSolaAlerts(hParentWnd,hCOM,uc_MB_Address,pcAlertLog));
					}
					if ( bSuccess && pcStatusChangeCodes->SolaLockoutLog(i) != NULL )
					{
						bSuccess = (HardError != ReadSolaLockouts(hParentWnd,hCOM,uc_MB_Address,pcLockoutLog));
					}
				}
				usBit <<= 1;
			}

/* Check if there's a configuration update. */
/* System ID and Access has to be handled separately because of string */
/* values. */
			bConfigChange = false;
			usBit = 1;
			if ( bSuccess && usBit & pcSystemStatus->GetValue((int)1) )
			{
				bConfigChange = true;
				bSuccess = (HardError != ReadSolaMap(hParentWnd,hCOM,uc_MB_Address,pcSystemIDMaps));
			}

			for ( i = 0; !g_bQuit && bSuccess && i < pcConfigChangeCodes->GetSize() && pcSystemStatus->GetValue((int)1); i++ )
			{
				if ( pcConfigChangeCodes->ItemBitMask(i) & pcSystemStatus->GetValue((int)1) )
				{
#if _DEBUG
					if ( 0x0001 & pcConfigChangeCodes->ItemBitMask(i) )
					{
						i = i;
					}
					if ( 0x0002 & pcConfigChangeCodes->ItemBitMask(i) )
					{
						i = i;
					}
					if ( 0x0800 & pcConfigChangeCodes->ItemBitMask(i) )
					{
						i = i;
					}
#endif
					bConfigChange = true;
					if ( pcConfigChangeCodes->SolaMBMap(i) != NULL )
					{
						bSuccess = (HardError != ReadSolaMap(hParentWnd,hCOM,uc_MB_Address,pcConfigChangeCodes->SolaMBMap(i)));
					}
				}
				usBit <<= 1;
			}

			::EnterCriticalSection(&gRWDataCritSect);
			while ( !g_bQuit && bSuccess && !g_MBSndRcvReqQ.empty() )
			{
				i = 0;
				while ( !g_bQuit && *g_MBSndRcvReqQ.front().ppchToSnd < *g_MBSndRcvReqQ.front().ppchEndSnd && *g_MBSndRcvReqQ.front().ppchToSnd < g_MBSndRcvReqQ.front().pchSndBuf + g_MBSndRcvReqQ.front().nSndBufSize  )
				{
					MBMsg[i++] = *(*g_MBSndRcvReqQ.front().ppchToSnd)++;
				}
				bSuccess = (HardError != MBWriteRead(hParentWnd,hCOM,MBMsg,i,g_MBSndRcvReqQ.front().pchRcvBuf,g_MBSndRcvReqQ.front().nRcvBufSize,&dwBytesRead));
				*g_MBSndRcvReqQ.front().ppchEndRcv = g_MBSndRcvReqQ.front().pchRcvBuf + dwBytesRead;
				bResult = ::PostMessage(g_MBSndRcvReqQ.front().hPage,g_MBSndRcvReqQ.front().nMsg,(WPARAM)3,(LPARAM) 0);
				g_MBSndRcvReqQ.pop();
			}
			::LeaveCriticalSection(&gRWDataCritSect);
			bSolaConnected = ((::GetTickCount() - dwTicksLastData) < SOLAREADSDTO);
			dwResult = ::WaitForSingleObject(g_hReSyncReqEvent,0);
			bSuccess = ((WAIT_FAILED != dwResult) && bSuccess);
			bSolaConnected = ((WAIT_FAILED != dwResult) && (WAIT_TIMEOUT == dwResult)) ? bSolaConnected : false;
		} /* end while ( !g_bQuit && bSuccess && bSolaConnected && ((dwResult = ::WaitForSingleObject(hEvents[0],0)) == WAIT_TIMEOUT) ) */
	} /* End of the main while() loop */

	for ( j = 0; j < sizeof(hEvents)/sizeof(HANDLE); j++ )
	{
		bResult = ::CloseHandle(hEvents[j]);
	}
	if ( !bSuccess )
	{
//		::DebugBreak();
		bSolaConnected = false;
		bResult = ::PostMessage(hParentWnd,WM_APPSOLAPOLLABORT,(WPARAM)0,(LPARAM)0);
	}

	return 0;
}


RTUCommResult GetSolaID(HWND& hwnd,HANDLE& hCOM,unsigned char uc_mb_addr,LPSOLAIDRESPONSE lpSolaID)
{
	RTUCommResult rcr = HardError;
	BOOL bSuccess = true;
	BOOL bBusQuiet = true;
	unsigned char SOLAIDRequest[2];
	int i;
	union {unsigned char SOLAResponse[512]; HOLDINGREG hr; SOLAIDRESPONSE SolaID;};
	DWORD dwBytesWritten = 0;
	DWORD dwBytesRead = 0;
	PVOID lpvTemp;

	bSuccess = (HardError != (rcr = IsBusQuiet(hCOM)));
	if ( bSuccess )
	{
		bBusQuiet = (rcr == Timeout);
	}
	if ( bBusQuiet && bSuccess )
	{
		lpvTemp = ::SecureZeroMemory((PVOID)SOLAResponse,sizeof(SOLAResponse));
		SOLAIDRequest[0] = uc_mb_addr;
		SOLAIDRequest[1] = 0x11;
		rcr = MBWriteRead(hwnd,hCOM,SOLAIDRequest,sizeof(SOLAIDRequest)/sizeof(unsigned char),SOLAResponse,sizeof(SOLAResponse),&dwBytesRead);
		bSuccess = (rcr != HardError);
	}
	if ( bBusQuiet && (rcr == NoError) && (dwBytesRead > 0) && (SolaID.ByteCount == 0x26) )
	{
		lpSolaID->SolaAddr = SolaID.SolaAddr;
		lpSolaID->FunctionCode = SolaID.FunctionCode;
		lpSolaID->SolaID = SolaID.SolaID;
		lpSolaID->RunIndicator = SolaID.RunIndicator;
		for ( i = 0; i < sizeof(SolaID.BurnerName); i++ )
		{
			*((lpSolaID->BurnerName)+i) = SolaID.BurnerName[i];
		}
		for ( i = 0; i < sizeof(SolaID.OSNumber); i++ )
		{
			*((lpSolaID->OSNumber)+i) = SolaID.OSNumber[i];
		}
		dwTicksLastData = ::GetTickCount();
	}
	return rcr;
}

RTUCommResult IsBusQuiet(HANDLE& hCOM)
{
	RTUCommResult rcr = HardError;
//	COMMTIMEOUTS OldCommTimeouts;
//	COMMTIMEOUTS CommTimeouts;
	unsigned char uchBuf[512];
	PVOID lpVoid;
	DWORD dwBytesRead = 0;
	BOOL bSuccess = true;

	::EnterCriticalSection(&gCOMCritSect);
	bSuccess = ::PurgeComm(hCOM,PURGE_RXCLEAR);
	rcr = ((bSuccess) ? NoError : HardError);
	if ( bSuccess )
	{
		lpVoid = ::SecureZeroMemory((PVOID)uchBuf,(SIZE_T)sizeof(uchBuf));
		rcr = MBRead(hCOM,uchBuf,sizeof(uchBuf)/sizeof(char),&dwBytesRead);
		bSuccess = (rcr != HardError);
	}
	::LeaveCriticalSection(&gCOMCritSect);
	return rcr;
}

RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMaps* lpMaps)
{
	RTUCommResult rcr = HardError;
	int j;
	union {unsigned char SOLAResponse[512];SOLAHRRESPONSE hr;SOLAIDRESPONSE SolaID;};

	for ( j = 0; !g_bQuit && j < lpMaps->GetSize(); j++ )
	{
		rcr = ReadSolaMap(hwnd,hCOM,ucMBAddr,lpMaps->GetLPMap(j));
	}
	return (RTUCommResult)rcr;
}

RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMap* lpMap)
{
	BOOL bOkCRC;
	BOOL bSuccess;
	RTUCommResult rcr = HardError;
	CSolaMBMap::LPSOLAMBMAP lpSolaRequest = NULL;
	DWORD dwBytesRead;
	int j;
	int nOffset;
	unsigned char MBMsg[512];
	unsigned char uchRespLo;
	unsigned char uchRespHi;
	U32 u32Value;
	U32 u32Ret;
	union {unsigned char SOLAResponse[512];SOLAHRRESPONSE hr;SOLAIDRESPONSE SolaID;};
	short sRegCount;
	PVOID lpvTemp;

	if ( NULL != lpMap->GetLPMap((int)0) )
	{
		lpSolaRequest = lpMap->GetLPMap((int)0);
		sRegCount = (short)lpMap->GetRegRequestLen();
	}
	if ( NULL != lpMap->GetLPU32Map((int)0) )
	{
		sRegCount = (short)lpMap->GetU32RegTotal();
	}
	MBMsg[0] = ucMBAddr;
	MBMsg[1] = lpMap->GetFuncCode((int)0);
	MBMsg[2] = (lpMap->GetStartRegAddr((int)0) >> 8) & 0x00ff;
	MBMsg[3] = lpMap->GetStartRegAddr((int)0) & 0x00ff;
	MBMsg[4] = (sRegCount >> 8) & 0x00ff;
	MBMsg[5] = sRegCount & 0x00ff;
	bSuccess = (HardError != (rcr = MBWriteRead(hwnd,hCOM,MBMsg,6,SOLAResponse,sizeof(SOLAResponse),&dwBytesRead)));
	bOkCRC = false;
	if ( (rcr == NoError) && (dwBytesRead > 0) )
	{
		for ( nOffset = 0; nOffset < lpMap->GetRegGroupSize(); nOffset++ )
		{
			switch (lpMap->GetType((int)nOffset))
			{
			case CSolaMBMap::Novalue:
				break;
			case CSolaMBMap::DupStringValue:
			case CSolaMBMap::Stringvalue:
				if (((lpMap->GetType((int)nOffset) == CSolaMBMap::Stringvalue) ||
					(lpMap->GetType((int)nOffset) == CSolaMBMap::DupStringValue)) &&
					(lpMap->GetStr((int)nOffset) != NULL) )
				{
					lpvTemp = (PVOID)lpMap->SetStr((int)nOffset,&SOLAResponse[sizeof(hr)],hr.cbByteCount);
				}
				break;
			case CSolaMBMap::Unsigned32:
				j = nOffset;
				u32Value = SOLAResponse[sizeof(hr)+(j*4)+3] +
					(SOLAResponse[sizeof(hr)+(j*4)+2]*256L) +
					(SOLAResponse[sizeof(hr)+(j*4)+1]*65536L) +
					(SOLAResponse[sizeof(hr)+(j*4)]*16777216L);
				u32Ret = lpMap->SetU32Value((int)nOffset,u32Value);
				break;
			default:
				j = nOffset;
				uchRespLo = SOLAResponse[sizeof(hr)+(j*2)+1];
				uchRespHi = SOLAResponse[sizeof(hr)+(j*2)];
				lpMap->SetValue((int)nOffset,uchRespLo+(256*uchRespHi));
				break;
			}
		}
	}

	return (RTUCommResult)rcr;
}

RTUCommResult ReadSolaAlerts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaAlert* lpLog)
{
	BOOL bOkCRC;
	BOOL bSuccess;
	RTUCommResult rcr = HardError;
	LPSOLAALERT lpAlertRecord;
	DWORD dwBytesRead;
	int i;
	unsigned char MBMsg[512];
	union {unsigned char SOLAResponse[512];SOLAHRRESPONSE hr;SOLAIDRESPONSE SolaID;};
	for ( i = 0; !g_bQuit && i < lpLog->GetSize(); i++ )
	{
		lpAlertRecord = pcAlertLog->GetLPMap(i);
		MBMsg[0] = ucMBAddr;
		MBMsg[1] = lpAlertRecord->uchFuncCode;
		MBMsg[2] = (lpAlertRecord->usStartRegAddr >> 8) & 0x00ff;
		MBMsg[3] = lpAlertRecord->usStartRegAddr & 0x00ff;
		MBMsg[4] = (lpAlertRecord->usRegCount >> 8) & 0x00ff;
		MBMsg[5] = lpAlertRecord->usRegCount & 0x00ff;
		bSuccess = (HardError != (rcr = MBWriteRead(hwnd,hCOM,MBMsg,6,SOLAResponse,sizeof(SOLAResponse),&dwBytesRead)));
		bOkCRC = false;
		if ( (rcr == NoError) && (dwBytesRead > 0) )
		{
			lpAlertRecord->pAlertRecord->aa.usAlertCode =  (256*SOLAResponse[sizeof(hr)]) + SOLAResponse[sizeof(hr)+1];
				lpAlertRecord->pAlertRecord->aa.ulCycle = (256*256*256*SOLAResponse[sizeof(hr)+2]) + (256*256*SOLAResponse[sizeof(hr)+3]) +
				(256*SOLAResponse[sizeof(hr)+4]) + (SOLAResponse[sizeof(hr)+5]);
			lpAlertRecord->pAlertRecord->aa.ulHours = (256*256*256*(SOLAResponse[sizeof(hr)+6])) + (256*256*SOLAResponse[sizeof(hr)+7]) +
				(256*SOLAResponse[sizeof(hr)+8]) + (256*256*256*SOLAResponse[sizeof(hr)+9]);
			lpAlertRecord->pAlertRecord->aa.uchCount =  SOLAResponse[sizeof(hr)+11];
		}
	}
	return rcr;
}

RTUCommResult ReadSolaLockouts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaLockout* lpLog)
{
	RTUCommResult rcr = HardError;
	BOOL bOkCRC;
	BOOL bSuccess;
	LPSOLALOCKOUT lpLockoutRecord;
	DWORD dwBytesRead;
	int i;
	unsigned char MBMsg[512];
	union {unsigned char SOLAResponse[512];SOLAHRRESPONSE hr;SOLAIDRESPONSE SolaID;};

	for ( i = 0; !g_bQuit && i < pcLockoutLog->GetSize(); i++ )
	{
		lpLockoutRecord = lpLog->GetLPMap(i);
		MBMsg[0] = ucMBAddr;
		MBMsg[1] = lpLockoutRecord->uchFuncCode;
		MBMsg[2] = (lpLockoutRecord->usStartRegAddr >> 8) & 0x00ff;
		MBMsg[3] = lpLockoutRecord->usStartRegAddr & 0x00ff;
		MBMsg[4] = (lpLockoutRecord->usRegCount >> 8) & 0x00ff;
		MBMsg[5] = lpLockoutRecord->usRegCount & 0x00ff;
		bSuccess = (HardError != (rcr = MBWriteRead(hwnd,hCOM,MBMsg,8,SOLAResponse,sizeof(SOLAResponse),&dwBytesRead)));
		bOkCRC = false;
		if ( (rcr == NoError) && (dwBytesRead > 0) )
		{
			lpLockoutRecord->pLockoutUnion->slr.usLockoutCode = (256*SOLAResponse[sizeof(hr)]) + SOLAResponse[sizeof(hr)+1];
			lpLockoutRecord->pLockoutUnion->slr.usAnnunciatorFirstOut = (256*SOLAResponse[sizeof(hr)+2]) + SOLAResponse[sizeof(hr)+3];
			lpLockoutRecord->pLockoutUnion->slr.usBurnerControlState = (256*SOLAResponse[sizeof(hr)+4]) + SOLAResponse[sizeof(hr)+5];
			lpLockoutRecord->pLockoutUnion->slr.usSequenceTime = (256*SOLAResponse[sizeof(hr)+6]) + SOLAResponse[sizeof(hr)+7];
			lpLockoutRecord->pLockoutUnion->slr.ulCycle = (16777216*SOLAResponse[sizeof(hr)+8]) + (65536*SOLAResponse[sizeof(hr)+9]) +
				(256*SOLAResponse[sizeof(hr)+10]) + (SOLAResponse[sizeof(hr)+11]);
			lpLockoutRecord->pLockoutUnion->slr.ulHours = (16777216*SOLAResponse[sizeof(hr)+12]) + (65536*SOLAResponse[sizeof(hr)+13]) +
				(256*SOLAResponse[sizeof(hr)+14]) + (SOLAResponse[sizeof(hr)+15]);
			lpLockoutRecord->pLockoutUnion->slr.usIO = (256*SOLAResponse[sizeof(hr)+16]) + SOLAResponse[sizeof(hr)+17];
			lpLockoutRecord->pLockoutUnion->slr.usAnnunciator = (256*SOLAResponse[sizeof(hr)+18]) + SOLAResponse[sizeof(hr)+19];
			lpLockoutRecord->pLockoutUnion->slr.usOutletTemperature = (256*SOLAResponse[sizeof(hr)+20]) + SOLAResponse[sizeof(hr)+21];
			lpLockoutRecord->pLockoutUnion->slr.usInletTemperature = (256*SOLAResponse[sizeof(hr)+22]) + SOLAResponse[sizeof(hr)+23];
			lpLockoutRecord->pLockoutUnion->slr.usDHWTemperature = (256*SOLAResponse[sizeof(hr)+24]) + SOLAResponse[sizeof(hr)+25];
			lpLockoutRecord->pLockoutUnion->slr.usODTemperature = (256*SOLAResponse[sizeof(hr)+26]) + SOLAResponse[sizeof(hr)+27];
			lpLockoutRecord->pLockoutUnion->slr.usStackTemperature = (256*SOLAResponse[sizeof(hr)+28]) + SOLAResponse[sizeof(hr)+29];
			lpLockoutRecord->pLockoutUnion->slr.us4to20mAInput = (256*SOLAResponse[sizeof(hr)+30]) + SOLAResponse[sizeof(hr)+31];
			lpLockoutRecord->pLockoutUnion->slr.ucFaultData0 = SOLAResponse[sizeof(hr)+32];
			lpLockoutRecord->pLockoutUnion->slr.ucFaultData1 = SOLAResponse[sizeof(hr)+33];
		}
	}
	return rcr;
}
