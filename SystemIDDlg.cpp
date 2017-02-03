#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"
#include "SolaTCPComm.h"

CRITICAL_SECTION g_SystemIDCritSect;
const TCHAR* szSystemIDQuitEvent = {_T("SystemIDQuitEvent")};
const TCHAR* szSystemMBRWEvent = {_T("SystemMBRWEvent")};
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" HANDLE hCOMDup;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
HANDLE g_hSystemIDQuitEvent;
HANDLE g_hSystemIDMBRWEvent;
extern "C++" BOOL g_bQuit;
//extern "C++" HANDLE g_hReadEvent;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveConfigPages;
DWORD WINAPI SystemIDThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
//extern "C++" SOLAMULTIVALUE RegisterAccess[];
//extern "C++" SOLAMBMAP TrendStatus[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" CSolaMBMap* pcXSystemConfig;
extern "C++" CSolaMBMap* pcFlapValveConfig;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDResetRestart;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMultiValue* pcRegisterAccess;
extern "C++" CSolaPage* pcSystemIDPage;
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" BOOL UpdatePage(HWND hwndDialog,CSolaPage* lpPage);
extern "C++" MBConnType mbctMBConn;
extern "C++" enum TCP_Gateway_Type g_TCP_gw_selection;
extern "C++" CSolaTCPComm* lpSolaTCPComm;


