#pragma once

#include "stdafx.h"
#include "SolaComm.h"

class CSolaAlert
{
public:
	CSolaAlert();
	CSolaAlert(LPSOLAALERT p, int nSize) : m_lpSolaAlert(p), m_cbSize(nSize){}
	inline int GetSize(){ return m_cbSize;};
	inline LPSOLAALERT GetLPMap(int index){ return (m_lpSolaAlert+index);};
public:
	~CSolaAlert(void);
private:
	LPSOLAALERT m_lpSolaAlert;
	int m_cbSize;
};
