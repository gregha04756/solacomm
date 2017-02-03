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
#include "SolaPage.h"

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
extern "C++" DWORD dwCommThreadID;
extern "C++" DWORD g_dwConnectTime;
extern "C++" double g_dErrorRate;
extern "C++" DWORD g_dwTotalCRCErrors;
extern "C++" DWORD g_dwTotalSent;
extern "C++" DWORD g_dwTotalRcvd;
extern "C++" unsigned char SOLAMBAddress;
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
extern "C++" CSolaMBMaps* pcStatusMaps;
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

extern "C++" DWORD dwTicksLastData;
extern "C++" BOOL bCRCError;

// Forward declarations of functions included in this code module:
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg,unsigned short usDataLen);
extern "C++" int check_CRC16(unsigned char *buf,int buflen);
extern "C++" RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMap* lpMap);
extern "C++" RTUCommResult ReadSolaMap(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaMBMaps* lpMaps);
extern "C++" RTUCommResult ReadSolaAlerts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaAlert* lpLog);
extern "C++" RTUCommResult ReadSolaLockouts(HWND& hwnd,HANDLE& hCOM,unsigned char ucMBAddr,CSolaLockout* lpLog);
extern "C++" RTUCommResult GetSolaID(HWND& hwnd,HANDLE& hCOM,unsigned char SolaMBAddress,LPSOLAIDRESPONSE lpSolaID);
extern "C++" RTUCommResult IsBusQuiet(HANDLE& hCOM);

extern "C++" RTUCommResult MBWriteRead(HWND& hwnd,HANDLE& hCOM,unsigned char* mbmsg,int cbmsg,unsigned char* reply,int cbreply,LPDWORD lpreplylen);
extern "C++" RTUCommResult MBRead(HANDLE& hCOM,unsigned char* reply,int cbreply,LPDWORD lpreplylen);
extern "C++" RTUCommResult MBWrite(HANDLE& hCOM,unsigned char* mbmsg,int cbmsg);
extern "C++" std::list<CSolaMBMap*> *p_Reg_Group_List;

std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page)
{
	int i_i;
	if (!bSolaConnected || (TCP == mbctMBConn) || (RTU_Serial_Gateway == mbctMBConn))
	{
		EnterCriticalSection(&gRWDataCritSect);
		p_Reg_Group_List->clear();
		for (i_i = 0; i_i < p_page->GetSize(); i_i++)
		{
			p_Reg_Group_List->push_back(p_page->ItemMap(i_i));
		}
		p_Reg_Group_List->sort();
		p_Reg_Group_List->unique();
		LeaveCriticalSection(&gRWDataCritSect);
	}
	return p_Reg_Group_List;
}

DWORD WINAPI RTU_Serial_Gateway_Fn(LPVOID lpParam)
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
	unsigned char SolaMBAddress;
	unsigned short usBit;
	unsigned char MBMsg[512];
	union {	unsigned char SOLAResponse[512]; SOLAHRRESPONSE hr; SOLAIDRESPONSE SolaID; };
	PVOID lpvTemp;
	RTUCommResult rcr;
	LPSOLAIDRESPONSE lpSolaID = (LPSOLAIDRESPONSE)new SOLAIDRESPONSE;
	std::list<CSolaMBMap*>::iterator it_reg_list;
#if _DEBUG
	DWORD dw_ticks_now;
	DWORD dw_ticks_diff;
