#include "StdAfx.h"
#include "SolaTCPComm.h"

#if TCPDEBUGGING
int CSolaTCPComm::ReqFormat(LPMBMESSAGE lp,char *buf,int cbbufsiz)
{
	int len = 0;
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf,cbbufsiz,"Request: %08x ", lp->lpSolaReq);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%05d ",lp->mbaphdr.usTransactionIdentifier);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",lp->mbr.uchFunctionCode);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",lp->mbr.usRegisterCnt);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"0x%04x",lp->mbr.usStartAddr);
	}
	return strlen(buf);
}

int CSolaTCPComm::RespFormat(LPMBRESPMESSAGE lp,char* buf,int cbbufsiz)
{
	int len = 0;
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf,cbbufsiz,"Response:         %05d ", lp->mbaphdr.usTransactionIdentifier);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",lp->mbr.uchFunctionCode);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",lp->mbr.uscbRespLen);
	}
	len = strlen(buf);
	if ( cbbufsiz-(len+lp->mbr.uscbRespLen) > 0 )
	{
		for ( int i = 0; i < lp->mbr.uscbRespLen; i++ )
		{
			sprintf_s(buf+len,cbbufsiz-len,"0x%02x ",(unsigned char)lp->mbr.chResponse[i]);
			len = strlen(buf);
		}
	}
	return strlen(buf);
}

int CSolaTCPComm::RespFormat(std::list<MBRESPMESSAGE>::iterator ity,char* buf,int cbbufsiz)
{
	int len = 0;
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf,cbbufsiz,"Response:         %05d ", ity->mbaphdr.usTransactionIdentifier);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",ity->mbr.uchFunctionCode);
	}
	len = strlen(buf);
	if ( cbbufsiz-len > 0 )
	{
		sprintf_s(buf+len,cbbufsiz-len,"%d ",ity->mbr.uscbRespLen);
	}
	len = strlen(buf);
	if ( cbbufsiz-(len+ity->mbr.uscbRespLen) > 0 )
	{
		for ( int i = 0; i < ity->mbr.uscbRespLen; i++ )
		{
			sprintf_s(buf+len,cbbufsiz-len,"0x%02x ",(unsigned char)ity->mbr.chResponse[i]);
			len = strlen(buf);
		}
	}
	return strlen(buf);
}
#endif

CSolaTCPComm::CSolaTCPComm(void)
{
	m_lpReqCritSect = NULL;
	m_lpRespCritSect = NULL;
#if TCPDEBUGGING
	m_lpDbgFileCritSect = NULL;
#endif
	m_bQuit = false;
	m_hQuitEvent = NULL;
	m_hTimerEvent = NULL;
	m_hResponseEvent = NULL;
	m_bConnected = false;
	m_bSolaConnected = false;
	this->m_hTCPSocketThread = NULL;
	this->m_hdupTCPSocketThread = NULL;
	this->m_hwndParent = NULL;
	this->m_hwndSolaTCPSocketWnd = NULL;
	this->m_MBSocket = INVALID_SOCKET;
	this->m_usti = 0;
	this->m_szBurnerName = NULL;
	this->m_szOSNumber = NULL;
	m_szOEMID = NULL;
	this->m_lpSolaID = NULL;
	this->m_lplReqMBMsgs = NULL;
	this->m_lplRespMBMsgs = NULL;
	this->m_gwtype = TCP_Undefined;
	this->m_ui8fc = 0;
}

