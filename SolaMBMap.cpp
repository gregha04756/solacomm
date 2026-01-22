#include "StdAfx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"

CSolaMBMap::CSolaMBMap()
{
	m_lpSolaMBMap = NULL;
	m_lpSolaU32Map = NULL;
	m_cbSize = 0;
}

CSolaMBMap::CSolaMBMap(LPSOLAMBMAP p,int nSize) : m_lpSolaMBMap(p),m_cbSize(nSize)
{
	this->m_lpSolaU32Map = NULL;
	for (int i = 0; i < m_cbSize; i++ )
	{
		if ((NULL != m_lpSolaMBMap[i].pchStr) &&
			((Stringvalue == m_lpSolaMBMap[i].valuetype) ||
			(DupStringValue == m_lpSolaMBMap[i].valuetype)))
		{
			PVOID p_v = SecureZeroMemory((PVOID)m_lpSolaMBMap[i].pchStr,(SIZE_T)m_lpSolaMBMap[i].cbStrLen);
		}
	}
}

CSolaMBMap::CSolaMBMap(LPSOLAU32MAP p,int nSize) : m_lpSolaU32Map(p),m_cbSize(nSize)
{
	this->m_lpSolaMBMap = NULL;
	for (int i = 0; i < m_cbSize; i++ )
	{
		if ((NULL != m_lpSolaU32Map[i].pchStr) &&
			((Stringvalue == m_lpSolaU32Map[i].valuetype) ||
			(DupStringValue == m_lpSolaU32Map[i].valuetype)))
		{
			PVOID p_v = SecureZeroMemory((PVOID)m_lpSolaU32Map[i].pchStr,(SIZE_T)m_lpSolaU32Map[i].cbStrLen);
		}
	}
}

CSolaMBMap::~CSolaMBMap(void)
{
}

CSolaMBMap::LPSOLAMBMAP CSolaMBMap::GetLPMap(int index)
{
	if ( (NULL!= m_lpSolaMBMap) && (index < this->GetRegGroupSize()) )
	{
		return (LPSOLAMBMAP)(m_lpSolaMBMap+index);
	}
	return (LPSOLAMBMAP)NULL;
}

CSolaMBMap::LPSOLAU32MAP CSolaMBMap::GetLPU32Map(int index)
{
	if ( (NULL != m_lpSolaU32Map) && (index < this->GetRegGroupSize()) )
	{
		return (LPSOLAU32MAP)(m_lpSolaU32Map+index);
	}
	return (LPSOLAU32MAP)NULL;
}

CSolaMBMap::SolaType CSolaMBMap::GetType(unsigned short addr)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return m_lpSolaMBMap[i].valuetype;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return m_lpSolaU32Map[i].valuetype;
			}
		}
	}
	return Novalue;
}

CSolaMBMap::SolaType CSolaMBMap::GetType(int index)
{
	if ( index < GetRegGroupSize() )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			return m_lpSolaMBMap[index].valuetype;
		}
		if ( NULL != m_lpSolaU32Map )
		{
			return m_lpSolaU32Map[index].valuetype;
		}
	}
	return Novalue;
}

unsigned short CSolaMBMap::GetStartRegAddr(int index)
{
	if ( index < GetRegGroupSize() )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			return m_lpSolaMBMap[index].usStartRegAddr;
		}
		if ( NULL != m_lpSolaU32Map )
		{
			return m_lpSolaU32Map[index].usStartRegAddr;
		}
	}
	return (unsigned short)0;
}

unsigned char CSolaMBMap::GetFuncCode(int index)
{
	if ( NULL != m_lpSolaMBMap )
	{
		if ( index < GetRegGroupSize() )
		{
			return m_lpSolaMBMap[index].uchFuncCode;
		}
	}
	if ( NULL != m_lpSolaU32Map )
	{
		if ( index < GetRegGroupSize() )
		{
			return m_lpSolaU32Map[index].uchFuncCode;
		}
	}
	return (unsigned char)0;
}

unsigned char CSolaMBMap::GetFuncCode(unsigned short addr)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return m_lpSolaMBMap[i].uchFuncCode;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return m_lpSolaU32Map[i].uchFuncCode;
			}
		}
	}
	return (unsigned char)0;
}

short CSolaMBMap::GetValue(unsigned short addr)
{
	for ( int i = 0; (NULL != m_lpSolaMBMap) && (i < this->GetRegGroupSize()); i++ )
	{
		if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
		{
			return (short)m_lpSolaMBMap[i].sValue;
		}
	}
	return (short)-1;
}

