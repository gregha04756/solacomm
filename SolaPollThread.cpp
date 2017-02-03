#include "StdAfx.h"
#include "SolaTCPComm.h"

BOOL CSolaTCPComm::ProcessResponse(CSolaTCPComm* p)
{
	BOOL bRtnResult = true;
	BOOL bResult;
	int nResult;
	int i;
	int j;
	int ndx;
	CSolaMBMap* lpMap;
	LPSOLAALERT lpAlertRecord;
	LPSOLALOCKOUT lpLockoutRecord;
	unsigned char uchRespLo;
	unsigned char uchRespHi;
	U32 u32Value;
	U32 u32Ret;
	PVOID lpvTemp;
	MBReqType rt;
	char chResp[512];

	std::list<MBMESSAGE>::iterator it1;
	std::list<MBRESPMESSAGE>::iterator it2;
	std::list<MBMESSAGE>::iterator itx;
	std::list<MBRESPMESSAGE>::iterator ity;
#if _DEBUG
	std::wstring sss1;
	std::wstring sss2;
#endif

#if TCPDEBUGGING
	nResult = sprintf_s(chResp,sizeof(chResp),"Req. count=%d  Resp. count=%d",p->m_lplReqMBMsgs->size(),p->m_lplRespMBMsgs->size());
	::EnterCriticalSection(p->m_lpDbgFileCritSect);
	if ( nResult && p->m_lpdbgfile->is_open() )
	{
		*p->m_lpdbgfile << chResp << "\n";
	}
	::LeaveCriticalSection(p->m_lpDbgFileCritSect);
#endif

	BOOL bMatch = true;
	while ( bMatch && !p->m_bQuit )
	{
		::EnterCriticalSection(p->m_lpRespCritSect);
		bMatch = false;
		for( it1 = p->m_lplReqMBMsgs->begin(); !bMatch && (it1 != p->m_lplReqMBMsgs->end()); (it1)++ )
		{
			for( it2 = p->m_lplRespMBMsgs->begin(); !bMatch && (it2 != p->m_lplRespMBMsgs->end()); (it2)++ )
			{
				if ( it1->mbaphdr.uchUnitIdentifier == it2->mbaphdr.uchUnitIdentifier &&
					it1->mbaphdr.usTransactionIdentifier == it2->mbaphdr.usTransactionIdentifier )
				{
					itx = it1;
					ity = it2;
					bMatch = true;
				}
			}
		}
		if ( bMatch )
		{
			switch (ity->mbr.uchFunctionCode)
			{
			case 0x03:
			case 0x04:
				rt = itx->rt;
				switch (rt)
				{
				case LPInitIDreq:
#if TCPDEBUGGING
					nResult = RespFormat(ity,chResp,sizeof(chResp));
					::EnterCriticalSection(p->m_lpDbgFileCritSect);
					if ( nResult && p->m_lpdbgfile->is_open() )
					{
						*p->m_lpdbgfile << "Processing from list: " << chResp << "\n";
					}
					::LeaveCriticalSection(p->m_lpDbgFileCritSect);
#endif
					lpMap = (CSolaMBMap*)itx->lpMap;
					for ( ndx = 0; ndx < lpMap->GetRegGroupSize(); ndx++ )
					{
						switch (lpMap->GetType((int)ndx))
						{
						case CSolaMBMap::Novalue:
							break;
						case CSolaMBMap::DupStringValue:
						case CSolaMBMap::Stringvalue:
							if (((lpMap->GetType((int)ndx) == CSolaMBMap::Stringvalue) ||
								(lpMap->GetType((int)ndx) == CSolaMBMap::DupStringValue)) &&
								(lpMap->GetStr((int)ndx) != NULL))
							{
								lpvTemp = (PVOID)lpMap->SetStr((int)ndx,(unsigned char*)ity->mbr.chResponse,ity->mbr.uscbRespLen);
								if (std::wstring(lpMap->GetParmName((int)ndx)) == std::wstring(pc_Dup_OEMID->GetParmName(0)))
								{
									if (NULL == p->m_szOEMID)
									{
										p->m_szOEMID = (TCHAR*) new TCHAR[SOLAOEMIDLEN+1];
									}
									lpvTemp = SecureZeroMemory((PVOID)p->m_szOEMID,sizeof(SOLAOEMIDLEN)+1);
									nResult = ::MultiByteToWideChar(
										CP_ACP,
										MB_PRECOMPOSED,
										(LPCSTR)lpMap->GetStr((int)ndx),
										-1,
										p->m_szOEMID,
										SOLAOEMIDLEN+1);
								}
								if (std::wstring(lpMap->GetParmName((int)ndx)) == std::wstring(pc_Dup_Burner_Name->GetParmName(0)))
								{
									if (NULL == p->m_szBurnerName)
									{
										p->m_szBurnerName = (TCHAR*) new TCHAR[SOLABURNERNAMELEN+1];
									}
									lpvTemp = SecureZeroMemory((PVOID)p->m_szBurnerName,sizeof(SOLABURNERNAMELEN)+1);
									nResult = ::MultiByteToWideChar(
										CP_ACP,
										MB_PRECOMPOSED,
										(LPCSTR)lpMap->GetStr((int)ndx),
										-1,
										p->m_szBurnerName,
										SOLABURNERNAMELEN+1);
								}
							}
							if ((NULL != p->m_szBurnerName) && (NULL != p->m_szOEMID))
							{
								if ( !p->m_lpSolaID )
								{
									p->m_lpSolaID = (LPSOLAIDRESPONSE) new SOLAIDRESPONSE;
								}
								if (NULL != p->m_lpSolaID)
								{
									lpvTemp = SecureZeroMemory((PVOID)p->m_lpSolaID,sizeof(SOLAIDRESPONSE));
									bResult = p->Set_Sola_Connected(true);
									bResult = ::PostMessage(p->m_hwndParent,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)p->m_lpSolaID);
								}
							}
							break;
						case CSolaMBMap::Unsigned32:
							j = ndx;
							u32Value = ntohl((ity->mbr.chResponse[(j*4)+3]*16777216L) +
								(ity->mbr.chResponse[(j*4)+2]*65536L) +
								(ity->mbr.chResponse[(j*4)+1]*256L) +
								ity->mbr.chResponse[(j*4)]);
							u32Ret = lpMap->SetU32Value((int)ndx,u32Value);
							break;
						default:
							lpMap->SetValue((int)ndx,ntohs((256*ity->mbr.chResponse[(ndx*2)+1])+ity->mbr.chResponse[(ndx*2)]));
							break;
						}
					}
					break;
				case LPHoldRegReq:
#if TCPDEBUGGING
					nResult = RespFormat(ity,chResp,sizeof(chResp));
					::EnterCriticalSection(p->m_lpDbgFileCritSect);
					if ( nResult && p->m_lpdbgfile->is_open() )
					{
						*p->m_lpdbgfile << "Processing from list: " << chResp << "\n";
					}
					::LeaveCriticalSection(p->m_lpDbgFileCritSect);
#endif
					lpMap = (CSolaMBMap*)itx->lpMap;
					for ( ndx = 0; ndx < lpMap->GetRegGroupSize(); ndx++ )
					{
						switch (lpMap->GetType((int)ndx))
						{
						case CSolaMBMap::Novalue:
							break;
						case CSolaMBMap::DupStringValue:
						case CSolaMBMap::Stringvalue:
#if _DEBUG
							sss1.assign(lpMap->GetParmName((int)ndx));
							sss2.assign(pcSystemIDInstallationData->GetParmName((int)0));
							if ((lpMap->GetStartRegAddr((int)0) == 0x00B8) || (sss1 == sss2))
							{
								j = ndx;
							}
#endif
							if (((lpMap->GetType((int)ndx) == CSolaMBMap::Stringvalue) ||
								(lpMap->GetType((int)ndx) == CSolaMBMap::DupStringValue)) &&
								(lpMap->GetStr((int)ndx) != NULL))
							{
								lpvTemp = (PVOID)lpMap->SetStr((int)ndx,(unsigned char*)ity->mbr.chResponse,ity->mbr.uscbRespLen);
							}
							break;
						case CSolaMBMap::Unsigned32:
							j = ndx;
							u32Value = ntohl((ity->mbr.chResponse[(j*4)+3]*16777216L) +
								(ity->mbr.chResponse[(j*4)+2]*65536L) +
								(ity->mbr.chResponse[(j*4)+1]*256L) +
								ity->mbr.chResponse[(j*4)]);
							u32Ret = lpMap->SetU32Value((int)ndx,u32Value);
							break;
						default:
							j = ndx;
							uchRespLo = ity->mbr.chResponse[(j*2)+1];
							uchRespHi = ity->mbr.chResponse[(j*2)];
							lpMap->SetValue((int)ndx,uchRespLo+(256*uchRespHi));
							break;
						}
					}
					break;
				case LPAlertLogReq:
					lpAlertRecord = (LPSOLAALERT)itx->lpMap;
					lpAlertRecord->pAlertRecord->aa.usAlertCode =  ntohs((256*ity->mbr.chResponse[1]) + ity->mbr.chResponse[0]);
					lpAlertRecord->pAlertRecord->aa.ulCycle = ntohl((256*256*256*ity->mbr.chResponse[5]) + (256*256*ity->mbr.chResponse[4]) +
						(256*ity->mbr.chResponse[3]) + (ity->mbr.chResponse[2]));
					lpAlertRecord->pAlertRecord->aa.ulHours = ntohl((256*256*256*(ity->mbr.chResponse[9])) + (256*256*ity->mbr.chResponse[7]) +
						(256*ity->mbr.chResponse[7]) + (256*256*256*ity->mbr.chResponse[6]));
					lpAlertRecord->pAlertRecord->aa.uchCount = ity->mbr.chResponse[11];
					break;
				case LPLockoutLogReq:
					lpLockoutRecord = (LPSOLALOCKOUT)itx->lpMap;
					lpLockoutRecord->pLockoutUnion->slr.usLockoutCode = ntohs( (256*ity->mbr.chResponse[1]) + ity->mbr.chResponse[0]);
					lpLockoutRecord->pLockoutUnion->slr.usAnnunciatorFirstOut = ntohs((256*ity->mbr.chResponse[3]) + ity->mbr.chResponse[2]);
					lpLockoutRecord->pLockoutUnion->slr.usBurnerControlState = ntohs((256*ity->mbr.chResponse[5]) + ity->mbr.chResponse[4]);
					lpLockoutRecord->pLockoutUnion->slr.usSequenceTime = ntohs((256*ity->mbr.chResponse[7]) + ity->mbr.chResponse[6]);
					lpLockoutRecord->pLockoutUnion->slr.ulCycle = ntohl((16777216*ity->mbr.chResponse[11]) + (65536*ity->mbr.chResponse[10]) +
						(256*ity->mbr.chResponse[9]) + (ity->mbr.chResponse[8]));
					lpLockoutRecord->pLockoutUnion->slr.ulHours = ntohl((16777216*ity->mbr.chResponse[15]) + (65536*ity->mbr.chResponse[14]) +
						(256*ity->mbr.chResponse[13]) + (ity->mbr.chResponse[12]));
					lpLockoutRecord->pLockoutUnion->slr.usIO = ntohs((256*ity->mbr.chResponse[17]) + ity->mbr.chResponse[16]);
					lpLockoutRecord->pLockoutUnion->slr.usAnnunciator = ntohs((256*ity->mbr.chResponse[19]) + ity->mbr.chResponse[18]);
					lpLockoutRecord->pLockoutUnion->slr.usOutletTemperature = ntohs((256*ity->mbr.chResponse[21]) + ity->mbr.chResponse[20]);
					lpLockoutRecord->pLockoutUnion->slr.usInletTemperature = ntohs((256*ity->mbr.chResponse[23]) + ity->mbr.chResponse[22]);
					lpLockoutRecord->pLockoutUnion->slr.usDHWTemperature = ntohs((256*ity->mbr.chResponse[25]) + ity->mbr.chResponse[24]);
					lpLockoutRecord->pLockoutUnion->slr.usODTemperature = ntohs((256*ity->mbr.chResponse[27]) + ity->mbr.chResponse[26]);
					lpLockoutRecord->pLockoutUnion->slr.usStackTemperature = ntohs((256*ity->mbr.chResponse[29]) + ity->mbr.chResponse[28]);
					lpLockoutRecord->pLockoutUnion->slr.us4to20mAInput = ntohs((256*ity->mbr.chResponse[31]) + ity->mbr.chResponse[30]);
					lpLockoutRecord->pLockoutUnion->slr.ucFaultData0 = ity->mbr.chResponse[32];
					lpLockoutRecord->pLockoutUnion->slr.ucFaultData1 = ity->mbr.chResponse[33];
					break;
				}
				break;
			case 0x06:
			case 0x86:
				::EnterCriticalSection(&gRWDataCritSect);
				i = 0;
				g_MBSndRcvReqQ.front().pchRcvBuf[i++] = (char)ity->mbaphdr.uchUnitIdentifier;
				g_MBSndRcvReqQ.front().pchRcvBuf[i++] = (char)ity->mbr.uchFunctionCode;
				for ( ; !p->m_bQuit && ity->mbr.chResponse && i-2 < (ity->mbaphdr.usLength-2); i++ )
				{
					g_MBSndRcvReqQ.front().pchRcvBuf[i] = (char)ity->mbr.chResponse[i-2];
				}
				*g_MBSndRcvReqQ.front().ppchEndRcv = g_MBSndRcvReqQ.front().pchRcvBuf + (ity->mbaphdr.usLength);
				bResult = ::PostMessage(g_MBSndRcvReqQ.front().hPage, g_MBSndRcvReqQ.front().nMsg, (WPARAM) 4, (LPARAM) 0);
				g_MBSndRcvReqQ.pop();
				::LeaveCriticalSection(&gRWDataCritSect);
				break;
			case 0x10:
			case 0x90:
				::EnterCriticalSection(&gRWDataCritSect);
				i = 0;
				g_MBSndRcvReqQ.front().pchRcvBuf[i++] = (char)ity->mbaphdr.uchUnitIdentifier;
				g_MBSndRcvReqQ.front().pchRcvBuf[i++] = (char)ity->mbr.uchFunctionCode;
				for ( ; !p->m_bQuit && ity->mbr.chResponse && i-2 < (ity->mbaphdr.usLength-2); i++ )
				{
					g_MBSndRcvReqQ.front().pchRcvBuf[i] = (char)ity->mbr.chResponse[i-2];
				}
				*g_MBSndRcvReqQ.front().ppchEndRcv = g_MBSndRcvReqQ.front().pchRcvBuf + (ity->mbaphdr.usLength);
				bResult = ::PostMessage(g_MBSndRcvReqQ.front().hPage, g_MBSndRcvReqQ.front().nMsg, (WPARAM) 4, (LPARAM) 0);
				g_MBSndRcvReqQ.pop();
				::LeaveCriticalSection(&gRWDataCritSect);
				break;
			case 0x11:
				if ( !p->m_lpSolaID )
				{
					p->m_lpSolaID = (LPSOLAIDRESPONSE) new SOLAIDRESPONSE;
				}
				if ( !p->m_szBurnerName )
				{
					p->m_szBurnerName = (TCHAR*) new TCHAR[20];
				}
				if ( !p->m_szOSNumber )
				{
					p->m_szOSNumber = (TCHAR*) new TCHAR[16];
				}
				if ( p->m_lpSolaID && p->m_szBurnerName && p->m_szOSNumber )
				{
					p->m_lpSolaID->SolaAddr = ity->mbaphdr.uchUnitIdentifier;
					p->m_lpSolaID->FunctionCode = ity->mbr.uchFunctionCode;
					p->m_lpSolaID->ByteCount = ity->mbr.uscbRespLen;
					p->m_lpSolaID->SolaID = ity->mbr.chResponse[0];
					p->m_lpSolaID->RunIndicator = ity->mbr.chResponse[1];
					nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&ity->mbr.chResponse[2],SOLAOSNUMBERLEN,p->m_szOSNumber,SOLAOSNUMBERLEN);
					nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&ity->mbr.chResponse[18],SOLABURNERNAMELEN,p->m_szBurnerName,SOLABURNERNAMELEN);
					bResult = p->Set_Sola_Connected(true);
					bResult = ::PostMessage(p->m_hwndParent,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)p->m_lpSolaID);
				}
				break;
			case 0x91:
				break;
			default:
				break;
			}
			itx = p->m_lplReqMBMsgs->erase(itx);
			ity = p->m_lplRespMBMsgs->erase(ity);
		}
		if ( !bMatch )
		{
			if ( !p->m_lplReqMBMsgs->empty() )
			{
				it1 = p->m_lplReqMBMsgs->erase(p->m_lplReqMBMsgs->begin(),p->m_lplReqMBMsgs->end());
			}
			if ( !p->m_lplRespMBMsgs->empty() )
			{
				it2 = p->m_lplRespMBMsgs->erase(p->m_lplRespMBMsgs->begin(),p->m_lplRespMBMsgs->end());
			}
		}
		::LeaveCriticalSection(p->m_lpRespCritSect);
	}
	return bRtnResult;
}

