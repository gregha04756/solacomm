#pragma once

extern "C++" unsigned short calc_CRC16(unsigned char* puchMsg, unsigned short usDataLen);
extern "C++" int check_CRC16(unsigned char *buf, int buflen);

#include "SolaComm.h"
#include "NoticeDialog.h"

class CSolaID
{
public:
	CSolaID();
	CSolaID(HANDLE hCOM,HWND hParent,unsigned char MBAddr,LPCRITICAL_SECTION pCOMCritSect,TCHAR* szTitle, CNoticeDialog* pnd);
	BOOL GetSolaID(void);
	inline TCHAR* GetBurnerName(){return m_szBurnerName;};
	inline TCHAR* GetOSNumber(){return this->m_szOSNumber;};
	inline LPSOLAIDRESPONSE RtnSolaID() {return m_lpSolaID;};
	~CSolaID(void);
private:
	LPSOLAIDRESPONSE m_lpSolaID;
	TCHAR* m_szBurnerName;
	int m_cbBurnerName;
	TCHAR* m_szOSNumber;
	int m_cbOSNumber;
	TCHAR* m_szTitle;
	HANDLE m_hCOM;
	HANDLE m_hGetSolaIDThread;
	HANDLE m_hdupGetSolaIDThread;
	DWORD m_dwGetSolaIDThreadID;
	LPCRITICAL_SECTION m_pCOMCritSect;
	HWND m_hwndParent;
	unsigned char m_ucMBAddr;
	static DWORD WINAPI GetSolaIDThreadProc(LPVOID lParam);
	BOOL IsBusQuiet(HANDLE& hCOM);
	CNoticeDialog* m_pNoticeDialog;
};
