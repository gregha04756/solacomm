#include "StdAfx.h"
#include "SolaMBMaps.h"
#include "SolaComm.h"

CSolaMBMaps::CSolaMBMaps(void)
{
	this->m_lpSolaMBMaps = NULL;
	this->m_nSize = 0;
}

CSolaMBMaps::~CSolaMBMaps(void)
{
}

CSolaMBMap* CSolaMBMaps::GetLPMap(int index)
{
	if ( index < this->GetSize() )
	{
		return this->m_lpSolaMBMaps[index].lpCSolaMBMap;
	}
	return NULL;
}

//int CSolaMBMaps::GetMapSize(int index)
//{
//	if ( index < GetSize() )
//	{
//		return this->m_lpSolaMBMaps[index].nSize;
//	}
//	return 0;
//}