DWORD WINAPI CSolaTCPComm::SolaPollThread(LPARAM lParam)
{
	CSolaTCPComm* p_this = (CSolaTCPComm*) lParam;
	LPMBMESSAGE lpmbm = NULL;
	HRESULT hRes;
	BOOL bResult;
	BOOL bReConnect = true;
	BOOL bSuccess = true;
	BOOL bConfigChange = false;
	BOOL bStatusChange = false;
	BOOL bMBSndRcvQmt= true;
	TCHAR szError[100];
	unsigned char MBMsg[512];
	char chReq[512];
	unsigned char* lpMBMsg;
	int nSendResult;
	int nResult;
	int i;
	int j;
	int k;
	int nEventCount;
	int cbLen;
	DWORD dwResult;
	DWORD dwWaitResult;
	HANDLE hEvents[2];
	HANDLE hTimerEvents[2];
	HANDLE hPageEvents[NUMPROPPAGES+1];
	CSolaMBMap::LPSOLAMBMAP lpSolaRequest;
	LRESULT lr_ID_Active_Page;
	HWND hwnd_curr_ps_page;
	short sRegCount;
	unsigned short usBit;
	unsigned short usTemp;
	unsigned long ulSecsSinceLastResp = 0;
	CNoticeDialog* pnd;
	TCHAR szTemp[MAX_LOADSTRING];
	LPSOLAALERT lpAlertRecord;
	LPSOLALOCKOUT lpLockoutRecord;
	LARGE_INTEGER liDueTime;
	liDueTime.QuadPart = -10000000LL;
	uint8_t u8_stn_id;

	std::list<MBMESSAGE>::iterator it1;
	std::list<MBRESPMESSAGE>::iterator it2;
	std::list<MBMESSAGE>::iterator itx;
	std::list<MBRESPMESSAGE>::iterator ity;
	std::list<CSolaMBMap*>::iterator it_reg_list;

	if ( !p_this->IsConnected() )
	{
		return 0;
	}
	pnd = NULL;
	HANDLE h1SecTimer = ::CreateWaitableTimer(NULL,true,NULL);
	if ( !h1SecTimer )
	{
		hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("Create timer failed, error# %ld"),::GetLastError());
		nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
		return 0;
	}
	// First send slave ID request to establish if Sola really connected to gateway
	// Note: save request, in request list, in host format first, then convert to network format for sending
	//       only need to convert values longer than 1 byte
	if ( !lpmbm )
	{
		lpmbm = (LPMBMESSAGE) new MBMESSAGE;
	}

	bResult = ::DuplicateHandle(::GetCurrentProcess(),p_this->m_hQuitEvent,::GetCurrentProcess(),&hEvents[0],DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),p_this->m_hResponseEvent,::GetCurrentProcess(),&hEvents[1],DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),p_this->m_hQuitEvent,::GetCurrentProcess(),&hTimerEvents[0],DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);
	bResult = ::DuplicateHandle(::GetCurrentProcess(),h1SecTimer,::GetCurrentProcess(),&hTimerEvents[1],DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);

	bResult = ::DuplicateHandle(::GetCurrentProcess(),p_this->m_hQuitEvent,GetCurrentProcess(),&hPageEvents[0],0,false,DUPLICATE_SAME_ACCESS);
	for ( j = 1; j < NUMPROPPAGES+1; j++ )
	{
		bResult = ::DuplicateHandle(::GetCurrentProcess(), g_hPageUpdEvents[j-1], GetCurrentProcess(), &hPageEvents[j], 0, false, DUPLICATE_SAME_ACCESS);
	}