CSolaTCPComm::CSolaTCPComm(HWND hwndParent,enum TCP_Gateway_Type gw):m_hwndParent(hwndParent),m_gwtype(gw)
{
	m_lpReqCritSect = NULL;
	m_lpRespCritSect = NULL;
	m_bQuit = false;
	m_hQuitEvent = NULL;
	m_hTimerEvent = NULL;
	m_hResponseEvent = NULL;
	m_bConnected = false;
	m_bSolaConnected = false;
	this->m_hTCPSocketThread = NULL;
	this->m_hdupTCPSocketThread = NULL;
	this->m_hwndSolaTCPSocketWnd = NULL;
	this->m_MBSocket = INVALID_SOCKET;
	this->m_usti = 0;
	this->m_lpSolaID = NULL;
	this->m_szBurnerName = NULL;
	m_szOEMID = NULL;
	this->m_szOSNumber = NULL;
	this->m_lplReqMBMsgs = NULL;
	this->m_lplRespMBMsgs = NULL;
	switch (m_gwtype)
	{
	case TCP_Protonode:
		m_ui8fc = 4;
		break;
	case TCP_Lantronix:
		m_ui8fc = 3;
		break;
	case TCP_Other:
		m_ui8fc = 3;
		break;
	case TCP_Undefined:
		m_ui8fc = 0;
		break;
	default:
		m_ui8fc = 3;
		break;
	}
#if TCPDEBUGGING
	m_lpDbgFileCritSect = (LPCRITICAL_SECTION) new CRITICAL_SECTION;
	::InitializeCriticalSection(m_lpDbgFileCritSect);
	this->m_lpdbgfile = (ofstream*) new ofstream;
	m_lpdbgfile->open(_T("tcpdebug.txt"));
#endif
}
CSolaTCPComm::~CSolaTCPComm(void)
{
	BOOL bResult;
	DWORD dwResult;
	int nResult;
	m_bQuit = true;
	bResult = ::SetEvent(m_hQuitEvent);
	if ( this->m_hSolaPollThread )
	{
		dwResult = ::WaitForSingleObject(this->m_hSolaPollThread,5000);
		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			nResult = ::MessageBox(m_hwndParent,_T("TCP polling thread timeout"),szTitle,MB_OK);
			break;
		}
	}
	if ( this->m_hwndSolaTCPSocketWnd )
	{
		bResult = ::PostMessage(this->m_hwndSolaTCPSocketWnd,WM_APPUSERQUIT,(WPARAM)0,(LPARAM)0);
		dwResult = ::WaitForSingleObject(this->m_hTCPSocketThread,5000);
		switch (dwResult)
		{
		case WAIT_OBJECT_0:
			break;
		case WAIT_TIMEOUT:
			nResult = ::MessageBox(m_hwndParent,_T("Socket thread timeout"),szTitle,MB_OK);
			break;
		}
	}
	if ( m_szOSNumber )
	{
		delete[] m_szOSNumber;
	}
	if ( m_szBurnerName )
	{
		delete[] m_szBurnerName;
	}
	if (m_szOEMID)
	{
		delete[] m_szOEMID;
		m_szOEMID = NULL;
	}
	if ( m_lpSolaID )
	{
		delete m_lpSolaID;
	}
	if ( m_lplReqMBMsgs )
	{
		while ( !m_lplReqMBMsgs->empty() )
		{
			m_lplReqMBMsgs->pop_back();
		}
		delete m_lplReqMBMsgs;
		m_lplReqMBMsgs = NULL;
	}
	if ( m_lplRespMBMsgs )
	{
		for ( m_itRespMBMsgs = m_lplRespMBMsgs->begin(); m_itRespMBMsgs != m_lplRespMBMsgs->end(); (m_itRespMBMsgs)++ )
		{
			if ( m_itRespMBMsgs->mbr.chResponse )
			{
				delete m_itRespMBMsgs->mbr.chResponse;
			}
		}
		while ( !m_lplRespMBMsgs->empty() )
		{
			m_lplRespMBMsgs->pop_back();
		}
		delete m_lplRespMBMsgs;
		m_lplRespMBMsgs = NULL;
	}

	if ( m_MBSocket != INVALID_SOCKET )
	{
		nResult = closesocket(m_MBSocket); //Shut down socket
	}
	if ( m_hQuitEvent )
	{
		bResult = ::CloseHandle(m_hQuitEvent);
	}
	if ( m_hResponseEvent )
	{
		bResult = ::CloseHandle(m_hResponseEvent);
	}
	if ( m_hTimerEvent )
	{
		bResult = ::CloseHandle(m_hTimerEvent);
	}
	if ( m_lpReqCritSect )
	{
		::DeleteCriticalSection(m_lpReqCritSect);
		m_lpReqCritSect = NULL;
	}
	if ( m_lpRespCritSect )
	{
		::DeleteCriticalSection(m_lpRespCritSect);
		m_lpRespCritSect = NULL;
	}
	m_bConnected = false;
	m_bSolaConnected = false;
#if TCPDEBUGGING
	if ( m_lpdbgfile->is_open() )
	{
		::EnterCriticalSection(m_lpDbgFileCritSect);
		m_lpdbgfile->close();
		::LeaveCriticalSection(m_lpDbgFileCritSect);
	}
	::DeleteCriticalSection(m_lpDbgFileCritSect);