#endif

	bCRCError = false;

	hCOM = ((LPRWTHREADPARMS) lpParam)->hCOM;
	hParentWnd = ((LPRWTHREADPARMS) lpParam)->hParentDlg;
	hQuitEvent = ((LPRWTHREADPARMS) lpParam)->hQuitEvent;
	hReadEvent = ((LPRWTHREADPARMS) lpParam)->hReadEvent;
	lpCOMCritSect = ((LPRWTHREADPARMS) lpParam)->pCommCritSect;
	lpReadDataCritSect= ((LPRWTHREADPARMS) lpParam)->pDataCritSect;
	SolaMBAddress = ((LPRWTHREADPARMS) lpParam)->uchSolaAddr;
	delete ((LPRWTHREADPARMS) lpParam);

	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,GetCurrentProcess(),&hEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	for ( j = 1; !g_bQuit && j < NUMPROPPAGES+1; j++ )
	{
		bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hPageUpdEvents[j-1],GetCurrentProcess(),&hEvents[j],0,false,DUPLICATE_SAME_ACCESS);
	}

	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hReadQuitEvent,GetCurrentProcess(),&hStartEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),g_hStartRTUPollEvent,GetCurrentProcess(),&hStartEvents[1],0,false,DUPLICATE_SAME_ACCESS);
	dwResult = ::WaitForMultipleObjects(sizeof(hStartEvents)/sizeof(HANDLE),hStartEvents,false,INFINITE);
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

	while ( bSuccess && !g_bQuit && ((dwResult = ::WaitForSingleObject(hEvents[0],0)) == WAIT_TIMEOUT) )
	{
		lpvTemp = ::SecureZeroMemory((PVOID)&g_SolaID,sizeof(g_SolaID));
		bStatusChange = true;
		bConfigChange = true;
		if ( !bSolaConnected )
		{
			bResult = ::PostMessage(hParentWnd,WM_APPSOLACONNECTING,(WPARAM)0,(LPARAM)&g_SolaID);
		}
		while ( !g_bQuit && bSuccess && !bSolaConnected )
		{
			bSuccess = (HardError != GetSolaID(hParentWnd,hCOM,SolaMBAddress,&g_SolaID));
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

		dwTicksLastData = ::GetTickCount();
		bSuccess = (HardError != ReadSolaMap(hParentWnd,hCOM,SolaMBAddress,pcAllSolaMaps));

/* Get alert log */
		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bSuccess = (HardError != ReadSolaAlerts(hParentWnd,hCOM,SolaMBAddress,pcAlertLog));
		}

/* Get Lockout history log */
		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bSuccess = (HardError != ReadSolaLockouts(hParentWnd,hCOM,SolaMBAddress,pcLockoutLog));
		}

		if ( bSuccess && !g_bQuit && bSolaConnected )
		{
			bResult = ::PostMessage(hParentWnd,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)&g_SolaID);
		}
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

/* Always get dynamic trend and status data */
			if ( bSuccess && !g_bQuit && bSolaConnected )
			{
				rcr = ReadSolaMap(hParentWnd,hCOM,::SOLAMBAddress,pcStatusMaps);
				bSuccess = (HardError != rcr);
				dwTicksLastData = ((rcr == NoError) ? ::GetTickCount() : dwTicksLastData);
			}

/* Only get register groups needed for the currently displayed page */
			if ( bSuccess && !g_bQuit && bSolaConnected )
			{
				EnterCriticalSection(&gRWDataCritSect);
				if (!p_Reg_Group_List->empty())
				{
					for (it_reg_list = p_Reg_Group_List->begin();!g_bQuit && (it_reg_list != p_Reg_Group_List->end());it_reg_list++)
					{
						rcr = ReadSolaMap(hParentWnd,hCOM,::SOLAMBAddress,*it_reg_list);
						bSuccess = (HardError != rcr);
						dwTicksLastData = ((rcr == NoError) ? ::GetTickCount() : dwTicksLastData);
					}
				}
				LeaveCriticalSection(&gRWDataCritSect);
			}

/* Get alert log */
			if ( bSuccess && !g_bQuit && bSolaConnected )
			{
				bSuccess = (HardError != ReadSolaAlerts(hParentWnd,hCOM,SolaMBAddress,pcAlertLog));
			}

/* Get Lockout history log */
			if ( bSuccess && !g_bQuit && bSolaConnected )
			{
				bSuccess = (HardError != ReadSolaLockouts(hParentWnd,hCOM,SolaMBAddress,pcLockoutLog));
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
#if _DEBUG
			dw_ticks_now = GetTickCount();
			bSolaConnected = ((dw_ticks_diff = dw_ticks_now - dwTicksLastData) < SOLAREADSGTO);
#endif
#ifndef _DEBUG
			bSolaConnected = ((::GetTickCount() - dwTicksLastData) < SOLAREADSGTO);
#endif
			dwResult = ::WaitForSingleObject(g_hReSyncReqEvent,0);
			bSuccess = ((WAIT_FAILED != dwResult) && bSuccess);
			bSolaConnected = ((WAIT_FAILED != dwResult) && (WAIT_TIMEOUT == dwResult)) ? bSolaConnected : false;
		}  /* End of the secondary while() loop */
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