/* Main loop */
	while ( !p_this->m_bQuit && bSuccess )
	{
		u8_stn_id = SOLAMBAddress;
		if (NULL != p_this->m_lpSolaID)
		{
			delete p_this->m_lpSolaID;
			p_this->m_lpSolaID = NULL;
		}
		if (NULL != p_this->m_szOSNumber)
		{
			delete[] p_this->m_szOSNumber;
			p_this->m_szOSNumber = NULL;
		}
		if (NULL != p_this->m_szBurnerName)
		{
			delete[] p_this->m_szBurnerName;
			p_this->m_szBurnerName = NULL;
		}
		if (NULL != p_this->m_szOEMID)
		{
			delete[] p_this->m_szOEMID;
			p_this->m_szOEMID = NULL;
		}
		::EnterCriticalSection(&gTimeCritSect);
		g_dwConnectTime = 0L;
		::LeaveCriticalSection(&gTimeCritSect);
		::EnterCriticalSection(&gCOMCritSect);
		g_dwTotalCRCErrors = 0L;
		g_dwTotalRcvd = g_dwTotalSent = 0L;
		::LeaveCriticalSection(&gCOMCritSect);
		if (!p_this->IsSolaConnected())
		{
			bResult = ::PostMessage(p_this->m_hwndParent,WM_APPSOLACONNECTING,(WPARAM)0,(LPARAM)NULL);
		}
/*		while ((TCP_Lantronix == p_this->Get_Gateway_Type()) && !p_this->m_bQuit && !p_this->IsSolaConnected())*/
		if ((TCP_Lantronix == p_this->Get_Gateway_Type()) && !p_this->m_bQuit && !p_this->IsSolaConnected())
		{
			lpmbm->lpMap = (PVOID)NULL;
			lpmbm->dwTimeInTicks = ::GetTickCount();
			lpmbm->rt = LPSolaIDReq;
			lpmbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
			lpmbm->mbaphdr.usLength = (u_short)0x0006;
			lpmbm->mbaphdr.uchUnitIdentifier = u8_stn_id;
			lpmbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
			lpmbm->mbr.uchFunctionCode = 0x11;
			lpmbm->mbr.usRegisterCnt = (u_short)0x0000;
			lpmbm->mbr.usStartAddr = 0;
			::EnterCriticalSection(p_this->m_lpReqCritSect);
			p_this->m_lplReqMBMsgs->push_back(*lpmbm);
			::LeaveCriticalSection(p_this->m_lpReqCritSect);
			lpmbm->mbaphdr.usTransactionIdentifier = ::htons(lpmbm->mbaphdr.usTransactionIdentifier);
			lpmbm->mbaphdr.usLength = ::htons(lpmbm->mbaphdr.usLength);
			lpmbm->mbaphdr.usProtocolIdentifier = ::htons(lpmbm->mbaphdr.usProtocolIdentifier);
			lpmbm->mbr.usRegisterCnt = ::htons(lpmbm->mbr.usRegisterCnt);
			lpmbm->mbr.usStartAddr = ::htons(lpmbm->mbr.usStartAddr);
			nSendResult = send(p_this->m_MBSocket,(char*)&(lpmbm->mbaphdr),sizeof(MBAPHEADER)+sizeof(MBREQUEST),0);
			if ( nSendResult == SOCKET_ERROR )
			{
				hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
				nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
			}
			if (SOCKET_ERROR != nSendResult)
			{
				::EnterCriticalSection(&gCOMCritSect);
				g_dwTotalSent += nSendResult;
				::LeaveCriticalSection(&gCOMCritSect);
			}
			dwWaitResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,5000);
			switch (dwWaitResult)
			{
			case WAIT_OBJECT_0 + 0:
				break;
			case WAIT_OBJECT_0 + 1:
				ulSecsSinceLastResp = 0;
				bResult = p_this->ProcessResponse(p_this);
				if ( !p_this->IsSolaConnected() )
				{
					dwWaitResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,5000);
				}
				break;
			case WAIT_TIMEOUT:
				ulSecsSinceLastResp =+ 5;
				break;
			case WAIT_FAILED:
				break;
			default:
				break;
			}
		} // End of while ( !TCPPROTONODE && !p_this->m_bQuit && !p_this->IsSolaConnected() )
		
