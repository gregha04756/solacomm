#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaPage.h"

using namespace std;

#define CONFIGFILEBUFLEN 2800

CRITICAL_SECTION g_SaveRestoreCritSect;
const TCHAR* szSaveRestoreQuitEvent = {_T("SaveRestoreQuitEvent")};
const TCHAR* szSaveRestoreMBRWEvent = {_T("SaveRestoreMBRWEvent")};
HANDLE g_hSaveRestoreMBRWEvent;
extern "C++" unsigned char SOLAMBAddress;
extern "C++" HINSTANCE g_hInst;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" BOOL bSolaConnected;
extern "C++" DWORD dwCommThreadID;
HANDLE g_hSaveRestoreQuitEvent;
extern "C++" BOOL g_bQuit;
extern "C++" HANDLE g_hConfigChangeEvent;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" TCHAR szTitle[];
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveConfigPages;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcXCHConfig;
extern "C++" CSolaMBMap* pcX2CHConfig;
extern "C++" CSolaPage* pcSaveRestorePage;
extern "C++" SOLAMULTIVALUE RegisterAccess[];
extern "C++" SOLAMULTIVALUE ModbusExceptionCodes[];
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" PAGEUPDATE g_PageUpdates[];
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" HWND g_hODResetLineWnd;

DWORD WINAPI SaveRestoreThread(LPVOID lpParam);
extern "C++" float TempVal( BOOL units, short temp );
extern "C++" float HystVal( BOOL units, short temp );
extern "C++" signed short SolaTempVal( BOOL units, short temp );
extern "C++" signed short SolaHystVal( BOOL units, short temp );
extern "C++" int check_CRC16(unsigned char *buf, int buflen);
extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" BOOL UpdatePage(HWND hwndDialog,CSolaPage* lpPage);
/*extern "C++" void ReportError(DWORD dwError); */
extern "C++" INT_PTR CALLBACK LoadParmsDlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam);
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);

const TCHAR szInvalidChars[] = {L'<',L'>',L';',L':',L'"',L'/',L'\\',L'|',L'?',L'*',L'\0'};

BOOL MakeValidFileName(TCHAR* lpfn,size_t fnmaxlen)
{
	volatile TCHAR tcharTestChar;
	const std::wstring wsUScore(_T("_"));
	std::wstring::size_type wstPos = (std::wstring::size_type)0;
	std::wstring wsName(lpfn);
	for ( int i = 0; i < sizeof(szInvalidChars)/sizeof(TCHAR); i++ )
	{
		tcharTestChar = szInvalidChars[i];
		while ( !g_bQuit && (wstPos = wsName.find(tcharTestChar,(std::wstring::size_type)0)) != std::wstring::npos )
		{
			wsName.replace(wstPos,(std::wstring::size_type)1,wsUScore);
		}
	}
	HRESULT hRes = ::StringCchPrintf(lpfn,fnmaxlen,_T("%s"),wsName.c_str());
	return true;
}

