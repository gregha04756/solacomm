#include "stdafx.h"
#include "resource.h"
#include "solacomm.h"
#include "SolaMBMap.h"

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];
extern "C++" BOOL bSolaConnected;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" float TempVal(BOOL units, short temp);
extern "C++" short ssTempVal (BOOL units, short temp);
extern "C++" float HystVal(BOOL units, short temp);
extern "C++" signed short SolaTempVal(BOOL units, short temp);
extern "C++" signed short SolaHystVal (BOOL units, short temp );

BOOL ChkDecimalInput(TCHAR* szInput,size_t& nLen)
{
	int i = 0;
	int nDotCnt = 0;
	size_t cc = 0;

	if ( nLen <= 0 )
	{
		return false;
	}

	if ( 0.0L != ::_wtof(szInput) )
	{
		return true;
	}

	if ( nLen > 0 )
	{
		if ( (1 == nLen) && (szInput[0] == L'-') )
		{
			return false;
		}
		if ( (1 == nLen) && (szInput[0] == L'.') )
		{
			return false;
		}
		if ( (1 == nLen) && (szInput[0] == L'+') )
		{
			return false;
		}
		for ( i = 0; i < nLen; i++ )
		{
			if ( (i > 0) && ((szInput[i] == L'-') || (szInput[i] == L'+')) )
			{
				return false;
			}
			nDotCnt = ((szInput[i] == L'.') ? ++nDotCnt : nDotCnt);
			if ( nDotCnt > 1 )
			{
				return false;
			}
			if ( (i > 0) && (szInput[i] != L'0') && (szInput[i] != L'.') )
			{
				return false;
			}
		}
	}
	return true;
}

float HC2F(float temp)
{
	return (9.0*temp)/5.0;
}

float TC2F(float temp)
{
	return ((9.0*temp)/5.0)+32.0;
}

float HF2C(float temp)
{
	return (5.0*temp)/9.0;
}

float TF2C(float temp)
{
	return (5.0*(temp-32.0))/9.0;
}