INT_PTR CALLBACK SystemIDUIDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UINumericDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIMultiDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UITimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK SystemIDDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	const unsigned char SolaPassword[] = { 0x10, 0x00, 0xb1, 0x00, 0x01 };
	const char chBoiler1[] = "Boiler1";
	HRESULT hRes;
	static int nPageIndex;
	static BOOL bUpdFromSola;
	BOOL bResult;
	BOOL bOkCRC;
	LPNMHDR     lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	int j;
	int cbPwdLen;
	int cbInputLen;
	DWORD dwResult;
	static DWORD dwSystemIDThreadID;
	LRESULT lResult;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hSystemIDThread;
	static HANDLE hSystemIDThreadDup;
	TCHAR szTemp[MAX_LOADSTRING];
	static TCHAR szPassword[21];
	static char chPassword[20];
	static TCHAR szUserInput[21];
	static char chUserInput[20];
	PVOID lpVoid;
	static MBSNDRCVREQ MBSndRcvReq;
	static unsigned char* pchToSnd;
	static unsigned char* pchEndSnd;
	static unsigned char* pchToRcv;
	static unsigned char* pchEndRcv;
	static LPSTRINGUIPARMS lpUIParms;
	static CSolaMBMap::LPNUMERICUIPARMS lpNumUIParms;
	static LPMULTIUIPARMS lpMUIParms;
	UINT uiResult;
	INT_PTR ipResult;
	unsigned short usRegAddr;
	signed short ssValue;
	CSolaMBMap::SolaType stType;
	CSolaMBMap* lpMap;
	static int nControlIDMin;
	static int nControlIDMax;
	int nResult;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		bUpdFromSola = true;
		pchToSnd = pchEndSnd = chMBSndBuf;
		pchToRcv = pchEndRcv = chMBRcvBuf;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_SYSTEMIDDLG);
		g_hSystemIDQuitEvent = ::CreateEvent(NULL, true, false, szSystemIDQuitEvent);
		bResult = ::ResetEvent(g_hSystemIDQuitEvent);
		g_hSystemIDMBRWEvent = ::CreateEvent(NULL, false, false, szSystemMBRWEvent);
		// Start System ID Config update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hConfigChangeEvent;
		pSummaryParms->hQuitEvent = g_hSystemIDQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hSystemIDThread = ::CreateThread(NULL, 0, SystemIDThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwSystemIDThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hSystemIDThread, GetCurrentProcess(), &hSystemIDThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hSystemIDThread);
		bResult = ::SetDlgItemText(hDlg,
			IDC_TXTREGISTERACCESS,
			pcRegisterAccess->GetMultiString(pcTrendStatus->GetValue((int)13)));
		if ( !bSolaConnected )
		{
#if 0
			i = ::sprintf_s((char*)pcSystemIDBurnerName->GetLPMap((int)0)->pchStr,pcSystemIDBurnerName->GetLPMap((int)0)->cbStrLen, "Boiler1");
			i = ::sprintf_s((char*)pcSystemIDInstallationData->GetLPMap((int)0)->pchStr,pcSystemIDInstallationData->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDOEMID->GetLPMap((int)0)->pchStr,pcSystemIDOEMID->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDOSNumber->GetLPMap((int)0)->pchStr,pcSystemIDOSNumber->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDDateCode->GetLPMap((int)0)->pchStr,pcSystemIDDateCode->GetLPMap((int)0)->cbStrLen, "");
#endif
			i = ::sprintf_s((char*)pcSystemIDBurnerName->GetLPMap((int)0)->pchStr,pcSystemIDBurnerName->GetLPMap((int)0)->cbStrLen, "Boiler1");
			i = ::sprintf_s((char*)pcSystemIDInstallationData->GetLPMap((int)0)->pchStr,pcSystemIDInstallationData->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDOEMID->GetLPMap((int)0)->pchStr,pcSystemIDOEMID->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDOSNumber->GetLPMap((int)0)->pchStr,pcSystemIDOSNumber->GetLPMap((int)0)->cbStrLen, "");
			i = ::sprintf_s((char*)pcSystemIDDateCode->GetLPMap((int)0)->pchStr,pcSystemIDDateCode->GetLPMap((int)0)->cbStrLen, "");
		}
		bResult = UpdatePage(hDlg,pcSystemIDPage);
		lpVoid = ::SecureZeroMemory((PVOID)szPassword, sizeof(szPassword));
		lResult = ::SetDlgItemText(hDlg, IDC_EDITLOGIN, szPassword);
		lResult = ::SendMessage(::GetDlgItem(hDlg, IDC_EDITLOGIN), EM_LIMITTEXT, (WPARAM)20, (LPARAM)0);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_EDITLOGIN), bSolaConnected);
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveConfigPages++;
		g_lpPageDataEvents[nPageIndex].typePage = ConfigPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		::InitializeCriticalSection(&g_SystemIDCritSect);
		bResult = ::PostMessage(hDlg, WM_APPSYSTEMIDUPD, (WPARAM)1, (LPARAM)0);
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPSYSTEMIDUPD;
		nControlIDMin = TXTIDBASE + 0x0fff;
		nControlIDMax = TXTIDBASE;
		for ( i = 0; i < pcSystemIDPage->GetSize(); i++ )
		{
			nResult = TXTIDBASE + pcSystemIDPage->ItemMap(i)->GetStartRegAddr(pcSystemIDPage->ItemIndex(i));
			nControlIDMin = nControlIDMin < nResult ? nControlIDMin : nResult;
			nControlIDMax = nControlIDMax > nResult ? nControlIDMax : nResult;
		}
		Make_Reg_Group_List(pcSystemIDPage);
		break;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcSystemIDPage);
		bUpdFromSola = true;
#if 0
		lpVoid = ::SecureZeroMemory(szPassword, sizeof(szPassword)/sizeof(TCHAR));
		lResult = ::SetDlgItemText(hDlg, IDC_EDITLOGIN, szPassword);
#endif
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNLOGIN), bSolaConnected);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_EDITLOGIN), bSolaConnected);
		bResult = ::SetDlgItemText(hDlg, IDC_TXTREGISTERACCESS, pcRegisterAccess->GetMultiString(pcTrendStatus->GetLPMap(13)->sValue));
		if ( bSolaConnected )
		{
			bResult = UpdatePage(hDlg,pcSystemIDPage);
		}
		return (LRESULT)0;
		break;
	case WM_APPSYSTEMIDUPD:
		bUpdFromSola = true;