short CSolaMBMap::GetValue(int index)
{
	if ( (NULL != m_lpSolaMBMap) &&  (index < GetRegGroupSize()) )
	{
		return (short)m_lpSolaMBMap[index].sValue;
	}
	return (short)-1;
}

U32 CSolaMBMap::GetU32Value(unsigned short addr)
{
	for ( int i = 0; (NULL != m_lpSolaU32Map) && (i < this->GetRegGroupSize()); i++ )
	{
		if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
		{
			return (U32)m_lpSolaU32Map[i].u32Value;
		}
	}
	return (U32)-1;
}

U32 CSolaMBMap::GetU32Value(int index)
{
	if ( (NULL != m_lpSolaU32Map) && (index < GetRegGroupSize()) )
	{
		return (U32)m_lpSolaU32Map[index].u32Value;
	}
	return (U32)-1;
}

const TCHAR* CSolaMBMap::GetParmName(unsigned short addr)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return m_lpSolaMBMap[i].ParmName;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return m_lpSolaU32Map[i].ParmName;
			}
		}
	}
	return NULL;
}

const TCHAR* CSolaMBMap::GetParmName(int index)
{
	if ( index < GetRegGroupSize() )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			return m_lpSolaMBMap[index].ParmName;
		}
		if ( NULL != m_lpSolaU32Map )
		{
			return m_lpSolaU32Map[index].ParmName;
		}
	}
	return NULL;
}

LPSOLAMULTIVALUE CSolaMBMap::GetLPMulti(unsigned short addr)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return (LPSOLAMULTIVALUE)m_lpSolaMBMap[i].lpValueList;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return (LPSOLAMULTIVALUE)m_lpSolaU32Map[i].lpValueList;
			}
		}
	}
	return NULL;
}
LPSOLAMULTIVALUE CSolaMBMap::GetLPMulti(int index)
{
	if ( index < GetRegGroupSize() )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[index].lpValueList != NULL )
			{
				return m_lpSolaMBMap[index].lpValueList;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[index].lpValueList != NULL )
			{
				return m_lpSolaU32Map[index].lpValueList;
			}
		}
	}
	return (LPSOLAMULTIVALUE)NULL;
}

int CSolaMBMap::GetMultiListSize(unsigned short addr)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return m_lpSolaMBMap[i].nValueListSize;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return m_lpSolaU32Map[i].nValueListSize;
			}
		}
	}
	return 0;
}

int CSolaMBMap::GetMultiListSize(int index)
{
	if ( index < GetRegGroupSize() )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[index].lpValueList != NULL )
			{
				return m_lpSolaMBMap[index].nValueListSize;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[index].lpValueList != NULL )
			{
				return m_lpSolaU32Map[index].nValueListSize;
			}
		}
	}
	return 0;
}

TCHAR* CSolaMBMap::GetMultiValueItem(unsigned short usAddr, unsigned short usValue)
{
	for ( int i = 0; i < GetMultiListSize((unsigned short)usAddr); i++ )
	{
		if ( (usValue == GetLPMulti((unsigned short)usAddr)[i].nValue) && (NULL != GetLPMulti((unsigned short)usAddr)[i].szString) )
		{
			return (TCHAR*)GetLPMulti((unsigned short)usAddr)[i].szString;
		}
	}
	return NULL;
}

TCHAR* CSolaMBMap::GetMultiValueItem(int index, unsigned short usValue)
{
	if ( GetLPMulti(index) != NULL )
	{
		for ( int i = 0; i < GetMultiListSize(index); i++ )
		{
			if ( usValue == (unsigned short)GetLPMulti(index)[i].nValue )
			{
				return (TCHAR*)GetLPMulti(index)[i].szString;
			}
		}
	}
	return NULL;
}

TCHAR* CSolaMBMap::GetMultiValueItem(int index, int nBitNumber)
{
	if ( this->GetLPMulti(index) != NULL )
	{
		for ( int i = 0; i < this->GetMultiListSize(index); i++ )
		{
			if ( this->GetLPMulti(index)[i].nValue == nBitNumber )
			{
				return (TCHAR*)this->GetLPMulti(index)[i].szString;
			}
		}
	}
	return NULL;
}