/*		while ( ((TCP_Protonode == p_this->Get_Gateway_Type()) ||
			(TCP_Other == p_this->Get_Gateway_Type())) &&
			!p_this->m_bQuit && !p_this->IsSolaConnected() )*/
		if ( ((TCP_Protonode == p_this->Get_Gateway_Type()) ||
			(TCP_Other == p_this->Get_Gateway_Type())) &&
			!p_this->m_bQuit && !p_this->IsSolaConnected() )
		{
			dwResult = ReadSolaMap(
				(CSolaTCPComm*)p_this,
				(unsigned char const)u8_stn_id,
				pc_Dup_Burner_Name,
				lpmbm,
				LPInitIDreq,
				hEvents,
				sizeof(hEvents)/sizeof(HANDLE),
				ulSecsSinceLastResp,
				bReConnect);
			dwResult = ReadSolaMap(
				(CSolaTCPComm*)p_this,
				(unsigned char const)u8_stn_id,
				pc_Dup_OEMID,
				lpmbm,
				LPInitIDreq,
				hEvents,
				sizeof(hEvents)/sizeof(HANDLE),
				ulSecsSinceLastResp,
				bReConnect);
		} // End of while ( TCPPROTONODE && !p_this->m_bQuit && !p_this->IsSolaConnected() )
		
		if ( !p_this->m_bQuit && bReConnect && p_this->IsSolaConnected() )
		{
			bReConnect = false;
			bResult = ::PostMessage(p_this->m_hwndParent, WM_APPDATAUPDSTART, (WPARAM) 0, (LPARAM) 0);
			if (NULL == pnd)
			{
			  pnd = (CNoticeDialog*)new CNoticeDialog(p_this->m_hwndParent,g_hInst,szTitle);
			}
			dwResult = (DWORD)pnd->SetNoticeTxt(_T("Retrieving Sola data"));
			pnd->Start();
			dwResult = ReadSolaMaps(
				(CSolaTCPComm*)p_this,
				(unsigned char const)u8_stn_id,
				pcAllSolaMaps,
				lpmbm,
				LPHoldRegReq,
				hEvents,
				sizeof(hEvents)/sizeof(HANDLE),
				ulSecsSinceLastResp,
				bReConnect,
				pnd);
/* Get alert log */
			dwResult = Read_Alert_Log(
				p_this,
				u8_stn_id,
				pcAlertLog,
				lpmbm,
				LPAlertLogReq,
				hEvents,
				sizeof(hEvents)/sizeof(HANDLE),
				ulSecsSinceLastResp,
				bReConnect,
				pnd);
/* Get lockout log */
			dwResult = Read_Lockout_Log(
				p_this,
				u8_stn_id,
				pcLockoutLog,
				lpmbm,
				LPLockoutLogReq,
				hEvents,
				sizeof(hEvents)/sizeof(HANDLE),
				ulSecsSinceLastResp,
				bReConnect,
				pnd);
			bResult = ::PostMessage(p_this->m_hwndParent, WM_APPREADOK, (WPARAM) 0, (LPARAM) 0);
			pnd->Stop();
			delete pnd;
			pnd = NULL;
		}

		while ( !p_this->m_bQuit && p_this->IsSolaConnected() && bSuccess )
		{
			bResult = ::SetWaitableTimer(h1SecTimer,&liDueTime,0,NULL,NULL,false);
			dwResult = ::WaitForMultipleObjects(sizeof(hTimerEvents)/sizeof(HANDLE),hTimerEvents,false,INFINITE);
			if ( WAIT_FAILED == dwResult )
			{
				hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("Wait failed, error# %d"),::WSAGetLastError());
				nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
			}
			if ( 0 == dwResult - WAIT_OBJECT_0 )
			{
				break;
			}
			
			if (TCP_Lantronix == p_this->Get_Gateway_Type())
			{
				::EnterCriticalSection( &gRWDataCritSect );
				nEventCount = g_nActiveTrendPages + (g_nActiveStatusPages*bStatusChange)+(g_nActiveConfigPages*bConfigChange);
				for ( i = 0; !p_this->m_bQuit && i < NUMPROPPAGES; i++ )
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
// Always get dynamic trend data
				dwResult = ReadSolaMaps(
					(CSolaTCPComm*)p_this,
					(unsigned char const)u8_stn_id,
					pcTrendMaps,
					lpmbm,
					LPHoldRegReq,
					hEvents,
					sizeof(hEvents)/sizeof(HANDLE),
					ulSecsSinceLastResp,
					bReConnect,
					NULL);

// Check if there's a status update
				bStatusChange = false;
				usBit = 1;
				for ( i = 0; !p_this->m_bQuit && i < pcStatusChangeCodes->GetSize(); i++ )
				{
					usTemp = pcSystemStatus->GetLPMap(0)->sValue;
					if ( pcStatusChangeCodes->ItemBitMask(i) & pcSystemStatus->GetLPMap(0)->sValue )
					{
						bStatusChange = true;
						if ( pcStatusChangeCodes->SolaMBMap(i) != NULL )
						{
							lpSolaRequest = pcStatusChangeCodes->SolaMBMap(i)->GetLPMap(0);
							dwResult = ReadSolaMap(
								(CSolaTCPComm*)p_this,
								(unsigned char const)u8_stn_id,
								pcStatusChangeCodes->SolaMBMap(i),
								lpmbm,
								LPHoldRegReq,
								hEvents,
								sizeof(hEvents)/sizeof(HANDLE),
								ulSecsSinceLastResp,
								bReConnect);
						}
						if ( pcStatusChangeCodes->SolaAlertLog(i) != NULL )
						{
							dwResult = Read_Alert_Log(
								p_this,
								u8_stn_id,
								pcAlertLog,
								lpmbm,
								LPAlertLogReq,
								hEvents,
								sizeof(hEvents)/sizeof(HANDLE),
								ulSecsSinceLastResp,
								bReConnect,
								NULL);
						}
						if ( pcStatusChangeCodes->SolaLockoutLog(i) != NULL )
						{
							dwResult = Read_Lockout_Log(
								p_this,
								u8_stn_id,
								pcLockoutLog,
								lpmbm,
								LPLockoutLogReq,
								hEvents,
								sizeof(hEvents)/sizeof(HANDLE),
								ulSecsSinceLastResp,
								bReConnect,
								pnd);
						}
					}
					usBit <<= 1;
				}
// Check if there's a configuration update.
// System ID and Access has to handled separately because of string
// values.
				bConfigChange = false;
				usBit = 1;
				if ( usBit & pcSystemStatus->GetLPMap(1)->sValue )
				{
					bConfigChange = true;
					dwResult = ReadSolaMaps(
						(CSolaTCPComm*)p_this,
						(unsigned char const)u8_stn_id,
						pcSystemIDMaps,
						lpmbm,
						LPHoldRegReq,
						hEvents,
						sizeof(hEvents)/sizeof(HANDLE),
						ulSecsSinceLastResp,
						bReConnect,
						NULL);
				}
	
				for ( i = 0; !p_this->m_bQuit && i < pcConfigChangeCodes->GetSize() && pcSystemStatus->GetLPMap(1)->sValue; i++ )
				{
					if ( pcConfigChangeCodes->ItemBitMask(i) & pcSystemStatus->GetLPMap(1)->sValue )
					{
						bConfigChange = true;
						if ( pcConfigChangeCodes->SolaMBMap(i) != NULL )
						{
							lpSolaRequest = pcConfigChangeCodes->SolaMBMap(i)->GetLPMap(0);
							dwResult = ReadSolaMap(
								(CSolaTCPComm*)p_this,
								(unsigned char const)u8_stn_id,
								pcConfigChangeCodes->SolaMBMap(i),
								lpmbm,
								LPHoldRegReq,
								hEvents,
								sizeof(hEvents)/sizeof(HANDLE),
								ulSecsSinceLastResp,
								bReConnect);
						}
					}
					usBit <<= 1;
				}
			} /* End if (TCP_Protonode != p_this->Get_Gateway_Type()) */

			if ((TCP_Protonode == p_this->Get_Gateway_Type()) || (TCP_Other == p_this->Get_Gateway_Type()))
			{
				EnterCriticalSection( &gRWDataCritSect );
				hwnd_curr_ps_page = (HWND)SendMessage(g_hPropSheet,PSM_GETCURRENTPAGEHWND,(WPARAM)0,(LPARAM)0);
				i = SendMessage(g_hPropSheet,PSM_HWNDTOINDEX,(WPARAM)hwnd_curr_ps_page,(LPARAM)0);
				lr_ID_Active_Page = SendMessage(g_hPropSheet,PSM_INDEXTOID,(WPARAM)i,(LPARAM)0);
				nEventCount = g_nActiveTrendPages + (g_nActiveStatusPages*bStatusChange)+(g_nActiveConfigPages*bConfigChange);
				for ( i = 0; !p_this->m_bQuit && i < NUMPROPPAGES; i++ )
				{
					if ( g_lpPageDataEvents[i].typePage == TrendPage )
					{
						bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
					}
					if (g_lpPageDataEvents[i].typePage == StatusPage)
					{
						bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
					}
					if (g_lpPageDataEvents[i].typePage == ConfigPage)
					{
						bResult = ::SetEvent(g_lpPageDataEvents[i].hEvent);
					}
				}
				LeaveCriticalSection(&gRWDataCritSect);
#if 0
// Always get dynamic trend data
				dwResult = ReadSolaMaps(
					(CSolaTCPComm*)p_this,
					(unsigned char const)u8_stn_id,
					pcStatusMaps,
					lpmbm,
					LPHoldRegReq,
					hEvents,
					sizeof(hEvents)/sizeof(HANDLE),
					ulSecsSinceLastResp,
					bReConnect,
					NULL);
#endif
/* Only get register groups needed for the currently displayed page */
				if (bSuccess && !g_bQuit && p_this->IsSolaConnected())
				{
					EnterCriticalSection(&gRWDataCritSect);
					if (!p_Reg_Group_List->empty())
					{
						for (it_reg_list = p_Reg_Group_List->begin();it_reg_list != p_Reg_Group_List->end();it_reg_list++)
						{
							dwResult = ReadSolaMap(
								(CSolaTCPComm*)p_this,
								(unsigned char const)u8_stn_id,
								*it_reg_list,
								lpmbm,
								LPHoldRegReq,
								hEvents,
								sizeof(hEvents)/sizeof(HANDLE),
								ulSecsSinceLastResp,
								bReConnect);
							}
					}
					LeaveCriticalSection(&gRWDataCritSect);
				}
/* Get alert log */
				if ((lr_ID_Active_Page == (LRESULT)IDD_ALERTLOGDLG) && bSuccess && !g_bQuit && p_this->IsSolaConnected())
				{
						dwResult = Read_Alert_Log(
							p_this,
							u8_stn_id,
							pcAlertLog,
							lpmbm,
							LPAlertLogReq,
							hEvents,
							sizeof(hEvents)/sizeof(HANDLE),
							ulSecsSinceLastResp,
							bReConnect,
							NULL);
				}

/* Get Lockout history log */
				if ((lr_ID_Active_Page == (LRESULT)IDD_LOCKOUTLOGDLG) && bSuccess && !g_bQuit && p_this->IsSolaConnected())
				{
						dwResult = Read_Lockout_Log(
							p_this,
							u8_stn_id,
							pcLockoutLog,
							lpmbm,
							LPLockoutLogReq,
							hEvents,
							sizeof(hEvents)/sizeof(HANDLE),
							ulSecsSinceLastResp,
							bReConnect,
							pnd);
				}
			}
			::EnterCriticalSection(&gRWDataCritSect);
			bMBSndRcvQmt = g_MBSndRcvReqQ.empty();
			::LeaveCriticalSection(&gRWDataCritSect);
			while ( !p_this->m_bQuit && !bMBSndRcvQmt && p_this->IsSolaConnected() ) /* Check for data to be sent to Sola */
			{
				i = 0;
				::SecureZeroMemory((PVOID)MBMsg,(SIZE_T)sizeof(MBMsg));
				::EnterCriticalSection(&gRWDataCritSect);
				while ( !p_this->m_bQuit && *g_MBSndRcvReqQ.front().ppchToSnd < *g_MBSndRcvReqQ.front().ppchEndSnd && *g_MBSndRcvReqQ.front().ppchToSnd < g_MBSndRcvReqQ.front().pchSndBuf + g_MBSndRcvReqQ.front().nSndBufSize  )
				{
					MBMsg[i++] = *(*g_MBSndRcvReqQ.front().ppchToSnd)++; /* Copy message to local buffer */
				}
				::LeaveCriticalSection(&gRWDataCritSect);
				switch (MBMsg[1])
				{
				case 0x06: /* Write Single Register */
					sRegCount = (short) 0x01;
					lpmbm->lpMap = (PVOID)NULL;
					lpmbm->rt = WriteSingleRegReq;
					lpmbm->dwTimeInTicks = ::GetTickCount();
					lpmbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
					lpmbm->mbaphdr.usLength = (u_short)0x0006;
					lpmbm->mbaphdr.uchUnitIdentifier = MBMsg[0]; /* Target Modbus address */
					lpmbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
					lpmbm->mbr.uchFunctionCode = (unsigned char) MBMsg[1]; /* Command to write a single register */
					lpmbm->mbr.usStartAddr = (MBMsg[2]*256) + MBMsg[3]; /* Followed by the value to be written */
					lpmbm->mbr.usRegisterCnt = (MBMsg[4]*256) + MBMsg[5]; /* Followed by the registers' address */
#if TCPDEBUGGING
					nResult = ReqFormat(lpmbm,chReq,sizeof(chReq));
					if ( nResult )
					{
						::EnterCriticalSection(p_this->m_lpDbgFileCritSect);
						*(p_this->m_lpdbgfile) << chReq << "\n";
						::LeaveCriticalSection(p_this->m_lpDbgFileCritSect);
					}
#endif
					::EnterCriticalSection(p_this->m_lpReqCritSect);
					p_this->m_lplReqMBMsgs->push_back(*lpmbm);
					::LeaveCriticalSection(p_this->m_lpReqCritSect);
					lpmbm->mbaphdr.usTransactionIdentifier = ::htons(lpmbm->mbaphdr.usTransactionIdentifier);
					lpmbm->mbaphdr.usLength = ::htons(lpmbm->mbaphdr.usLength);
					lpmbm->mbaphdr.usProtocolIdentifier = ::htons(lpmbm->mbaphdr.usProtocolIdentifier);
					lpmbm->mbr.usRegisterCnt = ::htons(lpmbm->mbr.usRegisterCnt);
					lpmbm->mbr.usStartAddr = ::htons(lpmbm->mbr.usStartAddr);
					nSendResult = send(p_this->m_MBSocket,(char*)&(lpmbm->mbaphdr),sizeof(MBAPHEADER)+sizeof(MBREQUEST),0);
					if ( nSendResult == SOCKET_ERROR )
					{
						hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
						nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
					}
					if (SOCKET_ERROR != nSendResult)
					{
						::EnterCriticalSection(&gCOMCritSect);
						g_dwTotalSent += nSendResult;
						::LeaveCriticalSection(&gCOMCritSect);
					}
					dwWaitResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,5000);
					switch (dwWaitResult)
					{
					case WAIT_OBJECT_0 + 0:
						break;
					case WAIT_OBJECT_0 + 1:
						ulSecsSinceLastResp = 0;
						bResult = p_this->ProcessResponse(p_this);
						break;
					case WAIT_TIMEOUT:
						ulSecsSinceLastResp += 5;
						if ( ulSecsSinceLastResp > 15 )
						{
							bReConnect = true;
							p_this->Set_Sola_Connected(false);
						}
						break;
					case WAIT_FAILED:
						break;
					default:
						break;
					}
					break;
				case 0x10: /* Write Multiple Registers */
					lpmbm->lpMap = (PVOID)NULL;
					lpmbm->rt = WriteMultiRegReq;
					lpmbm->dwTimeInTicks = ::GetTickCount();
					lpmbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
					lpmbm->mbaphdr.usLength = (u_short)0x0007 + (u_short)MBMsg[6]; /* Length includes 1 byte for the byte count of the payload */
					lpmbm->mbaphdr.uchUnitIdentifier = MBMsg[0]; /* Target Modbus address */
					lpmbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
					lpmbm->mbr.uchFunctionCode = (unsigned char) MBMsg[1]; /* Command to write multiple registers */
					lpmbm->mbr.usStartAddr = (MBMsg[2]*256) + MBMsg[3]; /* Followed by the registers' address */
					lpmbm->mbr.usRegisterCnt = (MBMsg[4]*256) + MBMsg[5]; /* Followed by the # of reg's to be written which is 1 by definition */
					cbLen = (int)MBMsg[6];
#if TCPDEBUGGING
					nResult = ReqFormat(lpmbm,chReq,sizeof(chReq));
					if ( nResult )
					{
						::EnterCriticalSection(p_this->m_lpDbgFileCritSect);
						*(p_this->m_lpdbgfile) << chReq << "\n";
						::LeaveCriticalSection(p_this->m_lpDbgFileCritSect);
					}
#endif
					::EnterCriticalSection(p_this->m_lpReqCritSect);
					p_this->m_lplReqMBMsgs->push_back(*lpmbm);
					::LeaveCriticalSection(p_this->m_lpReqCritSect);
					lpMBMsg = (unsigned char*) new unsigned char[cbLen];
					for ( i = 0; i < cbLen && i  < sizeof(MBMsg) - 7; i++ )
					{
						*(lpMBMsg+i) = MBMsg[i+7]; /* Copy message data payload to buffer */
					}
					MBMsg[0] = (lpmbm->mbaphdr.usTransactionIdentifier >> 8) & (unsigned short)0x00ff;
					MBMsg[1] = lpmbm->mbaphdr.usTransactionIdentifier & (unsigned short)0x00ff;
					MBMsg[2] = (lpmbm->mbaphdr.usProtocolIdentifier >> 8) & (unsigned short)0x00ff;
					MBMsg[3] = lpmbm->mbaphdr.usProtocolIdentifier;
					MBMsg[4] = (lpmbm->mbaphdr.usLength >> 8) & (unsigned short)0x00ff;
					MBMsg[5] = lpmbm->mbaphdr.usLength & (unsigned short)0x00ff;
					MBMsg[6] = lpmbm->mbaphdr.uchUnitIdentifier;
					MBMsg[7] = lpmbm->mbr.uchFunctionCode;
					MBMsg[8] = (lpmbm->mbr.usStartAddr >> 8) & (unsigned short)0x00ff;
					MBMsg[9] = lpmbm->mbr.usStartAddr & (unsigned short)0x00ff;
					MBMsg[10] = (lpmbm->mbr.usRegisterCnt >> 8) & (unsigned short)0x00ff;
					MBMsg[11] = lpmbm->mbr.usRegisterCnt & (unsigned short)0x00ff;
					MBMsg[12] = (unsigned char)cbLen;
					for ( i = 0; i < cbLen && i  < sizeof(MBMsg) - 7; i++ )
					{
						MBMsg[13+i] = *(lpMBMsg+i); /* Copy message data payload to buffer */
					}
					cbLen += sizeof(MBAPHEADER) + sizeof(MBREQUEST) + 1; /* The "+ 1" is for the payload byte count */
					delete[] lpMBMsg;
					nSendResult = send(p_this->m_MBSocket,(char*)MBMsg,cbLen,0);
					if ( nSendResult == SOCKET_ERROR )
					{
						hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
						nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
					}
					if (SOCKET_ERROR != nSendResult)
					{
						::EnterCriticalSection(&gCOMCritSect);
						g_dwTotalSent += nSendResult;
						::LeaveCriticalSection(&gCOMCritSect);
					}
					dwWaitResult = ::WaitForMultipleObjects(sizeof(hEvents)/sizeof(HANDLE),hEvents,false,5000);
					switch (dwWaitResult)
					{
					case WAIT_OBJECT_0 + 0:
						break;
					case WAIT_OBJECT_0 + 1:
						ulSecsSinceLastResp = 0;
						bResult = p_this->ProcessResponse(p_this);
						break;
					case WAIT_TIMEOUT:
						ulSecsSinceLastResp += 5;
						if ( ulSecsSinceLastResp > 15 )
						{
							bReConnect = true;
							p_this->Set_Sola_Connected(false);
						}
						break;
					case WAIT_FAILED:
						break;
					default:
						break;
					}
					break;
				}
				::EnterCriticalSection(&gRWDataCritSect);
				bMBSndRcvQmt = g_MBSndRcvReqQ.empty();
				::LeaveCriticalSection(&gRWDataCritSect);
			}

			dwResult = ::WaitForSingleObject(g_hReSyncReqEvent,0);
			bSuccess = ((WAIT_FAILED != dwResult) && bSuccess);
			if (WAIT_OBJECT_0 == dwResult)
			{
				p_this->Set_Sola_Connected(false);
				bReConnect = true;
			}

		} /* End of 1st inner loop while ( !p_this->m_bQuit && p_this->IsSolaConnected() && bSuccess ) */
	
	} /* End of main loop while ( !p_this->m_bQuit && bSuccess ) */


	if ( lpmbm )
	{
		delete lpmbm;
	}
	for ( nResult = 0; nResult < sizeof(hEvents)/sizeof(HANDLE); nResult++ )
	{
		bResult = ::CloseHandle(hEvents[nResult]);
	}
	for ( nResult = 0; nResult < sizeof(hTimerEvents)/sizeof(HANDLE); nResult++ )
	{
		bResult = ::CloseHandle(hTimerEvents[nResult]);
	}
	for ( nResult = 0; nResult < sizeof(hPageEvents)/sizeof(HANDLE); nResult++ )
	{
		bResult = ::CloseHandle(hPageEvents[nResult]);
	}
	if ( !bSuccess )
	{
		p_this->Set_Sola_Connected(false);
		bResult = ::PostMessage(p_this->m_hwndParent,WM_APPSOLAPOLLABORT,(WPARAM)0,(LPARAM)0);
	}
	if ( h1SecTimer )
	{
		bResult = ::CancelWaitableTimer(h1SecTimer);
		bResult = ::CloseHandle(h1SecTimer);
	}
	return (DWORD)bSuccess;
}

