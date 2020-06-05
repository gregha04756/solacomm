#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"

CRITICAL_SECTION g_LLStatusCritSect;
const TCHAR* szLLStatusQuitEvent = {_T("LLStatusQuitEvent")};
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD dwCommThreadID;
HANDLE g_hLLStatusQuitEvent;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hStatusChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveStatusPages;
extern "C++" CSolaMBMap* pcSystemConfiguration;
DWORD WINAPI LLStatusThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" CSolaMBMap::SOLAMBMAP TrendStatus[];
extern "C++" SOLAMULTIVALUE RegisterAccess[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" CSolaMBMap* pcLLStatus;
extern "C++" CSolaMBMap* pcXLLStatus;

LRESULT CALLBACK LLStatusDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	HRESULT hRes;
	static int nPageIndex;
	static BOOL bStatus;
	BOOL bResult;
	LPNMHDR     lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	int k;
	int hh;
	int mm;
	int ss;
	DWORD dwResult;
	static DWORD dwLLStatusThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hLLStatusThread;
	static HANDLE hLLStatusThreadDup;
	TCHAR szTemp[MAX_LOADSTRING];
	static BOOL fStop;

	switch (uMessage)
	{
	case WM_INITDIALOG:
		::InitializeCriticalSection(&g_LLStatusCritSect);
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg), PSM_IDTOINDEX, 0, (LPARAM)IDD_LLSTATUSDLG);
		g_hLLStatusQuitEvent = ::CreateEvent(NULL, true, false, szLLStatusQuitEvent);
		bResult = ::ResetEvent(g_hLLStatusQuitEvent);
		// Start LL Status update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hStatusChangeEvent;
		pSummaryParms->hQuitEvent = g_hLLStatusQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hLLStatusThread = ::CreateThread(NULL, 0, LLStatusThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwLLStatusThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hLLStatusThread, GetCurrentProcess(), &hLLStatusThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hLLStatusThread);
		for ( i = 0; i < pcLLStatus->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText( hDlg, LBLIDBASE+(pcLLStatus->GetStartRegAddr(i)), pcLLStatus->GetParmName(i));
			if ( !bResult )
			{
				hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
				k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
			}
		}
		for ( i = 0; i < pcXLLStatus->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText( hDlg, LBLIDBASE+(pcXLLStatus->GetStartRegAddr(i)), pcXLLStatus->GetParmName(i));
#if defined (_DEBUG)
			if ( !bResult )
			{
				hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcXLLStatus->GetStartRegAddr(i));
				k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
			}