#endif
}

BOOL CSolaTCPComm::IsConnected()
{
	return this->m_bConnected;
}

BOOL CSolaTCPComm::IsSolaConnected()
{
	return this->m_bSolaConnected;
}

BOOL CSolaTCPComm::Set_Sola_Connected(BOOL b_connect_status)
{
	return (this->m_bSolaConnected = bSolaConnected = b_connect_status);
}

int CSolaTCPComm::CreateSocket(addrinfo* lpAddrInfo)
{
	DWORD dwResult;
	BOOL bResult;
	if ( IsConnected() )
	{
		return true;
	}

	this->m_lpAddrInfo = lpAddrInfo;
	if ( !IsConnected() && m_hTCPSocketThread && (m_hwndSolaTCPSocketWnd != NULL) )
	{
		bResult = ::PostMessage(m_hwndSolaTCPSocketWnd,WM_APPCREATEANDCONNECT,(WPARAM)0,(LPARAM)0);
		return true;
	}

	// Create socket thread and window for async socket operations
	m_hTCPSocketThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)TCPSocketThread,(LPVOID)this,CREATE_SUSPENDED,&m_dwTCPSocketThreadID);
	if ( m_hTCPSocketThread )
	{
		bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hTCPSocketThread,::GetCurrentProcess(),&m_hdupTCPSocketThread,DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);
		dwResult = ::ResumeThread(m_hTCPSocketThread);
	}
	else
	{
		return 0;
	}
	return true;
}

DWORD WINAPI CSolaTCPComm::TCPSocketThread(LPARAM lParam)
{
	CSolaTCPComm* pThis = (CSolaTCPComm*) lParam;
	const TCHAR* lpszTCPSocketWnd = {_T("SolaTCPSocketWindow")};
	WNDCLASSEX wcex;
	MSG msg;
	ATOM atomResult;
	BOOL bResult;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= TCPSocketWndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= g_hInst;
	wcex.hIcon			= NULL;
	wcex.hCursor		= NULL;
	wcex.hbrBackground	= NULL;
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= lpszTCPSocketWnd;
	wcex.hIconSm		= NULL;

	if ( !(atomResult = RegisterClassEx(&wcex)) )
	{
		return (DWORD)false;
	}

	pThis->m_hwndSolaTCPSocketWnd = CreateWindowEx(0,lpszTCPSocketWnd,NULL,WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,0,CW_USEDEFAULT,0,HWND_MESSAGE,NULL,g_hInst,(LPVOID)pThis);

   if (!pThis->m_hwndSolaTCPSocketWnd)
   {
      return (DWORD)FALSE;
   }

   ShowWindow(pThis->m_hwndSolaTCPSocketWnd, SW_HIDE);
   UpdateWindow(pThis->m_hwndSolaTCPSocketWnd);

   bResult = ::PostMessage(pThis->m_hwndSolaTCPSocketWnd,WM_APPCREATEANDCONNECT,(WPARAM)0,(LPARAM)0);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, NULL, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	
	bResult = ::UnregisterClass(lpszTCPSocketWnd,g_hInst);
//	nResult = WSACleanup();

	return (DWORD) msg.wParam;
}

