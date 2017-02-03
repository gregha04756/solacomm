#include "StdAfx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaMultiValue.h"
#include "SolaLockout.h"
#include "SolaLockoutDesc.h"
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" CSolaMultiValue* pcSolaLockoutDesc;
extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerAnnunFirstOutCodes;
extern "C++" CSolaMultiValue* pcBurnerAnnunHoldCodes;
extern "C++" CSolaMultiValue* pcDigitalIOCodes;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" float TempVal(BOOL units, short temp);
extern "C++" short ssTempVal (BOOL units, short temp);

INT_PTR CALLBACK LockoutRecordDisplayDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	const U16 u16Mask = 0x703f;
	LPLOCKOUTRECORDDSPPARMS pnd;
	int i;
	static int nLockoutRecordIndex;
	BOOL bResult;
	HRESULT hRes;
	TCHAR szTemp[100];
	U16 u16Bit;
	U16 u16LockoutCode;
	U16 u16LockoutState;
	U16 u16DigitalIO;
	LPSOLALOCKOUT lpsl;
	switch (message)
	{
	case WM_INITDIALOG:
		pnd = (LPLOCKOUTRECORDDSPPARMS)lParam;
		nLockoutRecordIndex = pnd->nIndex;
		delete pnd;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.ulCycle);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTCYCLE,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.ulHours);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTHOURS,szTemp);
		lpsl = pcLockoutLog->GetLPMap(nLockoutRecordIndex);
//		u16LockoutCode = pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.u16LockoutCode;
		u16LockoutCode = lpsl->pLockoutUnion->slr.usLockoutCode;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d %s"),u16LockoutCode,pcSolaLockoutDesc->GetMultiString(u16LockoutCode));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTCODE,szTemp);
		u16LockoutState = pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usBurnerControlState;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),pcBurnerControlStateValues->GetMultiString(u16LockoutState));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTSTATE,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usSequenceTime);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTSEQTIME,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),
			pcBurnerAnnunFirstOutCodes->GetMultiString(pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usAnnunciatorFirstOut));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUT1STOUT,szTemp);
		// Digital I/O
		u16Bit = 1;
		u16DigitalIO = lpsl->pLockoutUnion->slr.usIO;
		for ( i = 0; i < sizeof(U16)*8; i++ )
		{
			bResult = ::SetDlgItemText(hDlg,LBLIDBASE + u16Bit,pcDigitalIOCodes->GetMultiString(i));
			if ( u16Bit & u16DigitalIO & u16Mask )
			{
				bResult = ::SetDlgItemText(hDlg,TXTLRDIDBASE + i,_T("ON"));
			}
			else
			{
				bResult = ::SetDlgItemText(hDlg,TXTLRDIDBASE + i,_T("OFF"));
			}
			u16Bit <<= 1;
		}
		// Temperatures
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usInletTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDINLET,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usOutletTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDOUTLET,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usDHWTemperature);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDDHW,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usODTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDOUTDOOR,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.usStackTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDSTACK,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.us4to20mAInput);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRD420MA,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("0x%02x 0x%02x"), (short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.ucFaultData0,
			(short)pcLockoutLog->GetLPMap(nLockoutRecordIndex)->pLockoutUnion->slr.ucFaultData1);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDDATA,szTemp);
		return (INT_PTR)TRUE;

	case WM_CTLCOLORSTATIC:
		{
			if ( (HWND)lParam == ::GetDlgItem(hDlg,IDC_TXTLOCKOUTCODE) )
			{
				HDC hdcStatic = (HDC)wParam;
				int i = ::SetBkMode(hdcStatic,TRANSPARENT);
				COLORREF cr = ::SetTextColor(hdcStatic,RGB(0xff,0x00,0x00));
				return (INT_PTR)CreateSolidBrush(RGB(0xff,0xff,0xff));
//				return (INT_PTR)::GetStockObject(LTGRAY_BRUSH);
			}
		}
		return (INT_PTR)true;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			bResult = ::EndDialog(hDlg,LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

#if 0
#include "LockoutRecordDisplay.h"

CLockoutRecordDisplay::CLockoutRecordDisplay(void)
{
	this->m_hdupLockoutRecordDisplayThread = NULL;
	this->m_hLockoutRecordDisplayThread = NULL;
	this->m_hInst = NULL;
	this->m_hwndOwner = NULL;
	this->m_szTitle = NULL;
}

CLockoutRecordDisplay::CLockoutRecordDisplay(HWND hOwner, HINSTANCE hInst,TCHAR* szTitle, CSolaMultiValue* psld):m_hwndOwner(hOwner), m_hInst(hInst), m_szTitle(szTitle), m_psld(psld)
{
	this->m_hdupLockoutRecordDisplayThread = NULL;
	this->m_hLockoutRecordDisplayThread = NULL;
	this->m_dwLockoutRecordDisplayThreadID = 0;
}

CLockoutRecordDisplay::~CLockoutRecordDisplay(void)
{
	BOOL bResult = Stop();
}

INT_PTR CALLBACK CLockoutRecordDisplay::LockoutRecordDisplayDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	const U16 u16Mask = 0x703f;
	static CLockoutRecordDisplay* pnd;
	int i;
	BOOL bResult;
	HRESULT hRes;
	TCHAR szTemp[100];
	U16 u16Bit;
	U16 u16LockoutCode;
	U16 u16LockoutState;
	U16 u16DigitalIO;
	LPSOLALOCKOUT lpsl;
	switch (message)
	{
	case WM_INITDIALOG:
		pnd = (CLockoutRecordDisplay*)lParam;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.ulCycle);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTCYCLE,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.ulHours);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTHOURS,szTemp);
		lpsl = pnd->m_pcsl->GetLPMap(pnd->m_slndx);
