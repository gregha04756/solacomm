#include "StdAfx.h"
#include "SolaID.h"

CSolaID::CSolaID(void)
{
	m_hCOM = NULL;
	m_hwndParent = NULL;
	m_ucMBAddr = 0;
	m_lpSolaID = NULL;
	m_szTitle = NULL;
	m_hGetSolaIDThread = NULL;
	m_szBurnerName = NULL;
	m_szOSNumber = NULL;
	m_cbOSNumber = 0;
	m_cbBurnerName = 0;
	m_pNoticeDialog = NULL;
}

CSolaID::CSolaID(HANDLE hCOM,HWND hwndParent,unsigned char MBAddr,LPCRITICAL_SECTION pCOMCritSect,TCHAR* szTitle, CNoticeDialog* pnd) : m_hwndParent(hwndParent),m_ucMBAddr(MBAddr),m_pCOMCritSect(pCOMCritSect),m_szTitle(szTitle), m_pNoticeDialog(pnd)
{
	m_lpSolaID = NULL;
	m_hGetSolaIDThread = NULL;
	m_szBurnerName = NULL;
	m_szOSNumber = NULL;
	m_cbOSNumber = 0;
	m_cbBurnerName = 0;
	BOOL bResult = ::DuplicateHandle(::GetCurrentProcess(), hCOM, ::GetCurrentProcess(), &m_hCOM, 0, false, DUPLICATE_SAME_ACCESS);
}

CSolaID::~CSolaID(void)
{
	DWORD dwResult;
	BOOL bResult;
	int nResult;
	if ( this->m_hGetSolaIDThread )
	{
		if ( (dwResult = ::WaitForSingleObject(m_hGetSolaIDThread,3000)) == WAIT_TIMEOUT )
		{
			nResult = ::MessageBox(this->m_hwndParent,_T("GetSolaID thread not closed"),this->m_szTitle,MB_OK);
		}
		else
		{
			bResult = CloseHandle(m_hGetSolaIDThread);
			bResult = CloseHandle(m_hdupGetSolaIDThread);
			m_hGetSolaIDThread = NULL;
			m_hdupGetSolaIDThread = NULL;
		}
	}
	if ( m_szBurnerName )
	{
		delete m_szBurnerName;
		m_szBurnerName = NULL;
		m_cbBurnerName = 0;
	}
	if ( m_szOSNumber )
	{
		delete m_szOSNumber;
		m_szOSNumber = NULL;
		m_cbOSNumber = 0;
	}
	if ( m_lpSolaID )
	{
		delete m_lpSolaID;
		m_lpSolaID = NULL;
	}
	if ( m_hCOM )
	{
		BOOL bResult = ::CloseHandle(m_hCOM);
		m_hCOM = NULL;
	}
}

BOOL CSolaID::GetSolaID()
{
	BOOL bSuccess = true;
	BOOL bResult;
	DWORD dwResult;
	int nResult;
	if ( m_hGetSolaIDThread == NULL )
	{
		m_hGetSolaIDThread = ::CreateThread(NULL,0,GetSolaIDThreadProc,(LPVOID)this,CREATE_SUSPENDED,&m_dwGetSolaIDThreadID);
		bResult = ::DuplicateHandle(::GetCurrentProcess(),m_hGetSolaIDThread,::GetCurrentProcess(),&m_hdupGetSolaIDThread,DUPLICATE_SAME_ACCESS,true,0);
		dwResult = ::ResumeThread(m_hGetSolaIDThread);
	}
	if ( m_hGetSolaIDThread == NULL )
	{
		nResult = ::MessageBox(m_hwndParent,_T("Error starting GetSolaID thread"),m_szTitle,MB_OK);
		bSuccess = false;
	}
	return bSuccess;
}