LRESULT CALLBACK CSolaTCPComm::TCPSocketWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static BOOL bSuccess;
	BOOL bResult;
	int nResult;
	int cbRecvResult;
	int nWSAError;
	int nBytesRead;
	static long lcbRecvTotal;
	int i;
	DWORD dwResult;
	LPCREATESTRUCT lpcs;
	static CSolaTCPComm* pThis;
	TCHAR szError[100];
	unsigned char recvBuf[512];
	HRESULT hRes;
	static LPMBMESSAGE lpmbm;
	static int nSocketConnect;
	int nConnectResult;
	static LPMBAPHEADER lpmbapHdr;
	static LPMBRESPMESSAGE lpMBRespMsg;
	static UINT_PTR uipTimer1;
	static unsigned long ulSecsSinceLastResp;

	switch (message)
	{
	case WM_CREATE:
		lcbRecvTotal = 0;
		bSuccess = true;
		lpmbm = NULL;
		lpcs = (LPCREATESTRUCT)lParam;
		pThis = (CSolaTCPComm*) lpcs->lpCreateParams;
		lpmbapHdr = (LPMBAPHEADER) new MBAPHEADER;
		lpMBRespMsg = (LPMBRESPMESSAGE) new MBRESPMESSAGE;
		ulSecsSinceLastResp = 0L;
		uipTimer1 = ::SetTimer(hWnd,IDT_TIMER1,5000,(TIMERPROC)NULL);
		break;
	case WM_TIMER:
		ulSecsSinceLastResp += 5L;
		bResult = ::SetEvent(pThis->m_hTimerEvent);
		break;
	case WM_APPCREATEANDCONNECT:
		for ( addrinfo* ptr = pThis->m_lpAddrInfo; ptr != NULL; ptr = ptr->ai_next )
		{
			// Create a SOCKET for connecting to server
			pThis->m_MBSocket = socket(ptr->ai_family,ptr->ai_socktype,ptr->ai_protocol);
			if ( pThis->m_MBSocket == INVALID_SOCKET )
			{
				return 0;
			}
			nResult = ::WSAAsyncSelect(pThis->m_MBSocket,pThis->m_hwndSolaTCPSocketWnd,WM_APPMBSOCKETEVENT,FD_READ|FD_CONNECT|FD_CLOSE);
			if ( nResult )
			{
				closesocket(pThis->m_MBSocket);
				pThis->m_MBSocket = INVALID_SOCKET;
				return 0;
			}
			// Connect to server.
			nConnectResult = connect(pThis->m_MBSocket,ptr->ai_addr,(int)ptr->ai_addrlen);
			if (nConnectResult)
			{
				if ( ::WSAGetLastError() != WSAEWOULDBLOCK )
				{
					closesocket(pThis->m_MBSocket);
					pThis->m_MBSocket = INVALID_SOCKET;
					continue;
				}
			}
			break;
		}
		freeaddrinfo(pThis->m_lpAddrInfo);
		break;
	case WM_APPMBSOCKETEVENT:
		if ( WSAGETSELECTEVENT(lParam) == FD_CLOSE )
		{
			hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("Connection closed %d"),::WSAGetLastError());
			nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
			closesocket(pThis->m_MBSocket);
			pThis->m_MBSocket = INVALID_SOCKET;
			nSocketConnect = 0;
			pThis->m_bConnected = false;
			pThis->Set_Sola_Connected(false);
		}
		if ( WSAGETSELECTEVENT(lParam) == FD_READ )
		{
			cbRecvResult = ::recv(pThis->m_MBSocket,(char*)recvBuf,sizeof(recvBuf)/sizeof(char),0);
			if ( cbRecvResult == SOCKET_ERROR )
			{
				nWSAError = ::WSAGetLastError();
				hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("recv() failed with error %d"),nWSAError);
				nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
				switch (nWSAError)
				{
				case WSAETIMEDOUT:
					break;
				default:
					nResult = ::MessageBox(pThis->m_hwndParent,_T("socket error, closing socket"),szTitle,MB_OK);
					closesocket(pThis->m_MBSocket);
					pThis->m_MBSocket = INVALID_SOCKET;
					nSocketConnect = 0;
					pThis->m_bConnected = false;
//					DestroyWindow(hWnd);
					break;
				}
				return 0;
			}
			if (SOCKET_ERROR != cbRecvResult)
			{
				::EnterCriticalSection(&gCOMCritSect);
				g_dwTotalRcvd += cbRecvResult;
				::LeaveCriticalSection(&gCOMCritSect);
			}
			lcbRecvTotal += cbRecvResult;
			nResult = sizeof(MBAPHEADER);
			if ( cbRecvResult < sizeof(MBAPHEADER) )
			{
				// Invalid packet
				return 0;
			}
			nBytesRead = 0;
			while ( !pThis->m_bQuit && (nBytesRead < cbRecvResult) )
			{
				::CopyMemory(lpMBRespMsg,recvBuf,sizeof(MBAPHEADER));
				lpMBRespMsg->mbaphdr.usTransactionIdentifier = ::ntohs(lpMBRespMsg->mbaphdr.usTransactionIdentifier);
				lpMBRespMsg->mbaphdr.usProtocolIdentifier = ::ntohs(lpMBRespMsg->mbaphdr.usProtocolIdentifier);
				lpMBRespMsg->mbaphdr.usLength = ::ntohs(lpMBRespMsg->mbaphdr.usLength);
				nBytesRead += sizeof(MBAPHEADER) + lpMBRespMsg->mbaphdr.usLength - 1;
				lpMBRespMsg->mbr.uchFunctionCode = recvBuf[sizeof(MBAPHEADER)];
				lpMBRespMsg->mbr.uscbRespLen = recvBuf[sizeof(MBAPHEADER)+1];
//				if ( lpMBRespMsg->mbr.uscbRespLen )
				lpMBRespMsg->mbr.chResponse = NULL;
				if ( lpMBRespMsg->mbaphdr.usLength - 2 )
				{
					lpMBRespMsg->mbr.chResponse = (char*) new char[lpMBRespMsg->mbaphdr.usLength-2];
					for ( i = 0; i < (lpMBRespMsg->mbaphdr.usLength-2); i++ )
					{
						lpMBRespMsg->mbr.chResponse[i] = recvBuf[sizeof(MBAPHEADER)+2+i];
					}
				}
#if TCPDEBUGGING
				nResult = RespFormat(lpMBRespMsg,chResp,sizeof(chResp));
				::EnterCriticalSection(pThis->m_lpDbgFileCritSect);
				if ( nResult && pThis->m_lpdbgfile->is_open() )
				{
					*pThis->m_lpdbgfile << chResp << "\n";
				}
				::LeaveCriticalSection(pThis->m_lpDbgFileCritSect);
#endif
				::EnterCriticalSection(pThis->m_lpRespCritSect);
				pThis->m_lplRespMBMsgs->push_back(*lpMBRespMsg);
				::LeaveCriticalSection(pThis->m_lpRespCritSect);
				ulSecsSinceLastResp = 0L;
			}
