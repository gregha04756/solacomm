#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"

CRITICAL_SECTION g_ModConfigCritSect;
const TCHAR* szModConfigQuitEvent = {_T("ModConfigQuitEvent")};
const TCHAR* szModConfigMBRWEvent = {_T("ModConfigMBRWEvent")};
HANDLE g_hModConfigMBRWEvent;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD dwCommThreadID;
HANDLE g_hModConfigQuitEvent;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveConfigPages;
extern "C++" CSolaMBMap* pcSystemConfiguration;
DWORD WINAPI ModConfigThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcModConfiguration;
extern "C++" CSolaMBMap* pcXModConfig;
extern "C++" SOLAMULTIVALUE RegisterAccess[];
extern "C++" SOLAMULTIVALUE ModbusExceptionCodes[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" CSolaPage* pcModConfigPage;
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);

extern "C++" INT_PTR CALLBACK UINumericDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIMultiDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UITimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
extern "C++" INT_PTR CALLBACK UIPercentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);


BOOL UpdatePage(HWND hwndDialog, CSolaPage* lpPage)
{
	BOOL bSuccess = true;
	BOOL bResult;
	BOOL bSelectable;
	HRESULT hRes;
	LRESULT lResult;
	int hh;
	int mm;
	int ss;
	int i;
	int nResult;
	TCHAR szTemp[MAX_LOADSTRING];
	unsigned short usR;
	short sV;
	TCHAR* szS;
	float flDecimal1;
	unsigned short usMask;

		for ( i = 0; i < lpPage->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText(hwndDialog,
				LBLIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
				lpPage->ItemMap(i)->GetParmName(lpPage->ItemIndex(i)));
			bSelectable = bSolaConnected*
				pcTrendStatus->GetValue((int)13)*
				lpPage->ItemMap(i)->GetNonSafety(lpPage->ItemIndex(i))*
				lpPage->ItemMap(i)->GetVisible(lpPage->ItemIndex(i))*
				lpPage->ItemMap(i)->GetWrtable(lpPage->ItemIndex(i));
			switch (lpPage->ItemMap(i)->GetType(lpPage->ItemIndex(i)))
			{
			case CSolaMBMap::Temperature:
				if ( lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp,
						sizeof(szTemp)/sizeof(TCHAR),
						_T("%.2f"),
						TempVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("%.1f"),
							TempVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					}
				}
				lResult = ::SetDlgItemText(hwndDialog,
					TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
					szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog,TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))),
					bSelectable);
				break;
			case CSolaMBMap::TemperatureSetpoint:
				if ( lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp,
						sizeof(szTemp)/sizeof(TCHAR),
						_T("%.2f"),
						TempVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("%.1f"),
							TempVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					}
				}
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Hysteresis:
				if ( lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst,IDS_UNCONFIGURED,szTemp,sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp,
						sizeof(szTemp)/sizeof(TCHAR),
						_T("%.2f"),
						HystVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTemp,
							sizeof(szTemp)/sizeof(TCHAR),
							_T("%.1f"),
							TempVal(pcSystemConfiguration->GetValue(0),lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					}
				}
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Decimal1pl:
				usR = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i));
				flDecimal1 =  (float)lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i));
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%-5.1f"),flDecimal1/10.0);
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)));
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Percentvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%-5.1f"), ((float)lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)))/10.0);
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Bitmask:
				usMask = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i));
				hRes = ::StringCchPrintf(szTemp,
					sizeof(szTemp)/sizeof(TCHAR),
					_T("0x%04x"),
					usMask);
				lResult = ::SetDlgItemText(hwndDialog,
					TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
					szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Multivalue:
				if ( lpPage->ItemMap(i)->GetLPMulti(lpPage->ItemIndex(i)) != NULL )
				{
					usR = lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i));
					sV = lpPage->ItemMap(i)->GetValue(usR);
					szS = lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)usR);
					if ( usR == 0x0273 )
					{
						sV = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i));
						szS = lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)));
					}
					lResult = ::SetDlgItemText(hwndDialog,
						TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
						lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))));
					bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				}
				break;
			case CSolaMBMap::DupStringValue:
			case CSolaMBMap::Stringvalue:
				nResult = MultiByteToWideChar(CP_ACP,
					MB_PRECOMPOSED,
					lpPage->ItemMap(i)->GetLPMap(lpPage->ItemIndex(i))->pchStr,
					-1,
					szTemp,
					sizeof(szTemp)/sizeof(TCHAR));
				lResult = ::SetDlgItemText(hwndDialog,
					TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
					szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog,TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))),
					bSelectable);
				break;
			case CSolaMBMap::Timevalue:
				hh = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))/3600;
				mm = (lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))-(hh*3600))/60;
				ss = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Minutes:
				hh = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i))/60;
				mm = (lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)))-(hh*60);
				ss = 0;
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Unsigned32:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%u"), lpPage->ItemMap(i)->GetU32Value((int)lpPage->ItemIndex(i)));
				lResult = ::SetDlgItemText(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hwndDialog, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			default:
				break;
			}
		}
	return bSuccess;
}

