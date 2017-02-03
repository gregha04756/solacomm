#pragma once

#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaAlert.h"
#include "SolaLockout.h"

typedef struct __tagSolaChangeCode {	const unsigned short usBit;
										const TCHAR* szString;
										CSolaMBMap* pSolaMBMap;
										CSolaAlert* pSolaAlertLog;
										CSolaLockout* pSolaLockoutLog; } SOLACHANGECODE, *LPSOLACHANGECODE;

class CSolaChangeCode
{
public:
	CSolaChangeCode(void);
	~CSolaChangeCode(void);
	CSolaChangeCode(LPSOLACHANGECODE p, int nSize) : m_lpSolaChangeCode(p), m_nSize(nSize){}
	const unsigned short ItemBitMask(int nIndex);
	const TCHAR* ItemLabel(int nIndex);
	CSolaMBMap* SolaMBMap(int nIndex);
	CSolaAlert* SolaAlertLog(int nIndex);
	CSolaLockout* SolaLockoutLog(int nIndex);
	inline int GetSize(){return m_nSize;};
private:
	LPSOLACHANGECODE m_lpSolaChangeCode;
	int m_nSize;
};