int CSolaMBMap::GetMultiItemValue(unsigned short addr,int listindex)
{
	for ( int i = 0; i < GetRegGroupSize(); i++ )
	{
		if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
		{
			if ( listindex < GetMultiListSize(i) )
			{
				return (int)GetLPMulti(i)[listindex].nValue;
			}
		}
	}
	return 0;
}

int CSolaMBMap::GetMultiItemValue(int mapindex,int listindex)
{
	if ( GetLPMulti(mapindex) != NULL && listindex < GetMultiListSize(mapindex))
	{
		return (int)this->GetLPMulti(mapindex)[listindex].nValue;
	}
	return 0;
}

short CSolaMBMap::SetValue(unsigned short addr, short sValue)
{
	for ( int i = 0; i < this->GetRegGroupSize(); i++ )
	{
		if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
		{
			return (m_lpSolaMBMap[i].sValue = sValue);
		}
	}
	return sValue;
}

short CSolaMBMap::SetValue(int index, short sValue)
{
	m_lpSolaMBMap[index].sValue = sValue;
	return sValue;
}

U32 CSolaMBMap::SetU32Value(unsigned short addr, U32 u32Value)
{
	for ( int i = 0; (NULL != m_lpSolaU32Map) && (i < this->GetRegGroupSize()); i++ )
	{
		if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
		{
			return (m_lpSolaU32Map[i].u32Value = u32Value);
		}
	}
	return (U32)-1;
}

U32 CSolaMBMap::SetU32Value(int index, U32 u32Value)
{
	if ( (NULL != m_lpSolaU32Map) && (index < GetRegGroupSize()) )
	{
		return (m_lpSolaU32Map[index].u32Value = u32Value);
	}
	return (U32)-1;
}

BOOL CSolaMBMap::GetWrtable(int nIndex)
{
	if ( (NULL != m_lpSolaMBMap) && (nIndex < GetRegGroupSize()) )
	{
		return this->m_lpSolaMBMap[nIndex].bWrtable;
	}
	if ( (NULL != m_lpSolaU32Map) && (nIndex < GetRegGroupSize()) )
	{
		return this->m_lpSolaU32Map[nIndex].bWrtable;
	}
	return false;
}

BOOL CSolaMBMap::GetVisible(int nIndex)
{
	if ( (NULL != m_lpSolaMBMap) && (nIndex < GetRegGroupSize()) )
	{
		return this->m_lpSolaMBMap[nIndex].bVisible;
	}
	if ( (NULL != m_lpSolaU32Map) && (nIndex < GetRegGroupSize()) )
	{
		return this->m_lpSolaU32Map[nIndex].bVisible;
	}
	return false;
}

BOOL CSolaMBMap::GetNonSafety(int nIndex)
{
	if ( (NULL != m_lpSolaMBMap) && (nIndex < GetRegGroupSize()) )
	{
		return this->m_lpSolaMBMap[nIndex].bNonSafety;
	}
	return false;
}

int CSolaMBMap::GetSetBitNumber(int nIndex)
{
	unsigned short usMask;
	int i = -1;
	for ( usMask = GetValue(nIndex); usMask > 0; usMask >>= 1 )
	{
		i++;
	}
	return i;
}

unsigned short CSolaMBMap::GetU32RegTotal()
{
	unsigned short count = 0;
	for ( int i = 0; (NULL != m_lpSolaU32Map) && ( i < GetRegGroupSize()); i++ )
	{
		count += m_lpSolaU32Map[i].usRegCount;
	}
	return (unsigned short)count;
}

unsigned short CSolaMBMap::GetRegRequestLen()
{
	unsigned short count = 0;
	for ( int i = 0; (NULL != m_lpSolaU32Map) && ( i < GetRegGroupSize()); i++ )
	{
		count += m_lpSolaU32Map[i].usRegCount;
	}
	for ( int i = 0; (NULL != m_lpSolaMBMap) && ( i < GetRegGroupSize()); i++ )
	{
		count += m_lpSolaMBMap[i].usRegCount;
	}
	return (unsigned short)count;
}

unsigned char* CSolaMBMap::GetStr(int index)
{
	if ( (NULL != m_lpSolaMBMap) && (index < GetRegGroupSize()) )
	{
		if ( m_lpSolaMBMap[index].pchStr != NULL )
		{
			return (unsigned char*)m_lpSolaMBMap[index].pchStr;
		}
	}
	if ( (NULL != m_lpSolaU32Map) && (index < GetRegGroupSize()) )
	{
		if ( m_lpSolaU32Map[index].pchStr != NULL )
		{
			return (unsigned char*)m_lpSolaU32Map[index].pchStr;
		}
	}
	return NULL;
}

