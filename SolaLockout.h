#pragma once

#include "StdAfx.h"
#include "SolaComm.h"

class CSolaLockout
{
public:
	CSolaLockout();
	CSolaLockout(LPSOLALOCKOUT p, int nSize);
	inline int GetSize(){return m_cbSize;};
	LPSOLALOCKOUT GetLPMap(int index);
public:
	~CSolaLockout(void);
private:
	LPSOLALOCKOUT m_lpSolaLockout;
	int m_cbSize;
};