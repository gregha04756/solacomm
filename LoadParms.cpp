#include "stdafx.h"
#include "resource.h"
#include "solacomm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"

using namespace std;

#define NUMFILELINES 3

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];
extern "C++" BOOL bSolaConnected;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaPage* pcSaveRestorePage;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" float TempVal(BOOL units,short temp);
extern "C++" float HystVal(BOOL units,short temp);

extern "C++" const int i_SB_nWidth_factor;
extern "C++" const int i_SB_nMax_value;

INT_PTR CALLBACK LoadParmsDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	BOOL bResult;
	static bool bSuccess;
	char cch;
	char* pch;
	static char* pchinline;
	static int fldcnt;
	int i;
	int nResult;
	static int linecnt;
	static short* pvals;
	static unsigned short* pparms;
	static std::string ifline;
	DWORD dwResult;
	static LPLOADFILEPARMS lpLFParms;
	BOOL bSelectable;
	HRESULT hRes;
	LRESULT lResult;
	int hh;
	int mm;
	int ss;
	TCHAR szTemp[MAX_LOADSTRING];
	unsigned short usR;
	short sV;
	TCHAR* szS;
	float flDecimal1;
	unsigned short usMask;
	static CSolaPage* lpPage;
	int nLeftScrollAmt;
	int nRightScrollAmt;
	static HWND hwndScrollLeftRightBar;
	RECT crectDlg;
	RECT crectUpdate;
	static int nLastPos;
	static SCROLLINFO si = { 0 };

	switch (message)
	{
	case WM_INITDIALOG:
		lpLFParms = (LPLOADFILEPARMS)lParam;
		bSuccess = true;
		pparms = NULL;
		pch = NULL;
		pchinline = NULL;
		pparms = NULL;
		pvals = NULL;
		fldcnt = 0;
		linecnt = 0;
		lpPage = pcSaveRestorePage;


		lpLFParms->lpifile->exceptions(ifstream::failbit|ifstream::badbit);
		if ( lpLFParms->lpifile->is_open() )
		{
			while ( !lpLFParms->lpifile->eof() )
			{
				cch = lpLFParms->lpifile->peek();
				if ( lpLFParms->lpifile->rdstate() & ifstream::eofbit )
				{
					break;
				}
				try
				{
					getline(*(lpLFParms->lpifile),ifline);
				}
				catch (ifstream::failure fof)
				{
					ReportError(fof.what());
					bSuccess = false;
					break;
				}
				linecnt++;
			}
		}
		if ( !(bSuccess = (NUMFILELINES == linecnt)) )
		{
			i = ::MessageBox(hDlg,_T("File corrupted"),szTitle,MB_OK);
		}
		if ( lpLFParms->lpifile->eof() )
		{
			lpLFParms->lpifile->clear();
		}
		if ( bSuccess )
		{
			try
			{
				lpLFParms->lpifile->seekg(ifstream::beg);
			}
			catch (ifstream::failure fof)
			{
				ReportError(fof.what());
				bSuccess = false;
			}
		}
		if ( bSuccess )
		{
			try
			{
				getline(*(lpLFParms->lpifile),ifline);
			}
			catch (ifstream::failure fof)
			{
				ReportError(fof.what());
				bSuccess = false;
			}
		}
		if ( bSuccess )
		{
			fldcnt = 0;
			try
			{
				pchinline = (char*)new char[(ifline.size()+1)*sizeof(char)];
			}
			catch (std::bad_alloc ba)
			{
				bSuccess = false;
			}
		}
		if ( bSuccess )
		{
			try
			{
				getline(*(lpLFParms->lpifile),ifline);
			}
			catch (ifstream::failure fof)
			{
				ReportError(::GetLastError());
				bSuccess = false;
			}
		}
		if ( bSuccess )
		{
			i = ::sprintf(pchinline,"%s",ifline.c_str());
			pch = strtok(pchinline,",");
			while (pch != NULL)
			{
				fldcnt++;
				pch = strtok(NULL, ",");
			}
			bSuccess = (fldcnt == pcSaveRestorePage->GetSize());
			if ( !bSuccess )
			{
				i = ::MessageBox(hDlg,_T("File corrupted"),szTitle,MB_OK);
			}
		}
		if ( bSuccess )
		{
			try
			{
				pparms = (unsigned short*)new int[fldcnt];
				pvals = (short*)new int[fldcnt];
			}
			catch (std::bad_alloc ba)
			{
				bSuccess = false;
			}
		}
		if ( bSuccess && (NULL != pparms) )
		{
			i = ::sprintf(pchinline,"%s",ifline.c_str());
			i = 0;
			pch = strtok(pchinline,",");
			while ( (i < fldcnt) && (pch != NULL) )
			{
				pparms[i++] = ::atoi(pch);
				pch = strtok(NULL, ",");
			}
		}
		if ( bSuccess )
		{
			try
			{
				getline(*(lpLFParms->lpifile),ifline);
			}
			catch (ifstream::failure fof)
			{
				ReportError(::GetLastError());
				bSuccess = false;
			}
		}
		if ( bSuccess )
		{
			fldcnt = 0;
			i = ::sprintf(pchinline,"%s",ifline.c_str());
			pch = strtok(pchinline,",");
			while (pch != NULL)
			{
				fldcnt++;
				pch = strtok(NULL, ",");
			}
			bSuccess = (fldcnt == pcSaveRestorePage->GetSize());
			if ( !bSuccess )
			{
				i = ::MessageBox(hDlg,_T("File corrupted"),szTitle,MB_OK);
			}
		}
		if ( bSuccess && (NULL != pvals) )
		{
			i = ::sprintf(pchinline,"%s",ifline.c_str());
			i = 0;
			pch = strtok(pchinline,",");
			while ( (i < fldcnt) && (pch != NULL) )
			{
				pvals[i++] = ::atoi(pch);
				pch = strtok(NULL, ",");
			}
		}
		if ( NULL != pchinline )
		{
			delete[] pchinline;
		}

		for ( i = 0; bSuccess && i < pcSaveRestorePage->GetSize(); i++ )
		{
			bSuccess = (pparms[i] == lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)));
			if ( !bSuccess )
			{
				i = ::MessageBox(hDlg,_T("File corrupted"),szTitle,MB_OK);
			}
		}
		if ( bSuccess )
		{
			bResult = ::SetDlgItemText(hDlg,IDC_TXTFILECONTENTS,_T("File OK please check values before restoring"));
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDOK),true);
		}
		if ( !bSuccess )
		{
			bResult = ::SetDlgItemText(hDlg,IDC_TXTFILECONTENTS,_T("File corrupted, cannot load"));
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDOK),false);
		}
		for ( i = 0; bSuccess && i < lpPage->GetSize(); i++ )
		{
			bResult = ::SetDlgItemText(hDlg,
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
					::LoadString(g_hInst,IDS_UNCONFIGURED,szTemp,sizeof(szTemp)/sizeof(TCHAR));
				}
				else
				{
					hRes = ::StringCchPrintf(szTemp,
						sizeof(szTemp)/sizeof(TCHAR),
						_T("%02.0f"),
						TempVal(pvals[0],pvals[i]));
				}
				lResult = ::SetDlgItemText(hDlg,
					TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
					szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg,TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))),
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
						_T("%02.0f"),
						TempVal(pvals[0],pvals[i]));
				}
				lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
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
						_T("%02.0f"),
						HystVal(pvals[0],pvals[i]));
				}
				lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Decimal1pl:
				usR = pvals[i];
				flDecimal1 =  (float)pvals[i];
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%-5.1f"),flDecimal1/10.0);
				lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Numericvalue:
				hRes = ::StringCchPrintf(szTemp,
					sizeof(szTemp)/sizeof(TCHAR),
					_T("%d"),
					pvals[i]);
				lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Percentvalue:
				hRes = ::StringCchPrintf(szTemp,
					sizeof(szTemp)/sizeof(TCHAR),
					_T("%2.1f"),
					((float)pvals[i])/10.0);
				lResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Bitmask:
				usMask = (unsigned short)pvals[i];
				hRes = ::StringCchPrintf(szTemp,
					sizeof(szTemp)/sizeof(TCHAR),
					_T("0x%04x"),
					usMask);
				lResult = ::SetDlgItemText(hDlg,
					TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
					szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Multivalue:
				if ( lpPage->ItemMap(i)->GetLPMulti(lpPage->ItemIndex(i)) != NULL )
				{
					usR = lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i));