INT_PTR CALLBACK UINumericDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	int i;
	int nResult;
	int nTxtLen;
	float fT;
	static float fIntPart;
	static float fDecPart;
	static short nIntPart;
	static short nDecPart;
	short sEntered;
	HRESULT hRes;
	LRESULT lRes;
	PVOID pvTemp;
	TCHAR szTxt[MAX_LOADSTRING];
	TCHAR szTemp[MAX_LOADSTRING];
	static TCHAR szSave[32];
	static CSolaMBMap::LPNUMERICUIPARMS lpParms;
	static HWND hwndIntegerPart;
	static HWND hwndDecimalPart;
	static HWND hwndDecimalPoint;

	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (CSolaMBMap::LPNUMERICUIPARMS)lParam;
		if ( NULL != lpParms->szParmName )
		{
			nResult = ::GetWindowText(hDlg,szTxt,sizeof(szTxt)/sizeof(TCHAR));
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s %s"), szTxt, lpParms->szParmName);
			nResult = ::SetWindowText(hDlg,szTemp);
		}
		hwndIntegerPart = NULL;
		hwndDecimalPart = NULL;
		hwndDecimalPoint = NULL;
		nIntPart = 0;
		nDecPart = 0;
		pvTemp = ::SecureZeroMemory((PVOID)szSave,(SIZE_T)sizeof(szSave));
		if ( lpParms->st >= CSolaMBMap::Novalue && lpParms->st <= CSolaMBMap::Decimal1pl )
		{
			nIntPart = lpParms->ssValue;
			switch (lpParms->st)
			{
			case CSolaMBMap::TemperatureSetpoint:
				hwndDecimalPart = ::CreateWindowEx(	WS_EX_STATICEDGE,
												WC_EDIT,
												NULL,
												WS_CHILD|WS_VISIBLE|ES_RIGHT|WS_TABSTOP,
												100,
												25,
												100,
												25,
												hDlg,
												NULL,
												g_hInst,
												NULL);
				if ( hwndDecimalPart )
				{
					hRes = ::StringCchPrintf(szTxt,
						sizeof(szTxt)/sizeof(TCHAR),
						_T("%.2f"),
						::TempVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTxt,
							sizeof(szTxt)/sizeof(TCHAR),
							_T("%.1f"),
							::TempVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
					}
					bResult = ::SetWindowText(hwndDecimalPart,szTxt);
				}
				else
				{
					nResult = ::MessageBox(hDlg,_T("Internal program error"),szTitle,MB_OK);
				}
				return (INT_PTR)TRUE;
			case CSolaMBMap::Hysteresis:
				hwndDecimalPart = ::CreateWindowEx(	WS_EX_STATICEDGE,
												WC_EDIT,
												NULL,
												WS_CHILD|WS_VISIBLE|ES_RIGHT|WS_TABSTOP,
												100,
												25,
												100,
												25,
												hDlg,
												NULL,
												g_hInst,
												NULL);
				if ( hwndDecimalPart )
				{
					hRes = ::StringCchPrintf(szTxt,
						sizeof(szTxt)/sizeof(TCHAR),
						_T("%.2f"),
						::HystVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						hRes = ::StringCchPrintf(szTxt,
							sizeof(szTxt)/sizeof(TCHAR),
							_T("%.1f"),
							::HystVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
					}
					bResult = ::SetWindowText(hwndDecimalPart,szTxt);
				}
				else
				{
					nResult = ::MessageBox(hDlg,_T("Internal program error"),szTitle,MB_OK);
				}
				return (INT_PTR)TRUE;
			case CSolaMBMap::Decimal1pl:
				nIntPart /= 10;
				nDecPart = lpParms->ssValue - (nIntPart*10);
				hwndDecimalPart = ::CreateWindowEx(	WS_EX_CLIENTEDGE,
											WC_EDIT,
											NULL,
											WS_CHILD|WS_VISIBLE|ES_LEFT|ES_NUMBER|WS_TABSTOP,
											225,
											25,
											50,
											25,
											hDlg,
											NULL,
											g_hInst,
											NULL);
				if ( hwndDecimalPart )
				{
					lRes = ::SendMessage(hwndDecimalPart,EM_SETLIMITTEXT,(WPARAM)1,(LPARAM)0);
				}
				break;
			case CSolaMBMap::Decimal2pl:
				nIntPart /= 100;
				nDecPart = lpParms->ssValue - (nIntPart*100);
				hwndDecimalPart = ::CreateWindowEx(	WS_EX_CLIENTEDGE,
											WC_EDIT,
											NULL,
											WS_CHILD|WS_VISIBLE|ES_LEFT|ES_NUMBER|WS_TABSTOP,
											225,
											25,
											50,
											25,
											hDlg,
											NULL,
											g_hInst,
											NULL);
				if ( hwndDecimalPart )
				{
					lRes = ::SendMessage(hwndDecimalPart,EM_SETLIMITTEXT,(WPARAM)2,(LPARAM)0);
				}
				break;
			default:
				break;
			}
			if ( hwndDecimalPart )
			{
				hwndDecimalPoint = ::CreateWindow(WC_STATIC,
											NULL,
											WS_CHILD|WS_VISIBLE|SS_RIGHT|WS_TABSTOP,
											205,
											30,
											15,
											25,
											hDlg,
											NULL,
											g_hInst,
											NULL);
				if ( hwndDecimalPoint )
				{
					hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T(".")); 
					bResult = ::SetWindowText(hwndDecimalPoint,szTxt);
				}
			}
			hwndIntegerPart = ::CreateWindowEx(	WS_EX_STATICEDGE,
											WC_EDIT,
											NULL,
											WS_CHILD|WS_VISIBLE|ES_RIGHT|ES_NUMBER|WS_TABSTOP,
											100,
											25,
											100,
											25,
											hDlg,
											NULL,
											g_hInst,
											NULL);
			if ( hwndIntegerPart )
			{
				lRes = ::SendMessage(hwndIntegerPart,EM_SETLIMITTEXT,(WPARAM)5,(LPARAM)0);
				hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"),nIntPart);
				hRes = ::StringCchPrintf(szSave,sizeof(szSave)/sizeof(TCHAR),_T("%d"),nIntPart);
				bResult = ::SetWindowText(hwndIntegerPart,szTxt);
			}
			else
			{
				nResult = ::MessageBox(hDlg,_T("Internal program error"),szTitle,MB_OK);
			}
			if ( hwndDecimalPart )
			{
				hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"),nDecPart);
				bResult = ::SetWindowText(hwndDecimalPart,szTxt);
			}
		}
		else
		{
			nResult = ::MessageBox(hDlg,_T("Internal program error, invalid data type"),szTitle,MB_OK);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			sEntered = 0;
			if ( hwndIntegerPart )
			{
				pvTemp = ::SecureZeroMemory((PVOID)szSave,(SIZE_T)sizeof(szSave)/sizeof(TCHAR));
				nResult = ::GetWindowText(hwndIntegerPart,szSave,sizeof(szSave)/sizeof(TCHAR));
				lpParms->ssValue = (short)::_wtoi(szSave);
			}
			if ( hwndDecimalPart )
			{
				switch (lpParms->st)
				{
				case CSolaMBMap::TemperatureSetpoint:
					pvTemp = ::SecureZeroMemory((PVOID)szSave,(SIZE_T)sizeof(szSave)/sizeof(TCHAR));
					nResult = ::GetWindowText(hwndDecimalPart,szSave,sizeof(szSave)/sizeof(TCHAR));
					hRes = ::StringCchLength(szSave,sizeof(szSave)/sizeof(TCHAR),(size_t*)&nTxtLen);
					if ( !ChkDecimalInput(szSave,(size_t&)nTxtLen) )
					{
						nResult = ::MessageBox(hDlg,_T("Invalid input, enter a number please"),szTitle,MB_OK);
						hRes = ::StringCchPrintf(szTxt,
							sizeof(szTxt)/sizeof(TCHAR),
							_T("%.2f"),
							::TempVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
						bResult = ::SetWindowText(hwndDecimalPart,szTxt);
						return (INT_PTR)TRUE;
					}
					if ( FAHRENHEITUNITS == pcSystemConfiguration->GetValue(0) )
					{
						fT = ::_wtof(szSave);
						fT = TF2C(fT);
						lpParms->ssValue = (short)(10.0*TF2C(::_wtof(szSave)));
					}
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						lpParms->ssValue = (short)(10.0*::_wtof(szSave));
					}
					break;
				case CSolaMBMap::Hysteresis:
					pvTemp = ::SecureZeroMemory((PVOID)szSave,(SIZE_T)sizeof(szSave)/sizeof(TCHAR));
					nResult = ::GetWindowText(hwndDecimalPart,szSave,sizeof(szSave)/sizeof(TCHAR));
					hRes = ::StringCchLength(szSave,sizeof(szSave)/sizeof(TCHAR),(size_t*)&nTxtLen);
					if ( !ChkDecimalInput(szSave,(size_t&)nTxtLen) )
					{
						nResult = ::MessageBox(hDlg,_T("Invalid input, enter a number please"),szTitle,MB_OK);
						hRes = ::StringCchPrintf(szTxt,
							sizeof(szTxt)/sizeof(TCHAR),
							_T("%.2f"),
							::HystVal(pcSystemConfiguration->GetValue(0),lpParms->ssValue));
						bResult = ::SetWindowText(hwndDecimalPart,szTxt);
						return (INT_PTR)TRUE;
					}
					if ( FAHRENHEITUNITS == pcSystemConfiguration->GetValue(0) )
					{
						fT = ::_wtof(szSave);
						fT = HF2C(fT);
						lpParms->ssValue = (short)(10.0*HF2C(::_wtof(szSave)));
					}
					if ( CELSIUSUNITS == pcSystemConfiguration->GetValue(0) )
					{
						lpParms->ssValue = (short)(10.0*::_wtof(szSave));
					}
					break;
				case CSolaMBMap::Decimal1pl:
					sEntered = (short)::GetDlgItemInt(hDlg,::GetDlgCtrlID(hwndDecimalPart),&bResult,true);
					lpParms->ssValue = (lpParms->ssValue*10) + sEntered;
					break;
				case CSolaMBMap::Decimal2pl:
					sEntered = (short)::GetDlgItemInt(hDlg,::GetDlgCtrlID(hwndDecimalPart),&bResult,true);
					lpParms->ssValue = (lpParms->ssValue*100) + sEntered;
					break;
				}
			}
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDCANCEL )
		{
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if ( ((HWND)lParam == hwndIntegerPart) && (EN_CHANGE == HIWORD(wParam)) )
		{
			nResult = ::GetWindowText(hwndIntegerPart,szTxt,sizeof(szTxt)/sizeof(TCHAR));
			hRes = ::StringCchLength(szTxt,sizeof(szTxt)/sizeof(TCHAR),(size_t*)&nTxtLen);
			if ( nTxtLen > 0 )
			{
				if ( (szTxt[0] == L'-') || ((szTxt[0] >= L'0') && (szTxt[0] <= L'9')) )
				{
					for ( i = 1; (i < nTxtLen) && ((szTxt[i] >= L'0') && (szTxt[i] <= L'9')); i++ )
					{
					}
					if ( i >= nTxtLen )
					{
						hRes = ::StringCchPrintf(szSave,sizeof(szSave)/sizeof(TCHAR),_T("%s"),szTxt);
					}
					if ( i < nTxtLen )
					{
						nResult = ::MessageBox(hDlg,_T("- or 0 to 9 only please"),szTitle,MB_OK);
						bResult = ::SetWindowText(hwndIntegerPart,szSave);
					}
				}
				else
				{
					nResult = ::MessageBox(hDlg,_T("- or 0 to 9 only please"),szTitle,MB_OK);
					bResult = ::SetWindowText(hwndIntegerPart,szSave);
				}
			}
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}

INT_PTR CALLBACK UIPercentDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	int nResult;
	UINT uiResult;
	LRESULT lRes;
	HRESULT hRes;
	TCHAR szTxt[MAX_LOADSTRING];
	TCHAR szTemp[MAX_LOADSTRING];
	static CSolaMBMap::LPNUMERICUIPARMS lpParms;
	static short sSaved;
	short sEntered;

	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (CSolaMBMap::LPNUMERICUIPARMS)lParam;
		sSaved = lpParms->ssValue;
		if ( NULL != lpParms->szParmName )
		{
			nResult = ::GetWindowText(hDlg,szTxt,sizeof(szTxt)/sizeof(TCHAR));
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("%s %s"), szTxt, lpParms->szParmName);
			nResult = ::SetWindowText(hDlg,szTemp);
		}
		hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"), lpParms->ssValue/10);
		bResult = ::SetDlgItemText(hDlg,IDC_CONFIGUIPCTEDIT,szTxt);
		hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"), lpParms->ssValue-((lpParms->ssValue/10)*10));
		bResult = ::SetDlgItemText(hDlg,IDC_CONFIGUIPCTEDITDEC,szTxt);
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_CONFIGUIPCTEDIT),EM_SETLIMITTEXT,(WPARAM)3,(LPARAM)0);
		lRes = ::SendMessage(::GetDlgItem(hDlg,IDC_CONFIGUIPCTEDITDEC),EM_SETLIMITTEXT,(WPARAM)1,(LPARAM)0);
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			uiResult = ::GetDlgItemText(hDlg,IDC_CONFIGUIPCTEDIT,szTxt,sizeof(szTxt));
			sEntered = (short) 10*::_wtoi(szTxt);
			uiResult = ::GetDlgItemText(hDlg,IDC_CONFIGUIPCTEDITDEC,szTxt,sizeof(szTxt));
			sEntered += (short) ::_wtoi(szTxt);
			if ( sEntered > 1000 )
			{
				nResult = ::MessageBox(hDlg,_T("Value too big, must be <= 100"),szTitle,MB_OK);
				hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"), sSaved/10);	
				bResult = ::SetDlgItemText(hDlg,IDC_CONFIGUIPCTEDIT,szTxt);
				hRes = ::StringCchPrintf(szTxt,sizeof(szTxt)/sizeof(TCHAR),_T("%d"), sSaved-((sSaved/10)*10));
				bResult = ::SetDlgItemText(hDlg,IDC_CONFIGUIPCTEDITDEC,szTxt);
				return (INT_PTR)TRUE;
			}
			lpParms->ssValue = sEntered;
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