LRESULT CALLBACK SaveRestoreDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam)
{
	BOOL bOkCRC;
	static BOOL bSaveOK;
	static int nPageIndex;
	static BOOL bStatus;
	BOOL bResult;
	BOOL bGSFNResult;
	LPNMHDR lpnmhdr;
	LPPSHNOTIFY lppsn;
	int i;
	DWORD dwResult;
	static DWORD dwSaveRestoreThreadID;
	LPSUMMARYTHREADPARMS pSummaryParms;
	static HANDLE hSaveRestoreThread;
	static HANDLE hSaveRestoreThreadDup;
	static MBSNDRCVREQ MBSndRcvReq;
	static unsigned char* pchToSnd;
	static unsigned char* pchEndSnd;
	static unsigned char* pchToRcv;
	static unsigned char* pchEndRcv;
	static CSolaMBMap::LPNUMERICUIPARMS lpUIParms;
	static LPMULTIUIPARMS lpMUIParms;
	static int nControlIDMin;
	static int nControlIDMax;
	int nResult;
	int nLen;
	int nLeftScrollAmt;
	int nRightScrollAmt;
	static HWND hwndScrollLeftRightBar;
	RECT crectDlg;
	RECT crectUpdate;
	static int nLastPos;
	static int nParmValsWrittenCnt;
	static int nParmWriteErrorCnt;
	static SCROLLINFO si = { 0 };
	static HANDLE hSaveConfigFile;
	static TCHAR* lpszSaveFileName;
	PVOID lpvTemp;
	static OPENFILENAME ofnSaveFile;
	static TCHAR szDefaultExt[5];
	static TCHAR szFilter[MAX_PATH];
	SYSTEMTIME st;
	HRESULT hRes;
	char* lpchSaveBuf;
	TCHAR* lpszSaveBuf;
	DWORD dwBytesWritten;
	INT_PTR ipResult;
	static char* lpchLoadFileName;
	static LPLOADFILEPARMS lpLFParms;
	TCHAR szTemp[100];

	switch (uMessage)
	{
	case WM_INITDIALOG:
		::InitializeCriticalSection(&g_SaveRestoreCritSect);
		lpLFParms = NULL;
		lpszSaveFileName = NULL;
		hSaveConfigFile = NULL;
		lpchSaveBuf = NULL;
		lpszSaveBuf = NULL;
		nParmValsWrittenCnt = 0;
		nParmWriteErrorCnt = 0;
		g_hSaveRestoreMBRWEvent = ::CreateEvent(NULL,false,false,szSaveRestoreMBRWEvent);
		bStatus = false;
		bSaveOK = false;
		nPageIndex = (int) ::SendMessage(::GetParent(hDlg),PSM_IDTOINDEX,0,(LPARAM)IDD_SAVERESTOREDLG);
		g_hSaveRestoreQuitEvent = ::CreateEvent(NULL, true, false, szSaveRestoreQuitEvent);
		bResult = ::ResetEvent(g_hSaveRestoreQuitEvent);
		// Start Save&Restore update thread
		pSummaryParms = new SUMMARYTHREADPARMS;
		pSummaryParms->hParentWnd = hDlg;
		pSummaryParms->pDataCritSect = &gRWDataCritSect;
		pSummaryParms->hReadEvent = g_hConfigChangeEvent;
		pSummaryParms->hQuitEvent = g_hSaveRestoreQuitEvent;
		pSummaryParms->nPg = nPageIndex;
		hSaveRestoreThread = ::CreateThread(NULL, 0, SaveRestoreThread, (LPVOID) pSummaryParms, CREATE_SUSPENDED, &dwSaveRestoreThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(), hSaveRestoreThread, GetCurrentProcess(), &hSaveRestoreThreadDup, 0, false, DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(hSaveRestoreThread);
		bResult = UpdatePage(hDlg,pcSaveRestorePage);
		::EnterCriticalSection(&gRWDataCritSect);
		g_nActivePages++;
		g_nActiveConfigPages++;
		g_lpPageDataEvents[nPageIndex].typePage = ConfigPage;
		::LeaveCriticalSection(&gRWDataCritSect);
		if ( bSolaConnected )
		{
			bResult = ::PostMessage(hDlg,WM_APPSAVERESTOREUPD,(WPARAM)1,(LPARAM)0);
		}
		g_PageUpdates[nPageIndex].hPage = hDlg;
		g_PageUpdates[nPageIndex].nMsg = WM_APPSAVERESTOREUPD;
		nControlIDMin = TXTIDBASE + 0x0fff;
		nControlIDMax = TXTIDBASE;
		for ( i = 0; i < pcSaveRestorePage->GetSize(); i++ )
		{
			nResult = TXTIDBASE + pcSaveRestorePage->ItemMap(i)->GetStartRegAddr(pcSaveRestorePage->ItemIndex(i));
			nControlIDMin = nControlIDMin < nResult ? nControlIDMin : nResult;
			nControlIDMax = nControlIDMax > nResult ? nControlIDMax : nResult;
		}
		bResult = ::GetClientRect(hDlg,&crectDlg);
		hwndScrollLeftRightBar = ::CreateWindowEx(	WS_EX_LEFT | SBS_HORZ,
												WC_SCROLLBAR,
												NULL,
												WS_CHILD | WS_VISIBLE,
												crectDlg.left,
												0,
												2*(crectDlg.right-crectDlg.left),
												15,
												hDlg,
												NULL,
												g_hInst,
												NULL);
		if ( (dwResult = ::GetLastError() != NOERROR ) )
		{
			ReportError(dwResult);
		}
		if ( hwndScrollLeftRightBar )
		{
			si.cbSize = sizeof(si);
			si.fMask = SIF_ALL;
			bResult = ::GetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si);
			si.fMask = SIF_ALL;
			si.nMax = 3;
			si.nMin = 0;
			si.nPage = 1;
			si.nPos = 0;
			nResult = ::SetScrollInfo(hwndScrollLeftRightBar,SB_CTL,&si,true);
		}
		nLastPos = 0;
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNSAVECONFIG),false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),false);
		if ( bSolaConnected )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNSAVECONFIG),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),bSolaConnected*pcTrendStatus->GetValue((int)13));
		}
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOADPARMSSTATUS,_T(""));
		Make_Reg_Group_List(pcSaveRestorePage);
		break;
	case  WM_CHILDACTIVATE:
		Make_Reg_Group_List(pcSaveRestorePage);
		nParmValsWrittenCnt = 0;
		nParmWriteErrorCnt = 0;
		if ( NULL != lpLFParms )
		{
			if ( NULL != lpLFParms->lpparms )
			{
				delete[] lpLFParms->lpparms;
			}
			if ( NULL != lpLFParms->lpvals )
			{
				delete[] lpLFParms->lpvals;
			}
			if ( NULL != lpLFParms->lpifile )
			{
				if ( lpLFParms->lpifile->is_open() )
				{
					lpLFParms->lpifile->close();
				}
				delete lpLFParms->lpifile;
				lpLFParms->lpifile = NULL;
			}
			delete lpLFParms;
			lpLFParms = NULL;
		}
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNSAVECONFIG),false);
		bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),false);
		if ( bSolaConnected )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNSAVECONFIG),true);
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),bSolaConnected*pcTrendStatus->GetValue((int)13));
		}
		bResult = UpdatePage(hDlg,pcSaveRestorePage);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOADPARMSSTATUS,_T(""));
		break;
	case WM_APPSAVERESTOREUPD:
		bResult = UpdatePage(hDlg,pcSaveRestorePage);
		if ( bSolaConnected && !LOWORD(wParam))
		{
			bResult = ::SetEvent(::g_hPageUpdEvents[nPageIndex]);
		}
		break;
	case WM_APPMBRESPONSE:
		nParmValsWrittenCnt++;
		if ( LOWORD(wParam) == 3 )
		{
			::EnterCriticalSection(&gRWDataCritSect);
			bOkCRC =  check_CRC16(chMBRcvBuf, (int)(pchEndRcv-pchToRcv));
			::LeaveCriticalSection(&gRWDataCritSect);
			nParmWriteErrorCnt = (bOkCRC ? nParmWriteErrorCnt : ++nParmWriteErrorCnt);
			if ( bOkCRC && ((unsigned char)0x80 & chMBRcvBuf[1]) )
			{
//				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
				nParmWriteErrorCnt++;
			}
			if ( bOkCRC && (0x06 == chMBRcvBuf[1]) )
			{
			}
		}
		if ( LOWORD(wParam) == 4 )
		{
			if ( (unsigned char)0x80 & chMBRcvBuf[1] )
			{
//				k = ::MessageBox(hDlg,_T("Not permitted"),szTitle,MB_OK);
				nParmWriteErrorCnt++;
			}
		}
		hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("Parms. sent: %d Errors: %d"), nParmValsWrittenCnt,nParmWriteErrorCnt);
		bResult = ::SetDlgItemText(hDlg,IDC_TXTLOADPARMSSTATUS,szTemp);
		if ( nParmValsWrittenCnt < pcSaveRestorePage->GetSize() )
		{
			::EnterCriticalSection(&gRWDataCritSect);
			MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
			MBSndRcvReq.ppchToSnd = &pchToSnd;
			MBSndRcvReq.ppchEndSnd = &pchEndSnd;
			MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
			*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
			*(*MBSndRcvReq.ppchEndSnd)++ = 0x06;
			*(*MBSndRcvReq.ppchEndSnd)++ = ((lpLFParms->lpparms[nParmValsWrittenCnt]) >> 8) & 0x00ff;
			*(*MBSndRcvReq.ppchEndSnd)++ = (lpLFParms->lpparms[nParmValsWrittenCnt]) & 0x00ff;
			*(*MBSndRcvReq.ppchEndSnd)++ = ((lpLFParms->lpvals[nParmValsWrittenCnt]) >> 8) & 0x00ff;
			*(*MBSndRcvReq.ppchEndSnd)++ = (lpLFParms->lpvals[nParmValsWrittenCnt]) & 0x00ff;
			MBSndRcvReq.pchRcvBuf = pchToRcv = pchEndRcv = chMBRcvBuf;
			MBSndRcvReq.ppchToRcv = &pchToRcv;
			MBSndRcvReq.ppchEndRcv = &pchEndRcv;
			MBSndRcvReq.nRcvBufSize = sizeof(chMBRcvBuf);
			MBSndRcvReq.hPage = hDlg;
			MBSndRcvReq.nMsg = WM_APPMBRESPONSE;
			g_MBSndRcvReqQ.push(MBSndRcvReq);
			::LeaveCriticalSection(&gRWDataCritSect);
		}
		if ( nParmValsWrittenCnt == pcSaveRestorePage->GetSize() )
		{
			bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),bSolaConnected*pcTrendStatus->GetValue((int)13));
		}
		break;
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
		break;
	case WM_COMMAND:
		if ( (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_BTNSAVECONFIG) )
		{
			bResult = ::SetDlgItemText(hDlg,IDC_TXTLOADPARMSSTATUS,_T(""));
			if ( bSolaConnected && (NULL == lpszSaveFileName) )
			{
				try
				{
					lpszSaveFileName = (TCHAR*)new TCHAR[MAX_PATH];
					lpchSaveBuf = (char*)new char[CONFIGFILEBUFLEN];
					lpszSaveBuf = (TCHAR*)new TCHAR[CONFIGFILEBUFLEN];
				}
				catch (std::bad_alloc &ba)
				{
					ReportError(::GetLastError());
					bSaveOK = false;
				}
			}
			if ( bSolaConnected && (NULL == hSaveConfigFile) && (NULL != lpszSaveFileName) )
			{
				lpvTemp = ::SecureZeroMemory((PVOID)lpszSaveFileName,MAX_PATH*sizeof(TCHAR));
				nResult = MultiByteToWideChar(CP_ACP,
					MB_PRECOMPOSED,
					(LPCSTR)pcSystemIDBurnerName->GetStr((int)0),
					-1,
					lpszSaveFileName,
					MAX_PATH);
				nResult = ::LoadString(g_hInst,IDS_DEFAULTEXT,szDefaultExt,sizeof(szDefaultExt)/sizeof(TCHAR));
				nResult = ::LoadString(g_hInst,IDS_NAMEFILTER,szFilter,sizeof(szFilter)/sizeof(TCHAR));
				ofnSaveFile.lStructSize = (DWORD)sizeof(OPENFILENAME);
				ofnSaveFile.hwndOwner = hDlg;
				ofnSaveFile.lpstrFilter = szFilter;
				::GetLocalTime(&st);
				hRes = ::StringCchLength(lpszSaveFileName,MAX_PATH,(size_t*)&nLen);
				hRes = ::StringCchPrintf(&lpszSaveFileName[nLen],MAX_PATH-nLen,_T("_%d-%02d-%02d_%02d_%02d"),st.wYear,st.wMonth,st.wDay,st.wHour,st.wMinute);
				ofnSaveFile.lpstrFile = lpszSaveFileName;
				ofnSaveFile.nMaxFile = MAX_PATH;
				ofnSaveFile.lpstrFileTitle = NULL;
				ofnSaveFile.lpstrTitle = NULL;
				ofnSaveFile.Flags = OFN_OVERWRITEPROMPT;
				ofnSaveFile.lpstrDefExt = szDefaultExt;
				while ( !g_bQuit && !(bGSFNResult = ::GetSaveFileName(&ofnSaveFile)) )
				{
					if ( FNERR_INVALIDFILENAME == ::CommDlgExtendedError() )
					{
						bResult = MakeValidFileName(lpszSaveFileName,(size_t)MAX_PATH);
					}
					else
					{
						break;
					}
				}
				if ( bGSFNResult )
				{
					hRes = ::StringCchLength(lpszSaveFileName,MAX_PATH,(size_t*)&nLen);
					if ( nLen )
					{
						hSaveConfigFile = ::CreateFile(lpszSaveFileName,
							GENERIC_READ|GENERIC_WRITE,
							0,
							NULL,
							CREATE_ALWAYS,
							FILE_ATTRIBUTE_NORMAL,
							NULL);
						if ( hSaveConfigFile == INVALID_HANDLE_VALUE )
						{
							ReportError(::GetLastError());
							hSaveConfigFile = NULL;
							bSaveOK = false;
						}
					}
				}
				else
				{
					ReportError(::CommDlgExtendedError());
					bSaveOK = false;
				}
			}
			if ( NULL != hSaveConfigFile && (NULL != lpchSaveBuf) && (NULL != lpszSaveBuf) )
			{
				lpvTemp = ::SecureZeroMemory((PVOID)lpchSaveBuf,CONFIGFILEBUFLEN*sizeof(char));
				lpvTemp = ::SecureZeroMemory((PVOID)lpszSaveBuf,CONFIGFILEBUFLEN*sizeof(TCHAR));
				for ( i = 0; i < pcSaveRestorePage->GetSize(); i++ )
				{
					hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
					hRes = ::StringCchCat(lpszSaveBuf,CONFIGFILEBUFLEN-nLen,
						pcSaveRestorePage->ItemMap(i)->GetParmName(pcSaveRestorePage->ItemIndex(i)));
					hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
					hRes = ::StringCchCat(lpszSaveBuf,CONFIGFILEBUFLEN-nLen,_T(","));
				}
				hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
				hRes = ::StringCchCat(lpszSaveBuf,CONFIGFILEBUFLEN-nLen,_T("\n"));
				hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
				nResult = ::WideCharToMultiByte(CP_ACP,0,lpszSaveBuf,nLen,lpchSaveBuf,CONFIGFILEBUFLEN,NULL,NULL);
				bSaveOK = true;
				bResult = ::WriteFile(hSaveConfigFile,(LPCVOID)lpchSaveBuf,(DWORD)(nLen*sizeof(char)),&dwBytesWritten,NULL);
				if ( !bResult )
				{
					ReportError(::GetLastError());
					bResult = ::CloseHandle(hSaveConfigFile);
					hSaveConfigFile = NULL;
					bSaveOK = false;
				}
				lpvTemp = ::SecureZeroMemory((PVOID)lpchSaveBuf,CONFIGFILEBUFLEN*sizeof(char));
				lpvTemp = ::SecureZeroMemory((PVOID)lpszSaveBuf,CONFIGFILEBUFLEN*sizeof(TCHAR));
				for ( i = 0; i < pcSaveRestorePage->GetSize(); i++ )
				{
					hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
					hRes = ::StringCchPrintf(&lpszSaveBuf[nLen],CONFIGFILEBUFLEN-nLen,_T("%d,"),
						pcSaveRestorePage->ItemMap(i)->GetStartRegAddr(pcSaveRestorePage->ItemIndex(i)));
				}
				hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
				hRes = ::StringCchCat(lpszSaveBuf,CONFIGFILEBUFLEN-nLen,_T("\n"));
				hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
				nResult = ::WideCharToMultiByte(CP_ACP,0,lpszSaveBuf,nLen,lpchSaveBuf,CONFIGFILEBUFLEN,NULL,NULL);
				bSaveOK = true;
				bResult = ::WriteFile(hSaveConfigFile,(LPCVOID)lpchSaveBuf,(DWORD)(nLen*sizeof(char)),&dwBytesWritten,NULL);
				if ( !bResult )
				{
					ReportError(::GetLastError());
					bResult = ::CloseHandle(hSaveConfigFile);
					hSaveConfigFile = NULL;
					bSaveOK = false;
				}
				lpvTemp = ::SecureZeroMemory((PVOID)lpchSaveBuf,CONFIGFILEBUFLEN*sizeof(char));
				lpvTemp = ::SecureZeroMemory((PVOID)lpszSaveBuf,CONFIGFILEBUFLEN*sizeof(TCHAR));
				for ( i = 0; i < pcSaveRestorePage->GetSize(); i++ )
				{
					hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
					hRes = ::StringCchPrintf(&lpszSaveBuf[nLen],CONFIGFILEBUFLEN-nLen,_T("%d,"),
						pcSaveRestorePage->ItemMap(i)->GetValue(pcSaveRestorePage->ItemIndex(i)));
				}
				hRes = ::StringCchLength(lpszSaveBuf,CONFIGFILEBUFLEN,(size_t*)&nLen);
				hRes = ::StringCchCat(lpszSaveBuf,CONFIGFILEBUFLEN-nLen,_T("\n\r"));
				nResult = ::WideCharToMultiByte(CP_ACP,0,lpszSaveBuf,nLen,lpchSaveBuf,CONFIGFILEBUFLEN,NULL,NULL);
				bSaveOK = true;
				bResult = ::WriteFile(hSaveConfigFile,(LPCVOID)lpchSaveBuf,(DWORD)(nLen*sizeof(char)),&dwBytesWritten,NULL);
				if ( !bResult )
				{
					ReportError(::GetLastError());
					bResult = ::CloseHandle(hSaveConfigFile);
					hSaveConfigFile = NULL;
					bSaveOK = false;
				}
			}
			if ( NULL != lpszSaveBuf )
			{
				delete[] lpszSaveBuf;
				lpszSaveBuf = NULL;
			}
			if ( NULL != lpchSaveBuf )
			{
				delete[] lpchSaveBuf;
				lpchSaveBuf = NULL;
			}
			if ( NULL != hSaveConfigFile )
			{
				bResult = ::FlushFileBuffers(hSaveConfigFile);
				if ( !bResult )
				{
					ReportError(::GetLastError());
				}
				bResult = ::CloseHandle(hSaveConfigFile);
				hSaveConfigFile = NULL;
			}
			if ( NULL != lpszSaveFileName )
			{
				delete[] lpszSaveFileName;
				lpszSaveFileName = NULL;
			}
			if ( bSaveOK )
			{
				nResult = ::MessageBox(hDlg,_T("Data saved"),szTitle,MB_OK);
			}
			else
			{
				nResult = ::MessageBox(hDlg,_T("Data not saved"),szTitle,MB_OK);
			}
		}
		if ( bSolaConnected && (HIWORD(wParam) == BN_CLICKED) && (LOWORD(wParam) == IDC_BTNLOADCONFIG) )
		{
			if ( NULL != lpLFParms )
			{
				if ( NULL != lpLFParms->lpparms )
				{
					delete[] lpLFParms->lpparms;
				}
				if ( NULL != lpLFParms->lpvals )
				{
					delete[] lpLFParms->lpvals;
				}
				if ( NULL != lpLFParms->lpifile )
				{
					if ( lpLFParms->lpifile->is_open() )
					{
						lpLFParms->lpifile->close();
					}
					delete lpLFParms->lpifile;
					lpLFParms->lpifile = NULL;
				}
				delete lpLFParms;
				lpLFParms = NULL;
			}
			if ( NULL == lpszSaveFileName )
			{
				try
				{
					lpszSaveFileName = (TCHAR*)new TCHAR[MAX_PATH];
				}
				catch (std::bad_alloc &ba)
				{
					ReportError(::GetLastError());
				}
			}
			if ( NULL != lpszSaveFileName )
			{
				lpvTemp = ::SecureZeroMemory((PVOID)lpszSaveFileName,MAX_PATH*sizeof(TCHAR));
				nResult = ::LoadString(g_hInst,IDS_DEFAULTEXT,szDefaultExt,sizeof(szDefaultExt)/sizeof(TCHAR));
				nResult = ::LoadString(g_hInst,IDS_NAMEFILTER,szFilter,sizeof(szFilter)/sizeof(TCHAR));
				ofnSaveFile.lStructSize = (DWORD)sizeof(OPENFILENAME);
				ofnSaveFile.hwndOwner = hDlg;
				ofnSaveFile.lpstrFilter = szFilter;
				ofnSaveFile.lpstrFile = lpszSaveFileName;
				ofnSaveFile.nMaxFile = MAX_PATH;
				ofnSaveFile.lpstrFileTitle = NULL;
				ofnSaveFile.lpstrTitle = NULL;
				ofnSaveFile.Flags = OFN_FILEMUSTEXIST;
				ofnSaveFile.lpstrDefExt = szDefaultExt;
				if ( (bResult = ::GetOpenFileName(&ofnSaveFile)) )
				{
					hRes = ::StringCchLength(lpszSaveFileName,MAX_PATH,(size_t*)&nLen);
					try
					{
						lpchLoadFileName = (char*)new char[nLen+1];
					}
					catch (std::bad_alloc &ba)
					{
						ReportError(::GetLastError());
					}
					if ( lpchLoadFileName )
					{
						lpvTemp = ::SecureZeroMemory((PVOID)lpchLoadFileName,(SIZE_T)nLen+1);
						nResult = ::WideCharToMultiByte(CP_ACP,0,lpszSaveFileName,-1,lpchLoadFileName,nLen,NULL,NULL);
					}
					if ( nLen && lpchLoadFileName )
					{
						try
						{
							lpLFParms = (LPLOADFILEPARMS) new LOADFILEPARMS;
						}
						catch (ifstream::failure fof)
						{
							::ReportError(::GetLastError());
						}
						if ( NULL != lpLFParms )
						{
							lpLFParms->lpifile = NULL;
							lpLFParms->lpparms = NULL;
							lpLFParms->lpvals = NULL;
						}
						try
						{
							lpLFParms->lpifile = (std::ifstream*) new std::ifstream(lpchLoadFileName,ifstream::in);
						}
						catch (ifstream::failure fof)
						{
							::ReportError(::GetLastError());
						}
					}
					if ( NULL != lpLFParms->lpifile )
					{
						if ( lpLFParms->lpifile->is_open() )
						{
							ipResult = IDCANCEL;
							if ( NULL != lpLFParms )
							{
								ipResult = ::DialogBoxParam(g_hInst,
									MAKEINTRESOURCE(IDD_LOADPARMSDLG),
									hDlg,
									LoadParmsDlgProc,
									(LPARAM)lpLFParms);
							}
							if ( -1 == ipResult )
							{
								::ReportError(::GetLastError());
							}
							if ( IDOK == ipResult )
							{
								if ( (NULL != lpLFParms->lpparms) && (NULL != lpLFParms->lpvals) )
								{
									bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),false);
									nParmValsWrittenCnt = 0;
									nParmWriteErrorCnt = 0;
									::EnterCriticalSection(&gRWDataCritSect);
									MBSndRcvReq.pchSndBuf = pchToSnd = pchEndSnd = chMBSndBuf;
									MBSndRcvReq.ppchToSnd = &pchToSnd;
									MBSndRcvReq.ppchEndSnd = &pchEndSnd;
									MBSndRcvReq.nSndBufSize = sizeof(chMBSndBuf);
									*(*MBSndRcvReq.ppchEndSnd)++ = SOLAMBAddress;
									*(*MBSndRcvReq.ppchEndSnd)++ = 0x06;
									*(*MBSndRcvReq.ppchEndSnd)++ = ((lpLFParms->lpparms[nParmValsWrittenCnt]) >> 8) & 0x00ff;
									*(*MBSndRcvReq.ppchEndSnd)++ = (lpLFParms->lpparms[nParmValsWrittenCnt]) & 0x00ff;
									*(*MBSndRcvReq.ppchEndSnd)++ = ((lpLFParms->lpvals[nParmValsWrittenCnt]) >> 8) & 0x00ff;
									*(*MBSndRcvReq.ppchEndSnd)++ = (lpLFParms->lpvals[nParmValsWrittenCnt]) & 0x00ff;
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
							if ( IDCANCEL == ipResult )
							{
								bResult = ::SetDlgItemText(hDlg,IDC_TXTLOADPARMSSTATUS,_T(""));
								if ( NULL != lpLFParms->lpparms )
								{
									delete[] lpLFParms->lpparms;
									lpLFParms->lpparms = NULL;
								}
								if ( NULL != lpLFParms->lpvals )
								{
									delete[] lpLFParms->lpvals;
									lpLFParms->lpvals = NULL;
								}
								if ( bSolaConnected )
								{
									bResult = ::EnableWindow(::GetDlgItem(hDlg,IDC_BTNLOADCONFIG),bSolaConnected*pcTrendStatus->GetValue((int)13));
								}
							}
						}
					}
				}
			}
			if ( NULL != lpLFParms )
			{
				if ( NULL != lpLFParms->lpifile )
				{
					if ( lpLFParms->lpifile->is_open() )
					{
						lpLFParms->lpifile->close();
					}
					delete lpLFParms->lpifile;
					lpLFParms->lpifile = NULL;
				}
			}
			if ( NULL != lpszSaveFileName )
			{
				delete[] lpszSaveFileName;
				lpszSaveFileName = NULL;
			}
			if ( NULL != lpchLoadFileName )
			{
				delete[] lpchLoadFileName;
				lpchLoadFileName = NULL;
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
				if ( NULL != lpLFParms )
				{
					if ( NULL != lpLFParms->lpparms )
					{
						delete[] lpLFParms->lpparms;
					}
					if ( NULL != lpLFParms->lpvals )
					{
						delete[] lpLFParms->lpvals;
					}
					if ( NULL != lpLFParms->lpifile )
					{
						if ( lpLFParms->lpifile->is_open() )
						{
							lpLFParms->lpifile->close();
						}
						delete lpLFParms->lpifile;
						lpLFParms->lpifile = NULL;
					}
					delete lpLFParms;
					lpLFParms = NULL;
				}
				if ( hSaveRestoreThread )
				{
					bResult = ::SetEvent(g_hSaveRestoreQuitEvent);
					dwResult = ::WaitForSingleObject(hSaveRestoreThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Save Restore update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hSaveRestoreThread);
					bResult = ::CloseHandle(hSaveRestoreThreadDup);
				}
				bResult = ::CloseHandle(g_hSaveRestoreQuitEvent);
				bResult = ::CloseHandle(g_hSaveRestoreMBRWEvent);
				::DeleteCriticalSection(&g_SaveRestoreCritSect);
			}
			break;

		case PSN_RESET:   //sent when Cancel button pressed
			if ( lppsn->lParam )	// lParam TRUE if OK button
			{
				if ( hSaveRestoreThread )
				{
					bResult = ::SetEvent(g_hSaveRestoreQuitEvent);
					dwResult = ::WaitForSingleObject(hSaveRestoreThread, 5000);
					switch (dwResult)
					{
					case WAIT_TIMEOUT:
						::MessageBox( hDlg, _T("Wait for Save Restore update thread completion timed out"), szTitle, MB_OK );
						break;
					case WAIT_OBJECT_0:
						break;
					}
					bResult = ::CloseHandle(hSaveRestoreThread);
					bResult = ::CloseHandle(hSaveRestoreThreadDup);
				}
				bResult = ::CloseHandle(g_hSaveRestoreQuitEvent);
				bResult = ::CloseHandle(g_hSaveRestoreMBRWEvent);
				::DeleteCriticalSection(&g_SaveRestoreCritSect);
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

DWORD WINAPI SaveRestoreThread(LPVOID lpParam)
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
	bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hSaveRestoreQuitEvent, GetCurrentProcess(), &hEvents[0], 0, false, DUPLICATE_SAME_ACCESS);
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
			bResult = ::PostMessage(hParentWnd, WM_APPSAVERESTOREUPD, (WPARAM) 0, (LPARAM) 0);
			if ( !bResult )
			{
				::MessageBox(hParentWnd, _T("Save Restore thread PostMessage error"), szTitle, MB_OK);
			}
		}
//		bResult = ::SetEvent(::g_hPageUpdEvents[nPage]);
	}
	bResult = ::CloseHandle(hEvents[0]);
	bResult = ::CloseHandle(hEvents[1]);
	return 0;
}