LRESULT CALLBACK ModConfigDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bOkCRC;
	static int nPageIndex;
	static BOOL bStatus;
	BOOL bResult;
	LPNMHDR     lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	int k;
	DWORD dwResult;
	static DWORD dwModConfigThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hModConfigThread;
	static HANDLE hModConfigThreadDup;
	signed short ssValue;
	unsigned short usRegAddr;
	static MBSNDRCVREQ MBSndRcvReq;
	static unsigned char* pchToSnd;
	static unsigned char* pchEndSnd;
	static unsigned char* pchToRcv;
	static unsigned char* pchEndRcv;
	static CSolaMBMap::LPNUMERICUIPARMS lpUIParms;
	static LPMULTIUIPARMS lpMUIParms;
	INT_PTR ipResult;
	CSolaMBMap::SolaType stType;
	CSolaMBMap* lpMap;
	static int nControlIDMin;
	static int nControlIDMax;
	int nResult;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		::InitializeCriticalSection(&g_ModConfigCritSect);
		g_hModConfigMBRWEvent = ::CreateEvent(NULL, false, false, szModConfigMBRWEvent);
		bStatus = false;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_MODCONFIGDLG);
		g_hModConfigQuitEvent = ::CreateEvent(NULL, true, false, szModConfigQuitEvent);
		bResult = ::ResetEvent(g_hModConfigQuitEvent);
		// Start Modulation Config update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hConfigChangeEvent;
		pSummaryParms->hQuitEvent = g_hModConfigQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hModConfigThread = ::CreateThread(NULL, 0, ModConfigThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwModConfigThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hModConfigThread, GetCurrentProcess(), &hModConfigThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hModConfigThread);
		bResult = UpdatePage(hDlg,pcModConfigPage);
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveConfigPages++;
		g_lpPageDataEvents[nPageIndex].typePage = ConfigPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected )
		{
			bResult = ::PostMessage(hDlg, WM_APPMODCONFIGUPD, (WPARAM)1, (LPARAM)0);
		}
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPMODCONFIGUPD;
		nControlIDMin = TXTIDBASE + 0x0fff;
		nControlIDMax = TXTIDBASE;
		for ( i = 0; i < pcModConfigPage->GetSize(); i++ )
		{
			nResult = TXTIDBASE + pcModConfigPage->ItemMap(i)->GetStartRegAddr(pcModConfigPage->ItemIndex(i));
			nControlIDMin = nControlIDMin < nResult ? nControlIDMin : nResult;
			nControlIDMax = nControlIDMax > nResult ? nControlIDMax : nResult;
		}
		Make_Reg_Group_List(pcModConfigPage);
		break;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcModConfigPage);
		bResult = UpdatePage(hDlg,pcModConfigPage);
		break;
	case WM_APPMODCONFIGUPD:
		bResult = UpdatePage(hDlg,pcModConfigPage);
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
			if ( bOkCRC && ((unsigned char)0x80 & chMBRcvBuf[1]) )
			{
				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
			}
		}
		if ( LOWORD(wParam) == 4 )
		{
			if ( (unsigned char)0x80 & chMBRcvBuf[1] )
			{
				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
			}
		}
		break;
	case WM_CTLCOLORSTATIC:
		{
				int nResult = ::SetBkMode((HDC)wParam,TRANSPARENT);
				return (LRESULT)CreateSolidBrush(0xFFFFFF);
		}
	case WM_COMMAND:
		if ( (HIWORD(wParam) == STN_CLICKED) && (LOWORD(wParam) >= nControlIDMin && LOWORD(wParam) <= nControlIDMax) )
		{
			ipResult = IDCANCEL;
			usRegAddr = (unsigned short)(LOWORD(wParam)-TXTIDBASE);
			stType = CSolaMBMap::Novalue;
			lpMap = NULL;
			for ( i = 0; i < pcModConfigPage->GetSize() && !lpMap; i++ )
			{
				if ( pcModConfigPage->ItemMap(i)->GetStartRegAddr(pcModConfigPage->ItemIndex(i)) == usRegAddr )
				{
					lpMap = pcModConfigPage->ItemMap(i);
				}
			}
			stType = (lpMap == NULL) ? CSolaMBMap::Novalue : lpMap->GetType(usRegAddr);
			switch (stType)
			{
			case CSolaMBMap::TemperatureSetpoint:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Hysteresis:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst,MAKEINTRESOURCE(IDD_UINUMERIC),hDlg,UINumericDlgProc,(LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
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
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = (short)lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UINUMERIC), hDlg, UINumericDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Percentvalue:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = (short)lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_CONFIGUIPERCENT), hDlg, UIPercentDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Timevalue:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UITIME), hDlg, UITimeDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
				break;
			case CSolaMBMap::Minutes:
				lpUIParms = new CSolaMBMap::NUMERICUIPARMS;
				lpUIParms->ssValue = lpMap->GetValue(usRegAddr);
				lpUIParms->szParmName = lpMap->GetParmName(usRegAddr);
				lpUIParms->st = stType;
				ipResult = ::DialogBoxParam(g_hInst, MAKEINTRESOURCE(IDD_UITIME), hDlg, UITimeDlgProc, (LPARAM)lpUIParms);
				ssValue = lpUIParms->ssValue;
				delete lpUIParms;
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
		break;

	case WM_NOTIFY:
		lppsn = (LPPSHNOTIFY) lParam;
		lpnmhdr = (LPNMHDR)&(lppsn->hdr);

		switch (lpnmhdr->code)
		{
		case PSN_APPLY:   //sent when OK or Apply button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hModConfigThread )
				{
					bResult = ::SetEvent(g_hModConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hModConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Mod. Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hModConfigThread);
					bResult = ::CloseHandle(hModConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hModConfigQuitEvent);
				bResult = ::CloseHandle(g_hModConfigMBRWEvent);
				::DeleteCriticalSection(&g_ModConfigCritSect);
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hModConfigThread )
				{
					bResult = ::SetEvent(g_hModConfigQuitEvent);
					dwResult = ::WaitForSingleObject(hModConfigThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Mod. Config update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hModConfigThread);
					bResult = ::CloseHandle(hModConfigThreadDup);
				}
				bResult = ::CloseHandle(g_hModConfigQuitEvent);
				bResult = ::CloseHandle(g_hModConfigMBRWEvent);
				::DeleteCriticalSection(&g_ModConfigCritSect);
			}
			break;
         
		case PSN_SETACTIVE:
			PropSheet_UnChanged(::GetParent(hDlg), hDlg);
			break;

		default:
			break;
		}
		break;

	default:
		break;
	}

	return FALSE;
}

DWORD WINAPI ModConfigThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hModConfigQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
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
			bResult = ::PostMessage(hParentWnd, WM_APPMODCONFIGUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("Mod Config thread PostMessage error"), szTitle, MB_OK);
			}
		}
//		bResult = ::SetEvent(::g_hPageUpdEvents[nPage]);
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