INT_PTR CALLBACK UIMultiDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	LRESULT lRes;
	static LPMULTIUIPARMS lpParms;

	switch (message)
	{
	case WM_INITDIALOG:
		{
			lpParms = (LPMULTIUIPARMS)lParam;
			for ( int i = 0; i < lpParms->nMultiListSize; i++ )
			{
				lRes = ::SendDlgItemMessage(hDlg,IDC_UIMULTICOMBO,CB_ADDSTRING,(WPARAM)0,(LPARAM)lpParms->lpMultiList[i].szString);
			}
			lRes = ::SendDlgItemMessage(hDlg,IDC_UIMULTICOMBO,CB_SETCURSEL,(WPARAM)lpParms->nCurSel,(LPARAM)0);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			lpParms->nCurSel = (int)::SendDlgItemMessage(hDlg,IDC_UIMULTICOMBO,CB_GETCURSEL,(WPARAM)0,(LPARAM)0);
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

INT_PTR CALLBACK UITimeDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	BOOL bResult;
	LRESULT lRes;
	UINT uiResult;
	HRESULT hRes;
	TCHAR szTemp[MAX_LOADSTRING];
	static CSolaMBMap::LPNUMERICUIPARMS lpParms;
	static HWND hCHHystSTHHSpin;
	static HWND hCHHystSTMMSpin;
	static HWND hCHHystSTSSSpin;
	int hh;
	int mm;
	int ss;
	int nResult;
	const TCHAR szHHSpinWndName[] = _T("HystSTHHSpin");
	const TCHAR szMMSpinWndName[] = _T("HystSTMMSpin");
	const TCHAR szSSSpinWndName[] = _T("HystSTSSSpin");

	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (CSolaMBMap::LPNUMERICUIPARMS)lParam;
		hCHHystSTHHSpin = ::CreateWindowEx(	WS_EX_RIGHTSCROLLBAR,
											UPDOWN_CLASS,
											szHHSpinWndName,
											WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
											10,
											25,
											12,
											12,
											hDlg,
											NULL,
											g_hInst,
											NULL);
		lRes = ::SendMessage(hCHHystSTHHSpin, UDM_SETBUDDY, (WPARAM) (HWND) ::GetDlgItem(hDlg,IDC_CHHYSTSTHOUREDIT), (LPARAM)0);
		lRes = ::SendMessage(hCHHystSTHHSpin, UDM_SETRANGE32, (WPARAM) (INT) 0, (LPARAM) (INT) 59);
		hCHHystSTMMSpin = ::CreateWindowEx(	WS_EX_RIGHTSCROLLBAR,
											UPDOWN_CLASS,
											szMMSpinWndName,
											WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
											70,
											50,
											12,
											12,
											hDlg,
											NULL,
											g_hInst,
											NULL);
		lRes = ::SendMessage(hCHHystSTMMSpin, UDM_SETBUDDY, (WPARAM) (HWND) ::GetDlgItem(hDlg,IDC_CHHYSTSTMINEDIT), (LPARAM)0);
		lRes = ::SendMessage(hCHHystSTMMSpin, UDM_SETRANGE32, (WPARAM) (INT) 0, (LPARAM) (INT) 59);
		hCHHystSTSSSpin = ::CreateWindowEx(	WS_EX_RIGHTSCROLLBAR,
											UPDOWN_CLASS,
											szSSSpinWndName,
											WS_CHILD | WS_VISIBLE | UDS_ALIGNRIGHT | UDS_SETBUDDYINT,
											130,
											50,
											12,
											12,
											hDlg,
											NULL,
											g_hInst,
											NULL);
		lRes = ::SendMessage(hCHHystSTSSSpin, UDM_SETBUDDY, (WPARAM) (HWND) ::GetDlgItem(hDlg,IDC_CHHYSTSTSECEDIT), (LPARAM)0);
		lRes = ::SendMessage(hCHHystSTSSSpin, UDM_SETRANGE32, (WPARAM) (INT) 0, (LPARAM) (INT) 59);
		
		switch ( lpParms->st )
		{
		case CSolaMBMap::Seconds:
			hh = lpParms->ssValue/3600;
			mm = (lpParms->ssValue-(hh*3600))/60;
			ss = lpParms->ssValue - (hh*3600) - (mm*60);
			break;
		case CSolaMBMap::Minutes:
			hh = lpParms->ssValue/60;
			mm = lpParms->ssValue-(hh*60);
			ss = 0;
			break;
		case CSolaMBMap::Hours:
			hh = lpParms->ssValue;
			mm = 0;
			ss = 0;
			break;
		case CSolaMBMap::Timevalue:
			hh = lpParms->ssValue/3600;
			mm = (lpParms->ssValue-(hh*3600))/60;
			ss = lpParms->ssValue - (hh*3600) - (mm*60);
			break;
		default:
			nResult = ::MessageBox(hDlg,_T("Internal program error"),szTitle,MB_OK);
			break;
		}

		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d"), hh);
		bResult = ::SetDlgItemText(hDlg, IDC_CHHYSTSTHOUREDIT, szTemp);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_CHHYSTSTHOUREDIT), bSolaConnected*pcTrendStatus->GetValue((int)13));
		
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d"), mm);
		bResult = ::SetDlgItemText(hDlg, IDC_CHHYSTSTMINEDIT, szTemp);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_CHHYSTSTMINEDIT), bSolaConnected*pcTrendStatus->GetValue((int)13));
		
		hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d"), ss);
		bResult = ::SetDlgItemText(hDlg, IDC_CHHYSTSTSECEDIT, szTemp);
		bResult = ::EnableWindow(::GetDlgItem(hDlg, IDC_CHHYSTSTSECEDIT), bSolaConnected*pcTrendStatus->GetValue((int)13));
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
//			uiResult = ::GetDlgItemText(hDlg,IDC_SYSTEMIDUIEDIT,szTxt,sizeof(szTxt));
//			hRes = ::StringCchPrintf(lpParms->szTxt,lpParms->cchLen,_T("%s"),szTxt);
			uiResult = ::GetDlgItemText(hDlg,IDC_CHHYSTSTHOUREDIT,szTemp,sizeof(szTemp));
			hh = ::_wtoi(szTemp);
			uiResult = ::GetDlgItemText(hDlg,IDC_CHHYSTSTMINEDIT,szTemp,sizeof(szTemp));
			mm = ::_wtoi(szTemp);
			uiResult = ::GetDlgItemText(hDlg,IDC_CHHYSTSTSECEDIT,szTemp,sizeof(szTemp));
			ss = ::_wtoi(szTemp);
			switch (lpParms->st)
			{
			case CSolaMBMap::Seconds:
				lpParms->ssValue = (short) ((hh*3600) + (mm*60) + ss);
				break;
			case CSolaMBMap::Minutes:
				lpParms->ssValue = (short) ((hh*60) + mm);
				break;
			case CSolaMBMap::Hours:
				lpParms->ssValue = (short) hh;
				break;
			case CSolaMBMap::Timevalue:
				lpParms->ssValue = (short) ((hh*3600) + (mm*60) + ss);
				break;
			default:
				nResult = ::MessageBox(hDlg,_T("Internal program error"),szTitle,MB_OK);
				break;
			}
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