DWORD WINAPI CSolaID::GetSolaIDThreadProc(LPVOID lParam)
{
	BOOL bResult;

	CSolaID* psid = (CSolaID*)lParam;

	unsigned char SOLAIDRequest[] = { 0x01, 0x11 };
	BOOL bSuccess = true;
	int i;
	unsigned short usCRC16;
	unsigned char MBMsg[512];
	union {	unsigned char SOLAResponse[512]; HOLDINGREG hr; SOLAIDRESPONSE SolaID; };
	LPSOLAIDRESPONSE lpSolaID = NULL;
	DWORD dwBytesWritten = 0;
	DWORD dwBytesRead = 0;

	i = psid->m_pNoticeDialog->SetNoticeTxt(_T("Checking bus for traffic"));
	if ( !psid->IsBusQuiet(psid->m_hCOM) )
	{
		i = psid->m_pNoticeDialog->SetNoticeTxt(_T("Unexpected traffic, cannot connect"));
		psid->m_lpSolaID = lpSolaID;
		bResult = ::PostMessage(psid->m_hwndParent,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)lpSolaID);
//		Sleep(1000);
		return 0;
	}
	i = psid->m_pNoticeDialog->SetNoticeTxt(_T("Quiet bus, getting Sola ID"));
	
	::EnterCriticalSection( psid->m_pCOMCritSect );
	SOLAIDRequest[0] = psid->m_ucMBAddr;
	for ( i = 0; i < sizeof(SOLAIDRequest); i++ )
	{
		MBMsg[i] = SOLAIDRequest[i];
	}
	usCRC16 = calc_CRC16( MBMsg, 2 );
	MBMsg[i++] = usCRC16 & 0x00ff;
	MBMsg[i++] = (usCRC16 >> 8) & 0x00ff;
	bSuccess = ::WriteFile(psid->m_hCOM, MBMsg, i, &dwBytesWritten, NULL);

	if ( bSuccess )
	{
		bSuccess = ::ReadFile(psid->m_hCOM, (LPVOID) &SOLAResponse[0], 512, &dwBytesRead, NULL);
	}

	if ( bSuccess )
	{
		bSuccess = (dwBytesRead > 0);
	}

	if ( bSuccess)
	{
		bSuccess =  check_CRC16(SOLAResponse, dwBytesRead);
	}

	if ( bSuccess )
	{
		bSuccess = (SolaID.ByteCount == 0x26);
	}

	if ( bSuccess )
	{
		lpSolaID = new SOLAIDRESPONSE;
		lpSolaID->SolaAddr = SolaID.SolaAddr;
		lpSolaID->FunctionCode = SolaID.FunctionCode;
		lpSolaID->SolaID = SolaID.SolaID;
		lpSolaID->RunIndicator = SolaID.RunIndicator;
		for ( i = 0; i < sizeof(SolaID.BurnerName); i++ )
		{
			*((lpSolaID->BurnerName)+i) = SolaID.BurnerName[i];
		}
		psid->m_cbBurnerName = i;
		psid->m_szBurnerName = new TCHAR[i+1];
		for ( i = 0; i < sizeof(SolaID.OSNumber); i++ )
		{
			*((lpSolaID->OSNumber)+i) = SolaID.OSNumber[i];
		}
		psid->m_cbOSNumber = i;
		psid->m_szOSNumber = new TCHAR[i+1];
		int nResult = MultiByteToWideChar(	CP_ACP,
											MB_PRECOMPOSED,
											lpSolaID->BurnerName,
											sizeof(lpSolaID->BurnerName),
											psid->m_szBurnerName,
											psid->m_cbBurnerName);
		nResult = MultiByteToWideChar(	CP_ACP,
										MB_PRECOMPOSED,
										lpSolaID->OSNumber,
										sizeof(lpSolaID->OSNumber),
										psid->m_szOSNumber,
										psid->m_cbOSNumber);
	}

	::LeaveCriticalSection( psid->m_pCOMCritSect );

	if ( !bSuccess )
	{
		lpSolaID = NULL;
	}	
	psid->m_lpSolaID = lpSolaID;
	bResult = ::PostMessage(psid->m_hwndParent,WM_APPSOLAIDUPD,(WPARAM)0,(LPARAM)lpSolaID);
	return 0;
}

BOOL CSolaID::IsBusQuiet(HANDLE& hCOM)
{
	COMMTIMEOUTS OldCommTimeouts;
	COMMTIMEOUTS CommTimeouts;
	unsigned char uchBuf[512];
	PVOID lpVoid;
	DWORD dwBytesRead = 0;
	BOOL bSuccess = true;
	BOOL bBusQuiet = true;

	::EnterCriticalSection( this->m_pCOMCritSect );
	bSuccess = ::PurgeComm(hCOM,PURGE_RXCLEAR);
	if ( bSuccess )
	{
		bSuccess = GetCommTimeouts(hCOM, &CommTimeouts);
	}
	if ( bSuccess )
	{
		OldCommTimeouts.ReadIntervalTimeout = CommTimeouts.ReadIntervalTimeout;
		OldCommTimeouts.ReadTotalTimeoutConstant = CommTimeouts.ReadTotalTimeoutConstant;
		OldCommTimeouts.ReadTotalTimeoutMultiplier = CommTimeouts.ReadTotalTimeoutMultiplier;
		OldCommTimeouts.WriteTotalTimeoutConstant = CommTimeouts.WriteTotalTimeoutConstant;
		OldCommTimeouts.WriteTotalTimeoutMultiplier = CommTimeouts.WriteTotalTimeoutMultiplier;
		CommTimeouts.ReadIntervalTimeout = 1000;           // 1.9ms max. interval between incoming chars.
		CommTimeouts.ReadTotalTimeoutConstant = 3000;
		CommTimeouts.ReadTotalTimeoutMultiplier = 0;      // #chars to read does not add to timeout amount
		CommTimeouts.WriteTotalTimeoutConstant = 2000;
		CommTimeouts.WriteTotalTimeoutMultiplier = 60;    // 60ms per char sent
		bSuccess = ::SetCommTimeouts(hCOM, &CommTimeouts);
	}
	if ( bSuccess )
	{
		lpVoid = ::SecureZeroMemory((PVOID)uchBuf,(SIZE_T)sizeof(uchBuf));
		bSuccess = ::ReadFile(hCOM,(LPVOID)uchBuf,sizeof(uchBuf),&dwBytesRead,NULL);
	}
	if ( bSuccess )
	{
		CommTimeouts.ReadIntervalTimeout = OldCommTimeouts.ReadIntervalTimeout;
		CommTimeouts.ReadTotalTimeoutConstant = OldCommTimeouts.ReadTotalTimeoutConstant;
		CommTimeouts.ReadTotalTimeoutMultiplier = OldCommTimeouts.ReadTotalTimeoutMultiplier;
		CommTimeouts.WriteTotalTimeoutConstant = OldCommTimeouts.WriteTotalTimeoutConstant;
		CommTimeouts.WriteTotalTimeoutMultiplier = OldCommTimeouts.WriteTotalTimeoutMultiplier;
		bSuccess = ::SetCommTimeouts(hCOM, &CommTimeouts);
	}
	if ( bSuccess )
	{
		bBusQuiet = (dwBytesRead == 0);
	}
	::LeaveCriticalSection( this->m_pCOMCritSect );
	return (bBusQuiet && bSuccess);
}
