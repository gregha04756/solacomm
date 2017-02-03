#include "StdAfx.h"
#include "SolaLockoutDesc.h"

CSolaLockoutDesc::CSolaLockoutDesc(void)
{
	m_nSize = 0;
	m_lpSLD = NULL;
}

CSolaLockoutDesc::CSolaLockoutDesc(const LPSOLALOCKOUTDESC lpSLD, int nSize):m_lpSLD(lpSLD),m_nSize(nSize)
{
}

CSolaLockoutDesc::~CSolaLockoutDesc(void)
{
}

const TCHAR* CSolaLockoutDesc::GetLockoutDesc(int ndx)
{
	if ( ndx < m_nSize )
	{
		return m_lpSLD[ndx].szLockoutText;
	}
	return NULL;
}