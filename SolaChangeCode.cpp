#include "StdAfx.h"
#include "SolaChangeCode.h"

CSolaChangeCode::CSolaChangeCode(void)
{
	m_lpSolaChangeCode = NULL;
	m_nSize = 0;
}

CSolaChangeCode::~CSolaChangeCode(void)
{
}

const unsigned short CSolaChangeCode::ItemBitMask(int nIndex)
{
	if ( nIndex < this->GetSize() && m_lpSolaChangeCode != NULL )
	{
		return this->m_lpSolaChangeCode[nIndex].usBit;
	}
	return NULL;
}

const TCHAR* CSolaChangeCode::ItemLabel(int nIndex)
{
	if ( nIndex < this->GetSize() && m_lpSolaChangeCode != NULL )
	{
		return this->m_lpSolaChangeCode[nIndex].szString;
	}
	return NULL;
}

CSolaMBMap* CSolaChangeCode::SolaMBMap(int nIndex)
{
	if ( nIndex < this->GetSize() && m_lpSolaChangeCode != NULL )
	{
		return this->m_lpSolaChangeCode[nIndex].pSolaMBMap;
	}
	return NULL;
}

CSolaAlert* CSolaChangeCode::SolaAlertLog(int nIndex)
{
	if ( nIndex < this->GetSize() && m_lpSolaChangeCode != NULL )
	{
		return this->m_lpSolaChangeCode[nIndex].pSolaAlertLog;
	}
	return NULL;
}

CSolaLockout* CSolaChangeCode::SolaLockoutLog(int nIndex)
{
	if ( nIndex < this->GetSize() && m_lpSolaChangeCode != NULL )
	{
		return this->m_lpSolaChangeCode[nIndex].pSolaLockoutLog;
	}
	return NULL;
}