DWORD CSolaTCPComm::ReadSolaMap(
	CSolaTCPComm *p_this,
	unsigned char const uc_addr,
	CSolaMBMap *lp_map,
	LPMBMESSAGE lp_mbm,
	MBReqType req_type,
	LPHANDLE ph_events,
	int i_ec,
	unsigned long &ulSecsSinceLastResp,
	BOOL &b_reconnect)
{
	char const *pch_Null = "\0";
	TCHAR szError[100];
	char chReq[512];
	BOOL bResult;
	DWORD dw_rc;
	CSolaMBMap::LPSOLAMBMAP lpSolaRequest;
	short sRegCount;
	int nSendResult;
	DWORD dwWaitResult;
	HRESULT hRes;
	int nResult;
	dw_rc = ERROR_SUCCESS;
#if _DEBUG
	std::wstring sss1;
	std::wstring sss2;
	sss1.assign(lp_map->GetParmName((int)0));
	sss2.assign(pcSystemIDInstallationData->GetParmName((int)0));
	if ((lp_map->GetStartRegAddr((int)0) == 0x00B9) || (lp_map->GetStartRegAddr((int)0) == 0x00B8) || (sss1 == sss2))
	{
		nResult = 0;
	}
#endif
	if (((TCP_Protonode == p_this->Get_Gateway_Type()) ||
		(TCP_Other == p_this->Get_Gateway_Type())) &&
		(CSolaMBMap::Stringvalue == lp_map->GetType((int)0)))
	{
			lp_map->SetStr((int)0,(unsigned char*)pch_Null,1);
			return dw_rc;
	}
	lpSolaRequest = lp_map->GetLPMap(0);
	sRegCount = (short)lp_map->GetRegRequestLen();
	lp_mbm->lpMap = (PVOID)lp_map;
	lp_mbm->rt = req_type;
	lp_mbm->dwTimeInTicks = ::GetTickCount();
	lp_mbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
	lp_mbm->mbaphdr.usLength = (u_short)0x0006;
	lp_mbm->mbaphdr.uchUnitIdentifier = uc_addr;
	lp_mbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
/*	lp_mbm->mbr.uchFunctionCode = lp_map->GetFuncCode((int)0);*/
	lp_mbm->mbr.uchFunctionCode = p_this->Get_MB_Function_Code();
	lp_mbm->mbr.usRegisterCnt = (u_short)sRegCount;
	lp_mbm->mbr.usStartAddr = lp_map->GetStartRegAddr((int)0);
#if TCPDEBUGGING
				nResult = ReqFormat(lp_mbm,chReq,sizeof(chReq));
				if ( nResult )
				{
					::EnterCriticalSection(p_this->m_lpDbgFileCritSect);
					*(p_this->m_lpdbgfile) << chReq << "\n";
					::LeaveCriticalSection(p_this->m_lpDbgFileCritSect);
				}
#endif
	::EnterCriticalSection(p_this->m_lpReqCritSect);
	p_this->m_lplReqMBMsgs->push_back(*lp_mbm);
	::LeaveCriticalSection(p_this->m_lpReqCritSect);
	lp_mbm->mbaphdr.usTransactionIdentifier = ::htons(lp_mbm->mbaphdr.usTransactionIdentifier);
	lp_mbm->mbaphdr.usLength = ::htons(lp_mbm->mbaphdr.usLength);
	lp_mbm->mbaphdr.usProtocolIdentifier = ::htons(lp_mbm->mbaphdr.usProtocolIdentifier);
	lp_mbm->mbr.usRegisterCnt = ::htons(lp_mbm->mbr.usRegisterCnt);
	lp_mbm->mbr.usStartAddr = ::htons(lp_mbm->mbr.usStartAddr);
	nSendResult = send(p_this->m_MBSocket,(char*)&(lp_mbm->mbaphdr),sizeof(MBAPHEADER)+sizeof(MBREQUEST),0);
	if ( nSendResult == SOCKET_ERROR )
	{
		dw_rc = WSAGetLastError();
		hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
		nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
	}
	if (SOCKET_ERROR != nSendResult)
	{
		::EnterCriticalSection(&gCOMCritSect);
		g_dwTotalSent += nSendResult;
		::LeaveCriticalSection(&gCOMCritSect);
	}
	dwWaitResult = ::WaitForMultipleObjects(i_ec,ph_events,false,5000);
	switch (dwWaitResult)
	{
	case WAIT_OBJECT_0 + 0:
		break;
	case WAIT_OBJECT_0 + 1:
		ulSecsSinceLastResp = 0;
		bResult = p_this->ProcessResponse(p_this);
		break;
	case WAIT_TIMEOUT:
		ulSecsSinceLastResp += 5;
		if ( ulSecsSinceLastResp > 15 )
		{
			b_reconnect = true;
			p_this->Set_Sola_Connected(false);
		}
		break;
	case WAIT_FAILED:
		break;
	default:
		break;
	}
	return dw_rc;
}