//			if ( !pThis->m_lplRespMBMsgs->empty() && pThis->m_lplReqMBMsgs->empty() )
//			{
//				nResult = ::MessageBox(pThis->m_hwndParent,_T("Unexpected response"),szTitle,MB_OK);
//			}
			bResult = ::SetEvent(pThis->m_hResponseEvent);
#if 0
			::EnterCriticalSection(pThis->m_lpRespCritSect);
			if ( !pThis->m_lplRespMBMsgs->empty() && pThis->m_lplReqMBMsgs->empty() )
			{
				nResult = ::MessageBox(pThis->m_hwndParent,_T("Unexpected response"),szTitle,MB_OK);
//				return 0;
			}
			if ( !pThis->m_lplRespMBMsgs->empty() && !pThis->m_lplReqMBMsgs->empty() )
			{
				for( pThis->m_itReqMBMsgs = pThis->m_lplReqMBMsgs->begin(); pThis->m_itReqMBMsgs != pThis->m_lplReqMBMsgs->end(); (pThis->m_itReqMBMsgs)++ )
				{
					for( pThis->m_itRespMBMsgs = pThis->m_lplRespMBMsgs->begin(); pThis->m_itRespMBMsgs != pThis->m_lplRespMBMsgs->end(); (pThis->m_itRespMBMsgs)++ )
					{
						if ( pThis->m_itReqMBMsgs->mbaphdr.uchUnitIdentifier == pThis->m_itRespMBMsgs->mbaphdr.uchUnitIdentifier &&
							pThis->m_itReqMBMsgs->mbaphdr.usTransactionIdentifier == pThis->m_itRespMBMsgs->mbaphdr.usTransactionIdentifier )
						{
							switch (pThis->m_itRespMBMsgs->mbr.uchFunctionCode)
							{
							case 0x11:
								if ( !pThis->m_lpSolaID )
								{
									pThis->m_lpSolaID = (LPSOLAIDRESPONSE) new SOLAIDRESPONSE;
									pThis->m_szBurnerName = (TCHAR*) new TCHAR[20];
									pThis->m_szOSNumber = (TCHAR*) new TCHAR[16];
								}
								if ( pThis->m_lpSolaID )
								{
									pThis->m_lpSolaID->SolaAddr = pThis->m_itRespMBMsgs->mbaphdr.uchUnitIdentifier;
									pThis->m_lpSolaID->FunctionCode = pThis->m_itRespMBMsgs->mbr.uchFunctionCode;
									pThis->m_lpSolaID->ByteCount = pThis->m_itRespMBMsgs->mbr.uscbRespLen;
									pThis->m_lpSolaID->SolaID = pThis->m_itRespMBMsgs->mbr.chResponse[0];
									pThis->m_lpSolaID->RunIndicator = pThis->m_itRespMBMsgs->mbr.chResponse[1];
									nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&pThis->m_itRespMBMsgs->mbr.chResponse[2],SOLAOSNUMBERLEN,pThis->m_szOSNumber,SOLAOSNUMBERLEN);
									nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&pThis->m_itRespMBMsgs->mbr.chResponse[18],SOLABURNERNAMELEN,pThis->m_szBurnerName,SOLABURNERNAMELEN);
									pThis->m_bSolaConnected = true;
									bResult = ::PostMessage(pThis->m_hwndParent,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)pThis->m_lpSolaID);
								}
								break;
							default:
								break;
							}
						}
					}
				}
			}
			::LeaveCriticalSection(pThis->m_lpRespCritSect);
