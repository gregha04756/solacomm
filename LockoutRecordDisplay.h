#pragma once

#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaLockout.h"

extern "C++" CSolaMultiValue* pcDigitalIOCodes;

extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerAnnunFirstOutCodes;
extern "C++" CSolaMultiValue* pcBurnerAnnunHoldCodes;
extern "C++" CSolaMultiValue* pcDigitalIOCodes;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" float TempVal(BOOL units, short temp);
extern "C++" short ssTempVal (BOOL units, short temp);

class CLockoutRecordDisplay
{
public:
	CLockoutRecordDisplay(void);
	CLockoutRecordDisplay(HWND hOwner, HINSTANCE hInst, TCHAR* szTitle, CSolaMultiValue* psld);
	~CLockoutRecordDisplay(void);
	BOOL LockoutRecordDisplayIsActive(void);
	BOOL Start(CSolaLockout* csl, int ndx);
	BOOL Stop(void);
private:
	static INT_PTR CALLBACK LockoutRecordDisplayDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
	static DWORD WINAPI LockoutRecordDisplayThreadProc(LPVOID lParam);
	HWND m_hwndLockoutRecordDisplayDlg;
	HWND m_hwndOwner;
	HANDLE m_hLockoutRecordDisplayThread;
	HANDLE m_hdupLockoutRecordDisplayThread;
	HINSTANCE m_hInst;
	TCHAR* m_szTitle;
	DWORD m_dwLockoutRecordDisplayThreadID;
	CSolaLockout* m_pcsl;
	int m_slndx;
	CSolaMultiValue* m_psld;
};