INT_PTR CALLBACK UIBitMaskDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	BOOL bResult;
	HRESULT hRes;
	RECT crectDlg;
	int i;
	DWORD dwResult;
	static HWND hwndLblMaskValue;
	static HWND hwndTxtMaskValue;
	static HWND hwndScrollLeftRightBar;
	static HWND hwndChecks[16];
	static HWND hwndBtnLbl[16];
	int x;
	int y;
	static TCHAR szBtnLbl[60];
	static unsigned short usMask;
	unsigned short usBit;
	static LPBITMASKUIPARMS lpParms;
	TCHAR szMaskValue[7];
	LRESULT lrBtnState;


	switch (message)
	{
	case WM_INITDIALOG:
		lpParms = (LPBITMASKUIPARMS)lParam;
		usMask = lpParms->usValue;
		bResult = ::GetClientRect(hDlg,&crectDlg);
		x = crectDlg.left+10;
		y = crectDlg.top+10;
		usBit = 1;
		for ( i = 0; i < sizeof(hwndChecks)/sizeof(HWND); i++ )
		{
			if ( i <= 7 )
			{
				hwndChecks[i] = ::CreateWindowEx(	WS_EX_LEFT,
													WC_BUTTON,
													NULL,
													WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
													x,
													y,
													15,
													20,
													hDlg,
													NULL,
													g_hInst,
													NULL);
				hwndBtnLbl[i] = ::CreateWindowEx(	WS_EX_LEFT,
													WC_STATIC,
													NULL,
													WS_CHILD | WS_VISIBLE| SS_LEFTNOWORDWRAP,
													x+20,
													y,
													175,
													20,
													hDlg,
													NULL,
													g_hInst,
													NULL);
				y += 25;
			}
			else
			{
				y = ( i == 8 ) ? crectDlg.top+10 : y + 25;
				hwndChecks[i] = ::CreateWindowEx(	WS_EX_LEFT,
													WC_BUTTON,
													NULL,
													WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
													x+250,
													y,
													15,
													20,
													hDlg,
													NULL,
													g_hInst,
													NULL);
				hwndBtnLbl[i] = ::CreateWindowEx(	WS_EX_LEFT,
													WC_STATIC,
													NULL,
													WS_CHILD | WS_VISIBLE | SS_LEFTNOWORDWRAP,
													x+270,
													y,
													175,
													20,
													hDlg,
													NULL,
													g_hInst,
													NULL);
			}
			if ( (dwResult = ::GetLastError() != NOERROR ) )
			{
				ReportError(dwResult);
			}
			if ( hwndChecks[i] )
			{
				if ( lpParms->usValue & usBit )
				{
					bResult = ::PostMessage(hwndChecks[i],BM_SETCHECK,(WPARAM)BST_CHECKED,(LPARAM)0);
				}
				if ( !(lpParms->usValue & usBit) )
				{
					bResult = ::PostMessage(hwndChecks[i],BM_SETCHECK,(WPARAM)BST_UNCHECKED,(LPARAM)0);
				}
			}
			if ( hwndBtnLbl[i] && (lpParms->lpMulti[i].szString != NULL) && (i < lpParms->nMultiSize) )
			{
				hRes = ::StringCchPrintf(szBtnLbl,sizeof(szBtnLbl)/sizeof(TCHAR),_T("%s"),lpParms->lpMulti[i].szString);
				bResult = ::SetWindowText(hwndBtnLbl[i],szBtnLbl);
			}
			usBit <<= 1;
		}
		hRes = ::StringCchPrintf(szMaskValue,sizeof(szMaskValue)/sizeof(TCHAR),_T("0x%04x"),usMask);
		bResult = ::SetWindowText(::GetDlgItem(hDlg,IDC_TXTUIBMMASKVALUE),szMaskValue);
		if ( lpParms->szParmName != NULL )
		{
			bResult = ::SetWindowText(hDlg,lpParms->szParmName);
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		usBit = 1;
		for ( i = 0; i < sizeof(hwndChecks)/sizeof(HWND); i++ )
		{
			if ((HWND)lParam == hwndChecks[i] )
			{
				lrBtnState = ::SendMessage(hwndChecks[i],BM_GETCHECK,(WPARAM)0,(LPARAM)0);
				if ( lrBtnState == BST_CHECKED )
				{
					usMask |= usBit;
				}
				if ( lrBtnState == BST_UNCHECKED )
				{
					usMask &= (~usBit);
				}
				hRes = ::StringCchPrintf(szMaskValue,sizeof(szMaskValue)/sizeof(TCHAR),_T("0x%04x"),usMask);
				bResult = ::SetWindowText(::GetDlgItem(hDlg,IDC_TXTUIBMMASKVALUE),szMaskValue);
			}
			usBit <<= 1;
		}
		if (LOWORD(wParam) == IDOK )
		{
			lpParms->usValue = usMask;
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDCANCEL )
		{
			bResult = EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

DWORD Init_TCP_GW_Combo(HWND hwnd_pd)
{
#if _DEBUG
	TCHAR sz_temp[100];
	HRESULT hr_rc;
	PVOID p_v;
#endif
	TCHAR const gw_names[3][12] = {{_T("Protonode")},{_T("Lantronix")},{_T("Other")}};
	int i_i;
	LRESULT lr_rc;
	DWORD dw_rc;
	dw_rc = ERROR_SUCCESS;
	lr_rc = ::SendMessage(GetDlgItem(hwnd_pd,IDC_TCPGWCOMBO),CB_SETCURSEL,(WPARAM)-1,(LPARAM)0);
	for (i_i = 0; (ERROR_SUCCESS == dw_rc) && (i_i < 3); i_i++)
	{
#if _DEBUG
		p_v = SecureZeroMemory((PVOID)sz_temp,sizeof(sz_temp));
		hr_rc = StringCchPrintf(sz_temp,sizeof(sz_temp)/sizeof(TCHAR),_T("%s"),gw_names[i_i]);
#endif
		lr_rc = SendMessage(GetDlgItem(hwnd_pd,IDC_TCPGWCOMBO),CB_ADDSTRING,(WPARAM)0,(LPARAM)gw_names[i_i]);
		dw_rc = GetLastError();
		if (ERROR_SUCCESS != dw_rc)
		{
			ReportError(dw_rc);
		}
	}
	lr_rc = ::SendMessage(GetDlgItem(hwnd_pd,IDC_TCPGWCOMBO),CB_SHOWDROPDOWN,(WPARAM)false,(LPARAM)0);
	lr_rc = ::SendMessage(GetDlgItem(hwnd_pd,IDC_TCPGWCOMBO),CB_SETCURSEL,(WPARAM)0,(LPARAM)0);
	return dw_rc;
}