#endif
#if 0
			::CopyMemory(lpmbapHdr,recvBuf,sizeof(MBAPHEADER));
			lpmbapHdr->usTransactionIdentifier = ::ntohs(lpmbapHdr->usTransactionIdentifier);
			lpmbapHdr->usProtocolIdentifier = ::ntohs(lpmbapHdr->usProtocolIdentifier);
			lpmbapHdr->usLength = ::ntohs(lpmbapHdr->usLength);
			i = (int)pThis->m_lplReqMBMsgs->size();
			if ( recvBuf[sizeof(MBAPHEADER)] == 0x11 )
			{
				// recvbuf contains Sola slave ID response
				pThis->m_lpSolaID = (LPSOLAIDRESPONSE) new SOLAIDRESPONSE;
				pThis->m_szBurnerName = (TCHAR*) new TCHAR[20];
				pThis->m_szOSNumber = (TCHAR*) new TCHAR[16];
				if ( pThis->m_lpSolaID )
				{
					pThis->m_lpSolaID->SolaAddr = lpmbapHdr->uchUnitIdentifier;
					pThis->m_lpSolaID->FunctionCode = recvBuf[sizeof(MBAPHEADER)];
					pThis->m_lpSolaID->ByteCount = recvBuf[sizeof(MBAPHEADER)+1];
					pThis->m_lpSolaID->SolaID = recvBuf[sizeof(MBAPHEADER)+2];
					pThis->m_lpSolaID->RunIndicator = recvBuf[sizeof(MBAPHEADER)+3];
					nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&recvBuf[sizeof(MBAPHEADER)+4],SOLAOSNUMBERLEN,pThis->m_szOSNumber,SOLAOSNUMBERLEN);
					nResult = ::MultiByteToWideChar(CP_ACP,MB_PRECOMPOSED,(LPCSTR)&recvBuf[sizeof(MBAPHEADER)+20],SOLABURNERNAMELEN,pThis->m_szBurnerName,SOLABURNERNAMELEN);
					pThis->m_bSolaConnected = true;
				}

			}
