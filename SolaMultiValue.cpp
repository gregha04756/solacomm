#include "stdafx.h"
#include "solacomm.h"
#include "SolaMultiValue.h"

CSolaMultiValue::CSolaMultiValue(void)
{
	this->m_lpSolaMultiValue = NULL;
	this->m_cbSize = 0;
}

CSolaMultiValue::~CSolaMultiValue(void)
{
}

LPSOLAMULTIVALUE CSolaMultiValue::GetLPMulti()
{
	return this->m_lpSolaMultiValue;
}

const TCHAR* CSolaMultiValue::GetMultiString(int index)
{
	if ( index < m_cbSize )
	{
		return m_lpSolaMultiValue[index].szString;
	}
	return NULL;
}

const TCHAR* CSolaMultiValue::GetMultiString(unsigned short usValue)
{
	for ( int i = 0; i < GetSize(); i++ )
	{
		if ( usValue == (unsigned short)m_lpSolaMultiValue[i].nValue )
		{
			return (TCHAR*)m_lpSolaMultiValue[i].szString;
		}
	}
	return NULL;
}