DWORD CSolaTCPComm::ReadSolaMaps(
	CSolaTCPComm* p_this,
	unsigned char const uc_addr,
	CSolaMBMaps *lp_maps,
	LPMBMESSAGE lp_mbm,
	MBReqType req_type,
	LPHANDLE ph_events,
	int i_ec,
	unsigned long &ulSecsSinceLastResp,
	BOOL &b_reconnect,
	CNoticeDialog *p_nd)
{
	DWORD dw_rc;
	int i_i;
#if _DEBUG
	int i_size = lp_maps->GetSize();
#endif
	for (i_i = 0; i_i < lp_maps->GetSize(); i_i++)
	{
		if (NULL != p_nd)
		{
			dw_rc = (DWORD)p_nd->SetNoticeTxt((TCHAR*)lp_maps->GetLPMap(i_i)->GetParmName((int)0));
		}
		dw_rc = ReadSolaMap(
			(CSolaTCPComm*)p_this,
			(unsigned char const)uc_addr,
			lp_maps->GetLPMap((int)i_i),
			lp_mbm,
			req_type,
			ph_events,
			i_ec,
			ulSecsSinceLastResp,
			b_reconnect);
	}

	return dw_rc;
}

DWORD CSolaTCPComm::Read_Alert_Log(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaAlert *lp_map,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect,
		CNoticeDialog *p_nd)
{
	TCHAR szTemp[100];
	TCHAR szError[100];
	BOOL bResult;
	DWORD dw_rc;
	int i_i;
	int i_rc;
	LPSOLAALERT lpAlertRecord;
	short sRegCount;
	int nSendResult;
	DWORD dwWaitResult;
	HRESULT hRes;
	int nResult;
	dw_rc = ERROR_SUCCESS;
	for (i_i = 0; !p_this->m_bQuit && i_i < pcAlertLog->GetSize(); i_i++)
	{
		if (NULL != p_nd)
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("Retrieving Alert record %d"), i_i+1);
			i_rc = p_nd->SetNoticeTxt(szTemp);
		}
		lpAlertRecord = pcAlertLog->GetLPMap(i_i);
		lp_mbm->lpMap = (PVOID)lpAlertRecord;
		lp_mbm->rt = LPAlertLogReq;
		lp_mbm->dwTimeInTicks = ::GetTickCount();
		lp_mbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
		lp_mbm->mbaphdr.usLength = (u_short)0x0006;
		lp_mbm->mbaphdr.uchUnitIdentifier = ucMBAddr;
		lp_mbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