#endif
		}
		if ( WSAGETSELECTEVENT(lParam) == FD_CONNECT )
		{
//			nSocketConnect = (nConnectResult != SOCKET_ERROR);
			nSocketConnect = true;
			switch (WSAGETSELECTERROR(lParam))
			{
			case WSAEAFNOSUPPORT:
				nSocketConnect = false;
				break;
			case WSAECONNREFUSED:
				nSocketConnect = false;
				break;
			case WSAENETUNREACH:
				nSocketConnect = false;
				break;
			case WSAEFAULT:
				nSocketConnect = false;
				break;
			case WSAEINVAL:
				nSocketConnect = false;
				break;
			case WSAEISCONN:
				nSocketConnect = false;
				break;
			case WSAEMFILE:
				nSocketConnect = false;
				break;
			case WSAENOBUFS:
				nSocketConnect = false;
				break;
			case WSAENOTCONN:
				nSocketConnect = false;
				break;
			case WSAETIMEDOUT:
				nSocketConnect = false;
				break;
			}
			if ( (pThis->m_MBSocket == INVALID_SOCKET) || (!nSocketConnect) )
			{
				nResult = ::MessageBox(pThis->m_hwndParent,_T("Unable to connect to server"),szTitle,MB_OK);
			}
			if ( !pThis->IsConnected() && nSocketConnect )
			{
				pThis->m_bConnected = true;
				if ( !pThis->m_lplRespMBMsgs )
				{
					pThis->m_lplRespMBMsgs = (std::list<MBRESPMESSAGE>*) new std::list<MBRESPMESSAGE>;
				}
				if ( !pThis->m_lplReqMBMsgs )
				{
					pThis->m_lplReqMBMsgs = (std::list<MBMESSAGE>*) new std::list<MBMESSAGE>;
				}
				pThis->m_hQuitEvent = ::CreateEvent(NULL,false,false,NULL);
				pThis->m_hResponseEvent = ::CreateEvent(NULL,false,false,NULL);
				pThis->m_hTimerEvent = ::CreateEvent(NULL,false,false,NULL);
				pThis->m_lpReqCritSect = (LPCRITICAL_SECTION) new CRITICAL_SECTION;
				pThis->m_lpRespCritSect = (LPCRITICAL_SECTION) new CRITICAL_SECTION;
				::InitializeCriticalSection(pThis->m_lpReqCritSect);
				::InitializeCriticalSection(pThis->m_lpRespCritSect);
				pThis->m_hSolaPollThread = ::CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)pThis->SolaPollThread,(LPVOID)pThis,CREATE_SUSPENDED,&(pThis->m_dwSolaPollThreadID));
				if ( pThis->m_hTCPSocketThread )
				{
					bResult = ::DuplicateHandle(::GetCurrentProcess(),pThis->m_hSolaPollThread,::GetCurrentProcess(),&(pThis->m_hdupSolaPollThread),DUPLICATE_SAME_ACCESS,true,DUPLICATE_SAME_ACCESS);
					dwResult = ::ResumeThread(pThis->m_hSolaPollThread);
				}
				else
				{
					nResult = ::MessageBox(pThis->m_hwndParent,_T("Unable to create polling thread"),szTitle,MB_OK);
					return 0;
				}
#if 0
				if ( !lpmbm )
				{
					lpmbm = (LPMBMESSAGE) new MBMESSAGE;
				}
				if ( !pThis->m_lplReqMBMsgs )
				{
					pThis->m_lplReqMBMsgs = (std::list<MBMESSAGE>*) new std::list<MBMESSAGE>;
				}
				if ( !pThis->m_lplRespMBMsgs )
				{
					pThis->m_lplRespMBMsgs = (std::list<MBRESPMESSAGE>*) new std::list<MBRESPMESSAGE>;
				}

				lpmbm->mbaphdr.usTransactionIdentifier = ::htons(++(pThis->m_usti));
				lpmbm->mbaphdr.usLength = ::htons((u_short)0x0006);
				lpmbm->mbaphdr.uchUnitIdentifier = 0x01;
				lpmbm->mbaphdr.usProtocolIdentifier = ::htons((u_short)0x0000);
				lpmbm->mbr.uchFunctionCode = 0x11;
				lpmbm->mbr.usRegisterCnt = ::htons((u_short)0x0000);
				lpmbm->mbr.usStartAddr = ::htons(0);
				nSendResult = send(pThis->m_MBSocket,(char*)lpmbm,sizeof(MBMESSAGE),0);
			    if ( nSendResult == SOCKET_ERROR )
				{
					hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
					nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
				}

				lpmbm->mbaphdr.usTransactionIdentifier = ::htons(++(pThis->m_usti));
				lpmbm->mbaphdr.usLength = ::htons((u_short)0x0006);
				lpmbm->mbaphdr.uchUnitIdentifier = 0x01;
				lpmbm->mbaphdr.usProtocolIdentifier = ::htons((u_short)0x0000);
				lpmbm->mbr.uchFunctionCode = 0x03;
				lpmbm->mbr.usRegisterCnt = ::htons((u_short)0x0006);
				lpmbm->mbr.usStartAddr = ::htons(0);
				nSendResult = send(pThis->m_MBSocket,(char*)lpmbm,sizeof(MBMESSAGE),0);
			    if ( nSendResult == SOCKET_ERROR )
				{
					hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
					nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
				}
				// First send slave ID request to establish if Sola really connected to gateway
				// Note: save request, in request list, in host format first, then convert to network format for sending
				//       only need to convert values longer than 1 byte
				lpmbm->mbaphdr.usTransactionIdentifier = ++(pThis->m_usti);
				lpmbm->mbaphdr.usLength = (u_short)0x0006;
				lpmbm->mbaphdr.uchUnitIdentifier = 0x01;
				lpmbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
				lpmbm->mbr.uchFunctionCode = 0x11;
				lpmbm->mbr.usRegisterCnt = (u_short)0x0000;
				lpmbm->mbr.usStartAddr = 0;
				pThis->m_lplReqMBMsgs->push_back(*lpmbm);
				lpmbm->mbaphdr.usTransactionIdentifier = ::htons(lpmbm->mbaphdr.usTransactionIdentifier);
				lpmbm->mbaphdr.usLength = ::htons(lpmbm->mbaphdr.usLength);
				lpmbm->mbaphdr.usProtocolIdentifier = ::htons(lpmbm->mbaphdr.usProtocolIdentifier);
				lpmbm->mbr.usRegisterCnt = ::htons(lpmbm->mbr.usRegisterCnt);
				lpmbm->mbr.usStartAddr = ::htons(lpmbm->mbr.usStartAddr);
				nSendResult = send(pThis->m_MBSocket,(char*)lpmbm,sizeof(MBMESSAGE),0);
			    if ( nSendResult == SOCKET_ERROR )
				{
					hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
					nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
				}