//		u16LockoutCode = pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.u16LockoutCode;
		u16LockoutCode = lpsl->pLockoutUnion->slr.usLockoutCode;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d %s"),u16LockoutCode,pnd->m_psld->GetMultiString(u16LockoutCode));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTCODE,szTemp);
		u16LockoutState = pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usBurnerControlState;
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),pcBurnerControlStateValues->GetMultiString(u16LockoutState));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTSTATE,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%d"),pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usSequenceTime);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUTSEQTIME,szTemp);
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s"),
			pcBurnerAnnunFirstOutCodes->GetMultiString(pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usAnnunciatorFirstOut));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOCKOUT1STOUT,szTemp);
		// Digital I/O
		u16Bit = 1;
		u16DigitalIO = lpsl->pLockoutUnion->slr.usIO;
		for ( i = 0; i < sizeof(U16)*8; i++ )
		{
			bResult = ::SetDlgItemText(hDlg,LBLIDBASE + u16Bit,pcDigitalIOCodes->GetMultiString(i));
			if ( u16Bit & u16DigitalIO & u16Mask )
			{
				bResult = ::SetDlgItemText(hDlg,TXTLRDIDBASE + i,_T("ON"));
			}
			else
			{
				bResult = ::SetDlgItemText(hDlg,TXTLRDIDBASE + i,_T("OFF"));
			}
			u16Bit <<= 1;
		}
		// Temperatures
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usInletTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDINLET,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usOutletTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDOUTLET,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usDHWTemperature);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDDHW,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usODTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDOUTDOOR,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02.0f"),
			TempVal(pcSystemConfiguration->GetLPMap((int)0)->sValue, (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.usStackTemperature));
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDSTACK,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%d"), (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.us4to20mAInput);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRD420MA,szTemp);
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("0x%02x 0x%02x"), (short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.ucFaultData0,
			(short)pnd->m_pcsl->GetLPMap(pnd->m_slndx)->pLockoutUnion->slr.ucFaultData1);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLRDDATA,szTemp);
		return (INT_PTR)TRUE;

	case WM_CTLCOLORSTATIC:
		{
			if ( (HWND)lParam == ::GetDlgItem(hDlg,IDC_TXTLOCKOUTCODE) )
			{
				HDC hdcStatic = (HDC)wParam;
				int i = ::SetBkMode(hdcStatic,TRANSPARENT);
				COLORREF cr = ::SetTextColor(hdcStatic,RGB(0xff,0x00,0x00));
				return (INT_PTR)CreateSolidBrush(RGB(0xff,0xff,0xff));
//				return (INT_PTR)::GetStockObject(LTGRAY_BRUSH);
			}
		}
		return (INT_PTR)true;
	case WM_APPENDLOCKOUTTHREAD:
		bResult = ::DestroyWindow(hDlg);
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			bResult = ::DestroyWindow(hDlg);
			return (INT_PTR)TRUE;
		}

	case WM_DESTROY:
//		pnd->m_hwndLockoutRecordDisplayDlg = NULL;
		::PostQuitMessage(0);
		return (INT_PTR)TRUE;
	}
	return (INT_PTR)FALSE;
}