/*		lp_mbm->mbr.uchFunctionCode = lpAlertRecord->uchFuncCode;*/
		lp_mbm->mbr.uchFunctionCode = p_this->Get_MB_Function_Code();
		lp_mbm->mbr.usRegisterCnt = lpAlertRecord->usRegCount;
		lp_mbm->mbr.usStartAddr = lpAlertRecord->usStartRegAddr;
		::EnterCriticalSection(p_this->m_lpReqCritSect);
		p_this->m_lplReqMBMsgs->push_back(*lp_mbm);
		::LeaveCriticalSection(p_this->m_lpReqCritSect);
		lp_mbm->mbaphdr.usTransactionIdentifier = ::htons(lp_mbm->mbaphdr.usTransactionIdentifier);
		lp_mbm->mbaphdr.usLength = ::htons(lp_mbm->mbaphdr.usLength);
		lp_mbm->mbaphdr.usProtocolIdentifier = ::htons(lp_mbm->mbaphdr.usProtocolIdentifier);
		lp_mbm->mbr.usRegisterCnt = ::htons(lp_mbm->mbr.usRegisterCnt);
		lp_mbm->mbr.usStartAddr = ::htons(lp_mbm->mbr.usStartAddr);
		nSendResult = send(p_this->m_MBSocket,(char*)&(lp_mbm->mbaphdr),sizeof(MBAPHEADER)+sizeof(MBREQUEST),0);
		if ( nSendResult == SOCKET_ERROR )
		{
			hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
			nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
		}
		if (SOCKET_ERROR != nSendResult)
		{
			::EnterCriticalSection(&gCOMCritSect);
			g_dwTotalSent += nSendResult;
			::LeaveCriticalSection(&gCOMCritSect);
		}
		dwWaitResult = ::WaitForMultipleObjects(i_ec,ph_events,false,5000);
		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0 + 0:
			break;
		case WAIT_OBJECT_0 + 1:
			ulSecsSinceLastResp = 0;
			bResult = p_this->ProcessResponse(p_this);
			break;
		case WAIT_TIMEOUT:
			ulSecsSinceLastResp += 5;
			if ( ulSecsSinceLastResp > 15 )
			{
				b_reconnect = true;
				bResult = p_this->Set_Sola_Connected(false);
			}
			break;
		case WAIT_FAILED:
			break;
		default:
			break;
		}
	}
	return dw_rc;
}