#endif
		}

		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d %d"), pcLLStatus->GetSize(), pcXLLStatus->GetSize());
		bResult = ::SetDlgItemText(hDlg, IDC_TXTXLLSTATUSARRAYSIZE, szTemp);

		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveStatusPages++;
		g_lpPageDataEvents[nPageIndex].typePage = StatusPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected )
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPLLSTATUSUPD;
		{
			bResult = ::PostMessage(hDlg, WM_APPLLSTATUSUPD, (WPARAM)1, (LPARAM)0);
		}
		fStop = false;
		break;
	case  WM_CHILDACTIVATE:
		for ( i = 0; i < pcLLStatus->GetSize(); i++ )
		{
			switch (pcLLStatus->GetType(i))
			{
			case CSolaMBMap::Temperature:
				if ( pcLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::TemperatureSetpoint:
				if ( pcLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Hysteresis:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), HystVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcLLStatus->GetValue(i));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Multivalue:
				if ( pcLLStatus->GetLPMulti(i) != NULL )
				{
#if _DEBUG
					if (2 == i)
					{
						k = i;
						hRes = ::StringCchPrintf(szTemp, sizeof(szTemp) / sizeof(TCHAR), _T("%d"), pcLLStatus->GetStartRegAddr(i));
						k = pcLLStatus->GetValue(i);
						hRes = ::StringCchPrintf(szTemp, sizeof(szTemp) / sizeof(TCHAR), _T("%s"), pcLLStatus->GetLPMulti(i)[pcLLStatus->GetValue(i)].szString);
					}
#endif
					bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), pcLLStatus->GetLPMulti(i)[pcLLStatus->GetValue(i)].szString);
					if ( !bResult )
					{
						hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
						k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
					}
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				}
				break;
			case CSolaMBMap::Bitmask:
				if ( pcLLStatus->GetLPMulti(i) != NULL )
				{
					bResult = ::SetDlgItemText(hDlg,
						TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)),
						pcLLStatus->GetMultiValueItem(i,pcLLStatus->GetSetBitNumber(i)));
					if ( !bResult )
					{
						hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
						k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
					}
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				}
				break;
			case CSolaMBMap::Timevalue:
				hh = pcLLStatus->GetValue(i)/3600;
				mm = (pcLLStatus->GetValue(i)-(hh*3600))/60;
				ss = pcLLStatus->GetValue(i)-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			default:
				break;
			}
		}

		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d %d"), pcLLStatus->GetSize(), pcXLLStatus->GetSize());
		bResult = ::SetDlgItemText(hDlg, IDC_TXTXLLSTATUSARRAYSIZE, szTemp);

		for ( i = 0; i < pcXLLStatus->GetSize(); i++ )
		{
			switch (pcXLLStatus->GetType(i))
			{
			case CSolaMBMap::Temperature:
				if ( pcXLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcXLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::TemperatureSetpoint:
				if ( pcXLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcXLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Hysteresis:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), HystVal(pcSystemConfiguration->GetValue((int)0),pcXLLStatus->GetValue(i)));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcXLLStatus->GetValue(i));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Multivalue:
				if ( pcXLLStatus->GetLPMulti(i) != NULL )
				{
					bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), pcXLLStatus->GetLPMulti(i)[pcXLLStatus->GetValue(i)].szString);
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				}
				break;
			case CSolaMBMap::Bitmask:
				break;
			case CSolaMBMap::Timevalue:
				hh = pcXLLStatus->GetValue(i)/3600;
				mm = (pcXLLStatus->GetValue(i)-(hh*3600))/60;
				ss = pcXLLStatus->GetValue(i)-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			default:
				break;
			}
		}
		break;
	case WM_APPLLSTATUSUPD:
		if ( fStop )
		{
			fStop = false;
		}
		for ( i = 0; i < pcLLStatus->GetSize(); i++ )
		{
			switch (pcLLStatus->GetType(i))
			{
			case CSolaMBMap::Temperature:
				if ( pcLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::TemperatureSetpoint:
				if ( pcLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Hysteresis:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), HystVal(pcSystemConfiguration->GetValue((int)0), pcLLStatus->GetValue(i)));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcLLStatus->GetValue(i));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Multivalue:
				if ( pcLLStatus->GetLPMulti(i) != NULL )
				{
					bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), pcLLStatus->GetLPMulti(i)[pcLLStatus->GetValue(i)].szString);
					if ( !bResult )
					{
						hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
						k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
					}
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				}
				break;
			case CSolaMBMap::Bitmask:
				break;
			case CSolaMBMap::Timevalue:
				hh = pcLLStatus->GetValue(i)/3600;
				mm = (pcLLStatus->GetValue(i)-(hh*3600))/60;
				ss = pcLLStatus->GetValue(i)-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i)), szTemp);
				if ( !bResult )
				{
					hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
					k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
				}
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			default:
				break;
			}
		}

		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d %d"), pcLLStatus->GetSize(), pcXLLStatus->GetSize());
		bResult = ::SetDlgItemText(hDlg, IDC_TXTXLLSTATUSARRAYSIZE, szTemp);
		if ( !bResult )
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLLStatus->GetStartRegAddr(i));
			k = ::MessageBox(hDlg,szTemp,szTitle,MB_OK);
		}

		for ( i = 0; i < pcXLLStatus->GetSize(); i++ )
		{
			switch (pcXLLStatus->GetType(i))
			{
			case CSolaMBMap::Temperature:
				if ( pcXLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcXLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::TemperatureSetpoint:
				if ( pcXLLStatus->GetValue(i) == (signed short)UNCONFIGUREDTEMP )
				{
					::LoadString(g_hInst, IDS_UNCONFIGURED, szTemp, sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), TempVal(pcSystemConfiguration->GetValue((int)0), pcXLLStatus->GetValue(i)));
				}
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Hysteresis:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"), HystVal(pcSystemConfiguration->GetValue((int)0),pcXLLStatus->GetValue(i)));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), pcXLLStatus->GetValue(i));
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			case CSolaMBMap::Multivalue:
				if ( pcXLLStatus->GetLPMulti(i) != NULL )
				{
					bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), pcXLLStatus->GetLPMulti(i)[pcXLLStatus->GetValue(i)].szString);
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				}
				break;
			case CSolaMBMap::Bitmask:
				break;
			case CSolaMBMap::Timevalue:
				hh = pcXLLStatus->GetValue(i)/3600;
				mm = (pcXLLStatus->GetValue(i)-(hh*3600))/60;
				ss = pcXLLStatus->GetValue(i)-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i)), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(pcXLLStatus->GetStartRegAddr(i))), bSolaConnected);
				break;
			default:
				break;
			}
		}
		if ( bSolaConnected && !LOWORD(wParam))
		{
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		}
		break;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDC_TXTXLLSTATUSARRAYSIZE && HIWORD(wParam) == STN_CLICKED )
		{
			fStop = true;
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
				if ( hLLStatusThread )
				{
					bResult = ::SetEvent(g_hLLStatusQuitEvent);
					dwResult = ::WaitForSingleObject(hLLStatusThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for LL Status thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLLStatusThread);
					bResult = ::CloseHandle(hLLStatusThreadDup);
				}
				bResult = ::CloseHandle(g_hLLStatusQuitEvent);
				::DeleteCriticalSection(&g_LLStatusCritSect);
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hLLStatusThread )
				{
					bResult = ::SetEvent(g_hLLStatusQuitEvent);
					dwResult = ::WaitForSingleObject(hLLStatusThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for LL Status update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hLLStatusThread);
					bResult = ::CloseHandle(hLLStatusThreadDup);
				}
				bResult = ::CloseHandle(g_hLLStatusQuitEvent);
				::DeleteCriticalSection(&g_LLStatusCritSect);
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

DWORD WINAPI LLStatusThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hLLStatusQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_lpPageDataEvents[nPage].hEvent, GetCurrentProcess(), &hEvents[1], 0, false, DUPLICATE_SAME_ACCESS);
	lpReadDataCritSect = ((LPSUMMARYTHREADPARMS)lpParam)->pDataCritSect;
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
			bResult = ::PostMessage(hParentWnd, WM_APPLLSTATUSUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("LL Status thread PostMessage error"), szTitle, MB_OK);
			}
		}
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