//					sV = lpPage->ItemMap(i)->GetValue(usR);
					sV = pvals[i];
//					szS = lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)usR);
					szS = lpPage->ItemMap(i)->GetMultiValueItem((unsigned short)usR,(unsigned short)pvals[i]);
					if ( usR == 0x0273 )
					{
						sV = lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i));
						szS = lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)lpPage->ItemMap(i)->GetValue(lpPage->ItemIndex(i)));
					}
					lResult = ::SetDlgItemText(hDlg,
						TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))),
						lpPage->ItemMap(i)->GetMultiValueItem(lpPage->ItemIndex(i),(unsigned short)pvals[i]));
					bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				}
				break;
			case CSolaMBMap::Timevalue:
				hh = pvals[i]/3600;
				mm = (pvals[i]-(hh*3600))/60;
				ss = pvals[i]-(hh*3600)-(mm*60);
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			case CSolaMBMap::Minutes:
				hh = pvals[i]/60;
				mm = pvals[i]-(hh*60);
				ss = 0;
				hRes = ::StringCchPrintf(szTemp, sizeof(szTemp)/sizeof(TCHAR), _T("%02d:%02d:%02d"), hh, mm, ss);
				bResult = ::SetDlgItemText(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i))), szTemp);
				bResult = ::EnableWindow(::GetDlgItem(hDlg, TXTIDBASE+(lpPage->ItemMap(i)->GetStartRegAddr(lpPage->ItemIndex(i)))), bSelectable);
				break;
			default:
				break;
			}
		}



		bResult = ::GetClientRect(hDlg,&crectDlg);
		hwndScrollLeftRightBar = ::CreateWindowEx(	WS_EX_LEFT | SBS_HORZ,
												WC_SCROLLBAR,
												NULL,
												WS_CHILD | WS_VISIBLE,
												crectDlg.left,
												0,
												i_SB_nWidth_factor*(crectDlg.right-crectDlg.left), /* Match multiplier to same setting in SaveRestore scrollbar */
												15,
												hDlg,
												NULL,
												g_hInst,
												NULL);
		if ( NOERROR != (dwResult = ::GetLastError()) )
		{
			ReportError(dwResult);
		}
		if ( hwndScrollLeftRightBar )
		{
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			bResult = ::GetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si);
			si.fMask = SIF_ALL;
			si.nMax = i_SB_nMax_value;  /* Match value to same setting in SaveRestore scrollbar */
			si.nMin = 0;
			si.nPage = 1;
			si.nPos = 0;
			nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
		}
		nLastPos = 0;
		return (INT_PTR)TRUE;
	case WM_CTLCOLORSTATIC:
		{
			int nResult = ::SetBkMode((HDC)wParam,TRANSPARENT);
			return (LRESULT)CreateSolidBrush(0xFFFFFF);
		}
	case WM_HSCROLL:
		if ( (HWND)lParam == hwndScrollLeftRightBar )
		{
			switch ( LOWORD(wParam) )
			{
			case SB_THUMBPOSITION:
				if ( HIWORD(wParam) > nLastPos )
				{
					bResult = ::GetWindowRect(hDlg,&crectDlg);
//					nRightScrollAmt = crectDlg.left - crectDlg.right;
					nRightScrollAmt = ((HIWORD(wParam)-nLastPos)*(crectDlg.left - crectDlg.right))/3;
					nResult = ::ScrollWindowEx(hDlg,nRightScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
					si.fMask = SIF_POS;
					si.nPos = HIWORD(wParam);
					if ( si.nPos > si.nMax )
					{
						si.nPos = si.nMax;
					}
					nLastPos = si.nPos;
					nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
					break;
				}
				if ( HIWORD(wParam) < nLastPos )
				{
					bResult = ::GetWindowRect(hDlg,&crectDlg);
//					nLeftScrollAmt = crectDlg.right - crectDlg.left;
					nLeftScrollAmt = ((nLastPos-HIWORD(wParam))*(crectDlg.right - crectDlg.left))/3;
					nResult = ::ScrollWindowEx(hDlg,nLeftScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
					si.fMask = SIF_POS;
					si.nPos = HIWORD(wParam);
					if ( si.nPos < si.nMin )
					{
						si.nPos = si.nMin;
					}
					nLastPos = si.nPos;
					nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				}
				break;
			case SB_PAGELEFT:
				bResult = ::GetWindowRect(hDlg,&crectDlg);
//				nLeftScrollAmt = crectDlg.right - crectDlg.left;
				nLeftScrollAmt = (crectDlg.right - crectDlg.left)/3;
				nResult = ::ScrollWindowEx(hDlg,nLeftScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
				switch (nResult)
				{
				case SIMPLEREGION:
					break;
				case COMPLEXREGION:
					break;
				case NULLREGION:
					break;
				case ERROR:
				default:
					dwResult = ::GetLastError();
					break;
				}
				si.fMask = SIF_POS;
				si.nPos--;
				if ( si.nPos < si.nMin )
				{
					si.nPos = si.nMin;
				}
				nLastPos = si.nPos;
				nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				break;
			case SB_PAGERIGHT:
				bResult = ::GetWindowRect(hDlg,&crectDlg);
//				nRightScrollAmt = crectDlg.left - crectDlg.right;
				nRightScrollAmt = (crectDlg.left - crectDlg.right)/3;
				nResult = ::ScrollWindowEx(hDlg,nRightScrollAmt,0,NULL,NULL,NULL,&crectUpdate,SW_SCROLLCHILDREN|SW_INVALIDATE|SW_ERASE);
				switch (nResult)
				{
				case SIMPLEREGION:
					break;
				case COMPLEXREGION:
					break;
				case NULLREGION:
					break;
				case ERROR:
				default:
					dwResult = ::GetLastError();
					break;
				}
				si.fMask = SIF_POS;
				si.nPos++;
				if ( si.nPos > si.nMax )
				{
					si.nPos = si.nMax;
				}
				nLastPos = si.nPos;
				nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
				break;
			}
		}
		return (INT_PTR)TRUE;
	case WM_COMMAND:
		if ( LOWORD(wParam) == IDOK )
		{
			lpLFParms->lpparms = pparms;
			lpLFParms->lpvals = pvals;
			bResult = EndDialog(hDlg,LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		if ( LOWORD(wParam) == IDCANCEL )
		{
			bResult = EndDialog(hDlg,LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
	}
	return (INT_PTR)FALSE;
}
