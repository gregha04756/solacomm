#include "StdAfx.h"
#include "SolaComm.h"
#include "SolaLockout.h"

CSolaLockout::CSolaLockout()
{
	m_lpSolaLockout = NULL;
	m_cbSize = 0;
}
CSolaLockout::CSolaLockout(LPSOLALOCKOUT p, int nSize) : m_lpSolaLockout(p), m_cbSize(nSize)
{
}

CSolaLockout::~CSolaLockout(void)
{
}

LPSOLALOCKOUT CSolaLockout::GetLPMap(int index)
{
	return (m_lpSolaLockout+index);
}