unsigned char* CSolaMBMap::GetStr(unsigned short usAddr)
{
	for ( int i = 0; i < GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == usAddr )
			{
				return (unsigned char*)m_lpSolaMBMap[i].pchStr;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == usAddr )
			{
				return (unsigned char*)m_lpSolaU32Map[i].pchStr;
			}
		}
	}
	return (unsigned char*)NULL;
}

std::unique_ptr<unsigned char> CSolaMBMap::SetStr(int index,std::unique_ptr<char> pchStr,short cbLen)
{
	unsigned char* p1;
	if ( (NULL != m_lpSolaMBMap) && (index < GetRegGroupSize()) )
	{
		if ( m_lpSolaMBMap[index].pchStr != NULL )
		{
			p1 = (unsigned char*)m_lpSolaMBMap[index].pchStr;
			for (int i = 0; i < GetcbStrLen((int)index); i++ )
			{
				if ( i < cbLen )
				{
//					*(p1++) = pchStr[i];
					*(p1++) = pchStr.get()[i];
				}
				else
				{
					*(p1++) = '\0';
				}
			}
			return m_lpSolaMBMap[index].pchStr;
		}
	}
	if ( (NULL != m_lpSolaU32Map) && (index < GetRegGroupSize()) )
	{
		if ( m_lpSolaU32Map[index].pchStr != NULL )
		{
			p1 = (unsigned char*)m_lpSolaU32Map[index].pchStr;
			for (int i = 0; i < GetcbStrLen((int)index); i++ )
			{
				if ( i < cbLen )
				{
					*(p1++) = pchStr.get()[i];
				}
				else
				{
					*(p1++) = '\0';
				}
			}
			return (unsigned char*)m_lpSolaU32Map[index].pchStr;
		}
	}
	return (unsigned char*)NULL;
}

unsigned char* CSolaMBMap::SetStr(unsigned short usAddr,unsigned char* pchStr,short cbLen)
{
	int ndx;
	int i;
	unsigned char* p1;
	for ( ndx = 0; ndx < GetRegGroupSize(); ndx++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[ndx].usStartRegAddr == usAddr )
			{
				if ( m_lpSolaMBMap[ndx].pchStr != NULL )
				{
					p1 = (unsigned char*)m_lpSolaMBMap[ndx].pchStr;
					for ( i = 0; i < GetcbStrLen((int)i); i++ )
					{
						if ( i < cbLen )
						{
							*(p1++) = pchStr[i];
						}
						else
						{
							*(p1++) = '\0';
						}
					}
					return (unsigned char*)m_lpSolaMBMap[ndx].pchStr;
				}
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[ndx].usStartRegAddr == usAddr )
			{
				if ( m_lpSolaU32Map[ndx].pchStr != NULL )
				{
					p1 = (unsigned char*)m_lpSolaU32Map[ndx].pchStr;
					for ( i = 0; i < GetcbStrLen((int)i); i++ )
					{
						if ( i < cbLen )
						{
							*(p1++) = pchStr[i];
						}
						else
						{
							*(p1++) = '\0';
						}
					}
					return (unsigned char*)m_lpSolaU32Map[ndx].pchStr;
				}
			}
		}
	}
	return (unsigned char*)NULL;
}

short CSolaMBMap::GetcbStrLen(int index)
{
	if ( (NULL != m_lpSolaMBMap) && (index < GetRegGroupSize()) )
	{
		return (short)m_lpSolaMBMap[index].cbStrLen;
	}
	if ( (NULL != m_lpSolaU32Map) && (index < GetRegGroupSize()) )
	{
		return (short)m_lpSolaU32Map[index].cbStrLen;
	}
	return (short)-1;
}

short CSolaMBMap::GetcbStrLen(unsigned short addr)
{
	for ( int i = 0; i < GetRegGroupSize(); i++ )
	{
		if ( NULL != m_lpSolaMBMap )
		{
			if ( m_lpSolaMBMap[i].usStartRegAddr == addr )
			{
				return (short)m_lpSolaMBMap[i].cbStrLen;
			}
		}
		if ( NULL != m_lpSolaU32Map )
		{
			if ( m_lpSolaU32Map[i].usStartRegAddr == addr )
			{
				return (short)m_lpSolaU32Map[i].cbStrLen;
			}
		}
	}
	return (short)-1;
}