#endif

			}
		}
		break;
	case WM_APPUSERQUIT:
		bResult = ::KillTimer(hWnd,uipTimer1);
		bResult = DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		if ( lpmbm )
		{
			delete lpmbm;
		}
		if ( lpmbapHdr )
		{
			delete lpmbapHdr;
		}
		if ( lpMBRespMsg )
		{
			delete lpMBRespMsg;
		}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

#if 0
DWORD WINAPI CSolaTCPComm::SolaPollThread(LPARAM lParam)
{
	CSolaTCPComm* pThis = (CSolaTCPComm*) lParam;
	LPMBMESSAGE lpmbm = NULL;
	HRESULT hRes;
	TCHAR szError[100];
	int nSendResult;
	int nResult;
	DWORD dwResult;

	if ( !pThis->IsConnected() )
	{
		return 0;
	}
	// First send slave ID request to establish if Sola really connected to gateway
	// Note: save request, in request list, in host format first, then convert to network format for sending
	//       only need to convert values longer than 1 byte
	if ( !lpmbm )
	{
		lpmbm = (LPMBMESSAGE) new MBMESSAGE;
	}
	lpmbm->mbaphdr.usTransactionIdentifier = ++(pThis->m_usti);
	lpmbm->mbaphdr.usLength = (u_short)0x0006;
	lpmbm->mbaphdr.uchUnitIdentifier = 0x01;
	lpmbm->mbaphdr.usProtocolIdentifier = (u_short)0x0000;
	lpmbm->mbr.uchFunctionCode = 0x11;
	lpmbm->mbr.usRegisterCnt = (u_short)0x0000;
	lpmbm->mbr.usStartAddr = 0;
	::EnterCriticalSection(pThis->m_lpReqCritSect);
	pThis->m_lplReqMBMsgs->push_back(*lpmbm);
	::LeaveCriticalSection(pThis->m_lpReqCritSect);
	lpmbm->mbaphdr.usTransactionIdentifier = ::htons(lpmbm->mbaphdr.usTransactionIdentifier);
	lpmbm->mbaphdr.usLength = ::htons(lpmbm->mbaphdr.usLength);
	lpmbm->mbaphdr.usProtocolIdentifier = ::htons(lpmbm->mbaphdr.usProtocolIdentifier);
	lpmbm->mbr.usRegisterCnt = ::htons(lpmbm->mbr.usRegisterCnt);
	lpmbm->mbr.usStartAddr = ::htons(lpmbm->mbr.usStartAddr);
	while ( !pThis->m_bQuit && !pThis->IsSolaConnected() )
	{
		nSendResult = send(pThis->m_MBSocket,(char*)lpmbm,sizeof(MBMESSAGE),0);
		if ( nSendResult == SOCKET_ERROR )
		{
			hRes = ::StringCchPrintf(szError,sizeof(szError)/sizeof(TCHAR),_T("send() failed with error %d"),::WSAGetLastError());
			nResult = ::MessageBox(pThis->m_hwndParent,szError,szTitle,MB_OK);
		}
		::Sleep(500);
	}
	dwResult = ::WaitForSingleObject(pThis->m_hQuitEvent,INFINITE);
	delete lpmbm;
	return 0;
}
#endif