#if 0
		lpVoid = ::SecureZeroMemory(szPassword, sizeof(szPassword)/sizeof(TCHAR));
		lResult = ::SetDlgItemText(hDlg, IDC_EDITLOGIN, szPassword);
#endif
		lpVoid = ::SecureZeroMemory(szTemp, sizeof(szTemp)/sizeof(TCHAR));
		dwResult = (DWORD)GetWindowText(::GetParent(hDlg),szTemp,sizeof(szTemp)/sizeof(TCHAR));
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNLOGIN), bSolaConnected);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_EDITLOGIN), bSolaConnected);
		bResult = ::SetDlgItemText(hDlg, IDC_TXTREGISTERACCESS, pcRegisterAccess->GetMultiString(pcTrendStatus->GetLPMap(13)->sValue));
		if (bSolaConnected)
		{
			if ((mbctMBConn == RTU_Serial_Direct) || (mbctMBConn == RTU_Serial_Gateway))
			{
				j = MultiByteToWideChar(
					CP_ACP,
					MB_PRECOMPOSED,
					pcSystemIDBurnerName->GetLPMap(0)->pchStr,
					-1,
					szTemp,
					sizeof(szTemp)/sizeof(TCHAR));
			}
			if ((mbctMBConn == TCP) && (NULL != lpSolaTCPComm))
			{
				if (TCP_Protonode == g_TCP_gw_selection)
				{
					j = StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),lpSolaTCPComm->GetBurnerName());
				}
				if (TCP_Protonode != g_TCP_gw_selection)
				{
					j = MultiByteToWideChar(
						CP_ACP,
						MB_PRECOMPOSED,
						pcSystemIDBurnerName->GetLPMap(0)->pchStr,
						-1,
						szTemp,
						sizeof(szTemp)/sizeof(TCHAR));
				}
			}
		}
		bResult = ::SetWindowText(::GetParent(hDlg),szTemp);
		bResult = UpdatePage(hDlg,pcSystemIDPage);
		if ( bSolaConnected && !LOWORD(wParam))
		{
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		}
		break;
	case WM_APPMBRESPONSE:
		if ( LOWORD(wParam) == 3 )
		{
			::EnterCriticalSection(&gRWDataCritSect);
			bOkCRC =  check_CRC16(chMBRcvBuf, (int)(pchEndRcv-pchToRcv));
			::LeaveCriticalSection(&gRWDataCritSect);
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
				int nResult = ::SetBkMode((HDC)wParam,TRANSPARENT);
				return (LRESULT)CreateSolidBrush(0xFFFFFF);
		}
	case WM_COMMAND:
		if ( LOWORD(wParam) >= nControlIDMin && LOWORD(wParam) <= nControlIDMax && HIWORD(wParam) == STN_CLICKED )
		{
			ipResult = IDCANCEL;
			usRegAddr = (unsigned short)(LOWORD(wParam)-TXTIDBASE);
			stType = CSolaMBMap::Novalue;
			lpMap = NULL;
			for ( i = 0; i < pcSystemIDPage->GetSize() && !lpMap; i++ )
			{
				if ( pcSystemIDPage->ItemMap(i)->GetStartRegAddr(pcSystemIDPage->ItemIndex(i)) == usRegAddr )
				{
					lpMap = pcSystemIDPage->ItemMap(i);
				}
			}
			stType = (lpMap == NULL) ? CSolaMBMap::Novalue : lpMap->GetType(usRegAddr);
			switch (stType)
			{
			case CSolaMBMap::TemperatureSetpoint:
				lpNumUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpNumUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpNumUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpNumUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpNumUIParms);
				ssValue = lpNumUIParms->ssValue;
				delete lpNumUIParms;
				break;
			case CSolaMBMap::Hysteresis:
				lpNumUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpNumUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpNumUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpNumUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpNumUIParms);
				ssValue = lpNumUIParms->ssValue;
				delete lpNumUIParms;
				break;
			case CSolaMBMap::Multivalue:
				lpMUIParms = new MULTIUIPARMS;
				lpMUIParms->lpMultiList = lpMap->GetLPMulti(usRegAddr);
				lpMUIParms->nCurSel = lpMap->GetValue(usRegAddr);
				lpMUIParms->nMultiListSize = lpMap->GetMultiListSize(usRegAddr);
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UIMULTI), hDlg, UIMultiDlgProc, (LPARAM)lpMUIParms);
				ssValue = lpMUIParms->nCurSel;
				delete lpMUIParms;
				break;
			case CSolaMBMap::Numericvalue:
				lpNumUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpNumUIParms->ssValue = (short)lpMap->GetValue(usRegAddr);
				lpNumUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpNumUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UINUMERIC), hDlg, UINumericDlgProc, (LPARAM)lpNumUIParms);
				ssValue = lpNumUIParms->ssValue;
				delete lpNumUIParms;
				break;
			case CSolaMBMap::Timevalue:
				lpNumUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpNumUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpNumUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpNumUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UITIME), hDlg, UITimeDlgProc, (LPARAM)lpNumUIParms);
				ssValue = lpNumUIParms->ssValue;
				delete lpNumUIParms;
				break;
			}
			if ( ipResult == IDOK )
			{
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x06;
				*(*MBSndRcvReq.ppchEndSnd)++ = (usRegAddr >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = usRegAddr & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = (ssValue >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = ssValue & 0x00ff;
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
			}
		}
		if ( LOWORD(wParam) == IDC_BTNLOGIN && HIWORD(wParam) == BN_CLICKED )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNLOGIN), false);
			if ( (cbPwdLen = ::GetDlgItemText(hDlg, IDC_EDITLOGIN, szPassword, sizeof(szPassword)/sizeof(TCHAR))) > 0 )
			{
				i = ::WideCharToMultiByte( CP_ACP, 0, szPassword, sizeof(szPassword)/sizeof(TCHAR), chPassword, sizeof(chPassword)/sizeof(char), NULL, NULL);
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				for ( j = 0; j < sizeof(SolaPassword); j++ )
				{
					*(*MBSndRcvReq.ppchEndSnd)++ = SolaPassword[j];
				}
				*(*MBSndRcvReq.ppchEndSnd)++ = (unsigned char) cbPwdLen;
				for ( j = 0; j < cbPwdLen && *MBSndRcvReq.ppchEndSnd < chMBSndBuf + sizeof(chMBSndBuf); j++ )
				{
					*(*MBSndRcvReq.ppchEndSnd)++ = chPassword[j];
				}
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
			}
			lpVoid = ::SecureZeroMemory(szPassword, sizeof(szPassword)/sizeof(TCHAR));
			lResult = ::SetDlgItemText(hDlg, IDC_EDITLOGIN, szPassword);
			bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_BTNLOGIN), true);
		}
		if ( LOWORD(wParam) == IDC_TXTBURNERNAME && HIWORD(wParam) == STN_CLICKED )
		{
			uiResult = ::GetDlgItemText(hDlg,IDC_TXTBURNERNAME,szUserInput,sizeof(szUserInput));
			lpUIParms = new STRINGUIPARMS;
			lpUIParms->szTxt = szUserInput;
			lpUIParms->cchLen = sizeof(szUserInput)/sizeof(TCHAR);
			lpUIParms->szParmName = pcSystemIDBurnerName[0].GetParmName((int)0);
			ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SYSTEMIDUIBOX), hDlg, SystemIDUIDlgProc, (LPARAM)lpUIParms);
			delete lpUIParms;
			if ( ipResult == IDOK )
			{
				hRes = ::StringCchLength(szUserInput,sizeof(szUserInput)/sizeof(TCHAR),(size_t*)&cbInputLen);
				if ( cbInputLen == 0 )
				{
					cbInputLen = 20;
					lpVoid = ::SecureZeroMemory((PVOID)chUserInput, sizeof(chUserInput)/sizeof(char));
				}
				else
				{
					i = ::WideCharToMultiByte( CP_ACP, 0, szUserInput, sizeof(szUserInput)/sizeof(TCHAR), chUserInput, sizeof(chUserInput)/sizeof(char), NULL, NULL);
				}
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x10;
				*(*MBSndRcvReq.ppchEndSnd)++ = (pcSystemIDBurnerName->GetLPMap((int)0)->usStartRegAddr >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = pcSystemIDBurnerName->GetLPMap((int)0)->usStartRegAddr & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x00;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x01;
				*(*MBSndRcvReq.ppchEndSnd)++ = (unsigned char) cbInputLen;
				for ( j = 0; j < cbInputLen && *MBSndRcvReq.ppchEndSnd < chMBSndBuf + sizeof(chMBSndBuf); j++ )
				{
					*(*MBSndRcvReq.ppchEndSnd)++ = chUserInput[j];
				}
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
			}
		}
		if ( LOWORD(wParam) == IDC_TXTINSTALLATIONDATA && HIWORD(wParam) == STN_CLICKED )
		{
			uiResult = ::GetDlgItemText(hDlg,IDC_TXTINSTALLATIONDATA,szUserInput,sizeof(szUserInput));
			lpUIParms = new STRINGUIPARMS;
			lpUIParms->szTxt = szUserInput;
			lpUIParms->cchLen = sizeof(szUserInput)/sizeof(TCHAR);
			lpUIParms->szParmName = pcSystemIDInstallationData[0].GetParmName((int)0);
			ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_SYSTEMIDUIBOX), hDlg, SystemIDUIDlgProc, (LPARAM)lpUIParms);
			delete lpUIParms;
			if ( ipResult == IDOK )
			{
				hRes = ::StringCchLength(szUserInput,sizeof(szUserInput)/sizeof(TCHAR),(size_t*)&cbInputLen);
				if ( cbInputLen == 0 )
				{
					cbInputLen = 20;
					lpVoid = ::SecureZeroMemory((PVOID)chUserInput, sizeof(chUserInput)/sizeof(char));
				}
				else
				{
					i = ::WideCharToMultiByte( CP_ACP, 0, szUserInput, sizeof(szUserInput)/sizeof(TCHAR), chUserInput, sizeof(chUserInput)/sizeof(char), NULL, NULL);
				}
				::EnterCriticalSection(&gRWDataCritSect);
				MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
				MBSndRcvReq.ppchToSnd = &pchToSnd;
				MBSndRcvReq.ppchEndSnd = &pchEndSnd;
				MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
				*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x10;
				*(*MBSndRcvReq.ppchEndSnd)++ = (pcSystemIDInstallationData->GetLPMap((int)0)->usStartRegAddr >> 8) & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = pcSystemIDInstallationData->GetLPMap((int)0)->usStartRegAddr & 0x00ff;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x00;
				*(*MBSndRcvReq.ppchEndSnd)++ = 0x01;
				*(*MBSndRcvReq.ppchEndSnd)++ = (unsigned char) cbInputLen;
				for ( j = 0; j < cbInputLen && *MBSndRcvReq.ppchEndSnd < chMBSndBuf + sizeof(chMBSndBuf); j++ )
				{
					*(*MBSndRcvReq.ppchEndSnd)++ = chUserInput[j];
				}
				MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
				MBSndRcvReq.ppchToRcv = &pchToRcv;
				MBSndRcvReq.ppchEndRcv = &pchEndRcv;
				MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
				MBSndRcvReq.hPage = hDlg;
				MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
				g_MBSndRcvReqQ.push(MBSndRcvReq);
				::LeaveCriticalSection(&gRWDataCritSect);
			}
		}
		return (INT_PTR) true;
		break;

	case WM_NOTIFY:
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hSystemIDThread )
				{
					bResult = ::SetEvent(g_hSystemIDQuitEvent);
					dwResult = ::WaitForSingleObject(hSystemIDThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for System ID update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hSystemIDThread);
					bResult = ::CloseHandle(hSystemIDThreadDup);
				}
				bResult = ::CloseHandle(g_hSystemIDQuitEvent);
				bResult = ::CloseHandle(g_hSystemIDMBRWEvent);
				::DeleteCriticalSection(&g_SystemIDCritSect);
			}
			if ( !lppsn->lParam )
			{
				i = 0;
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hSystemIDThread )
				{
					bResult = ::SetEvent(g_hSystemIDQuitEvent);
					dwResult = ::WaitForSingleObject(hSystemIDThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for System ID update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hSystemIDThread);
					bResult = ::CloseHandle(hSystemIDThreadDup);
				}
				bResult = ::CloseHandle(g_hSystemIDQuitEvent);
				bResult = ::CloseHandle(g_hSystemIDMBRWEvent);
				::DeleteCriticalSection(&g_SystemIDCritSect);
			}
			break;
         
		case PSN_SETACTIVE:
			PropSheet_UnChanged(::GetParent(hDlg), hDlg);
			break;

		default:
			break;
		}
		return (INT_PTR) true;
		break;

	default:
		break;
	}

	return (INT_PTR) FALSE;
}