DWORD CSolaTCPComm::Read_Lockout_Log(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaLockout *lp_map,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect,
		CNoticeDialog *p_nd)
{
	TCHAR szTemp[100];
	TCHAR szError[100];
	BOOL bResult;
	DWORD dw_rc;
	int i_i;
	int i_rc;
	LPSOLALOCKOUT lpLockoutRecord;
	short sRegCount;
	int nSendResult;
	DWORD dwWaitResult;
	HRESULT hRes;
	int nResult;
	dw_rc = ERROR_SUCCESS;
	for (i_i = 0; !p_this->m_bQuit && i_i < lp_map->GetSize(); i_i++)
	{
		if (NULL != p_nd)
		{
			hRes = ::StringCchPrintf(szTemp,sizeof(szTemp)/sizeof(TCHAR),_T("Retrieving Lockout record %d"), i_i+1);
			dw_rc = (DWORD)p_nd->SetNoticeTxt(szTemp);
		}
		lpLockoutRecord = pcLockoutLog->GetLPMap(i_i);
		lp_mbm->lpMap = (PVOID)lpLockoutRecord;
		lp_mbm->rt = LPLockoutLogReq;
		lp_mbm->dwTimeInTicks = ::GetTickCount();
		lp_mbm->mbaphdr.usTransactionIdentifier = ++(p_this->m_usti);
		lp_mbm->mbaphdr.usLength = (u_short)0x0006;
		lp_mbm->mbaphdr.uchUnitIdentifier = ucMBAddr;
		lp_mbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
/*		lp_mbm->mbr.uchFunctionCode = lpLockoutRecord->uchFuncCode;*/
		lp_mbm->mbr.uchFunctionCode = p_this->Get_MB_Function_Code();
		lp_mbm->mbr.usRegisterCnt = lpLockoutRecord->usRegCount;
		lp_mbm->mbr.usStartAddr = lpLockoutRecord->usStartRegAddr;
		::EnterCriticalSection(p_this->m_lpReqCritSect);
		p_this->m_lplReqMBMsgs->push_back(*lp_mbm);
		::LeaveCriticalSection(p_this->m_lpReqCritSect);
		lp_mbm->mbaphdr.usTransactionIdentifier = ::htons(lp_mbm->mbaphdr.usTransactionIdentifier);
		lp_mbm->mbaphdr.usLength = ::htons(lp_mbm->mbaphdr.usLength);
		lp_mbm->mbaphdr.usProtocolIdentifier = ::htons(lp_mbm->mbaphdr.usProtocolIdentifier);
		lp_mbm->mbr.usRegisterCnt = ::htons(lp_mbm->mbr.usRegisterCnt);
		lp_mbm->mbr.usStartAddr = ::htons(lp_mbm->mbr.usStartAddr);
		nSendResult = send(p_this->m_MBSocket,(char*)&(lp_mbm->mbaphdr),sizeof(MBAPHEADER)+sizeof(MBREQUEST),0);
		if ( nSendResult == SOCKET_ERROR )
		{
			hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
			nResult = ::MessageBox(p_this->m_hwndParent,szError,szTitle,MB_OK);
		}
		if (SOCKET_ERROR != nSendResult)
		{
			::EnterCriticalSection(&gCOMCritSect);
			g_dwTotalSent += nSendResult;
			::LeaveCriticalSection(&gCOMCritSect);
		}
		dwWaitResult = ::WaitForMultipleObjects(i_ec,ph_events,false,5000);
		switch (dwWaitResult)
		{
		case WAIT_OBJECT_0 + 0:
			break;
		case WAIT_OBJECT_0 + 1:
			ulSecsSinceLastResp = 0;
			bResult = p_this->ProcessResponse(p_this);
			break;
		case WAIT_TIMEOUT:
			ulSecsSinceLastResp += 5;
			if ( ulSecsSinceLastResp > 15 )
			{
				b_reconnect = true;
				p_this->Set_Sola_Connected(false);
			}
			break;
		case WAIT_FAILED:
			break;
		default:
			break;
		}
	}
	return dw_rc;
}