DWORD WINAPI CLockoutRecordDisplay::LockoutRecordDisplayThreadProc(LPVOID lParam)
{
	MSG message;
	BOOL bResult;

	CLockoutRecordDisplay* pnd = (CLockoutRecordDisplay*)lParam;
	pnd->m_hwndLockoutRecordDisplayDlg = NULL;
	
	if (!::IsWindow(pnd->m_hwndLockoutRecordDisplayDlg)) 
	{ 
		pnd->m_hwndLockoutRecordDisplayDlg = ::CreateDialogParam(pnd->m_hInst, MAKEINTRESOURCE(IDD_LOCKOUTRECORDDLG), pnd->m_hwndOwner, LockoutRecordDisplayDlgProc,(LPARAM)pnd); 
		bResult = ::ShowWindow(pnd->m_hwndLockoutRecordDisplayDlg, SW_SHOW); 
	} 

	while ((bResult = GetMessage(&message, NULL, 0, 0)) != 0) 
	{ 
		if (bResult == -1)
		{
        // Handle the error and possibly exit
			break;
		}
		else if (!IsWindow(pnd->m_hwndLockoutRecordDisplayDlg) || !IsDialogMessage(pnd->m_hwndLockoutRecordDisplayDlg,&message)) 
		{ 
			TranslateMessage(&message); 
			DispatchMessage(&message); 
		} 
	}
	pnd->m_hwndLockoutRecordDisplayDlg = NULL;
	return 0;
}

BOOL CLockoutRecordDisplay::Start(CSolaLockout* csl, int ndx)
{
	BOOL bSuccess = true;
	BOOL bResult;
	DWORD dwResult;
	int nResult;
	if ( LockoutRecordDisplayIsActive() )
	{
		return false;
	}
	if ( ::WaitForSingleObject(m_hLockoutRecordDisplayThread, 5000) == WAIT_TIMEOUT )
	{
		nResult = ::MessageBox(m_hwndOwner,_T("Error stopping Lockout Display thread"),m_szTitle,MB_OK);
		return false;
	}
	bResult = ::CloseHandle(m_hLockoutRecordDisplayThread);
	bResult = ::CloseHandle(m_hdupLockoutRecordDisplayThread);
	m_hLockoutRecordDisplayThread = NULL;
	m_hdupLockoutRecordDisplayThread = NULL;
	this->m_pcsl = csl;
	this->m_slndx = ndx;
	m_hLockoutRecordDisplayThread = ::CreateThread(NULL,0,LockoutRecordDisplayThreadProc,(LPVOID)this,CREATE_SUSPENDED,&m_dwLockoutRecordDisplayThreadID);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hLockoutRecordDisplayThread,::GetCurrentProcess(),&m_hdupLockoutRecordDisplayThread,DUPLICATE_SAME_ACCESS,true,0);
	dwResult = ::ResumeThread(m_hLockoutRecordDisplayThread);
	if ( m_hLockoutRecordDisplayThread == NULL )
	{
		nResult = ::MessageBox(m_hwndOwner,_T("Error starting Lockout Display thread"),m_szTitle,MB_OK);
	}
	return bSuccess;
}

BOOL CLockoutRecordDisplay::Stop()
{
	BOOL bResult;
	int nResult;

	if ( m_hLockoutRecordDisplayThread )
	{
		if ( m_hwndLockoutRecordDisplayDlg )
		{
			bResult = ::PostMessage(m_hwndLockoutRecordDisplayDlg,WM_APPENDLOCKOUTTHREAD,(WPARAM)0,(LPARAM)0);
			if ( ::WaitForSingleObject(m_hLockoutRecordDisplayThread, 5000) == WAIT_TIMEOUT )
			{
				nResult = ::MessageBox(m_hwndOwner,_T("Error stopping Lockout Display thread"),m_szTitle,MB_OK);
			}
		}
		bResult = ::CloseHandle(m_hLockoutRecordDisplayThread);
		bResult = ::CloseHandle(m_hdupLockoutRecordDisplayThread);
		m_hLockoutRecordDisplayThread = NULL;
		m_hdupLockoutRecordDisplayThread = NULL;
	}
	return true;
}

BOOL CLockoutRecordDisplay::LockoutRecordDisplayIsActive(void)
{
	return ::IsWindow(m_hwndLockoutRecordDisplayDlg);
}
#endif