INT_PTR CALLBACK SystemIDUIDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	int nResult;
	UINT uiResult;
	HRESULT hRes;
	TCHAR szTxt[MAX_LOADSTRING];
	TCHAR szTemp[MAX_LOADSTRING];
	static LPSTRINGUIPARMS lpParms;

	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (LPSTRINGUIPARMS)lParam;
		nResult = ::GetWindowText(hDlg,szTxt,sizeof(szTxt)/sizeof(TCHAR));
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s %s"), szTxt, lpParms->szParmName);
		nResult = ::SetWindowText(hDlg,szTemp);
		bResult = ::SetDlgItemText(hDlg,IDC_SYSTEMIDUIEDIT,lpParms->szTxt);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			uiResult = ::GetDlgItemText(hDlg,IDC_SYSTEMIDUIEDIT,szTxt,sizeof(szTxt));
			hRes = ::StringCchPrintf(lpParms->szTxt,lpParms->cchLen,_T("%s"),szTxt);
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

DWORD WINAPI SystemIDThread(LPVOID lpParam)
{
	BOOL bResult;
	BOOL bSuccess = true;
	int nPage;
	DWORD dwResult;
	HANDLE hEvents[2];
	HWND hParentWnd;
	LPCRITICAL_SECTION lpReadDataCritSect;

	hParentWnd = ((LPSUMMARYTHREADPARMS) lpParam)->hParentWnd;
	nPage = ((LPSUMMARYTHREADPARMS) lpParam)->nPg;
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hSystemIDQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_lpPageDataEvents[nPage].hEvent, GetCurrentProcess(), &hEvents[1], 0, false, DUPLICATE_SAME_ACCESS);
	lpReadDataCritSect= ((LPSUMMARYTHREADPARMS) lpParam)->pDataCritSect;
	delete ((LPSUMMARYTHREADPARMS) lpParam);

	while ( !g_bQuit && bSuccess && ((dwResult = ::WaitForSingleObject(hEvents[0], 0))==WAIT_TIMEOUT) )
	{
		dwResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE), hEvents, false, INFINITE);
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[0] || dwResult == WAIT_FAILED )
		{
			bResult = ::CloseHandle(hEvents[0]);
			bResult = ::CloseHandle(hEvents[1]);
			return 0;
		}
		if ( hEvents[dwResult-WAIT_OBJECT_0] == hEvents[1] )
		{
			::EnterCriticalSection(&g_SystemIDCritSect);
			bResult = ::PostMessage(hParentWnd, WM_APPSYSTEMIDUPD, (WPARAM) 0, (LPARAM) 0);
			::LeaveCriticalSection(&g_SystemIDCritSect);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("System ID thread PostMessage error"), szTitle, MB_OK);
			}
		}
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
