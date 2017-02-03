#include "StdAfx.h"
#include "SolaPage.h"

CSolaPage::CSolaPage(void)
{
	m_lpSolaPage = NULL;
	m_nSize = 0;
}

CSolaPage::~CSolaPage(void)
{
}

const TCHAR* CSolaPage::ItemLabel(int nIndex)
{
	if ( nIndex < this->GetSize() )
	{
		return this->m_lpSolaPage[nIndex].szLabel;
	}
	return NULL;
}

const int CSolaPage::ItemIndex(int nIndex)
{
	if ( nIndex < this->GetSize() )
	{
		return this->m_lpSolaPage[nIndex].nIndex;
	}
	return NULL;
}

CSolaMBMap* CSolaPage::ItemMap(int nIndex)
{
	if ( nIndex < this->GetSize() )
	{
		return this->m_lpSolaPage[nIndex].pcSolaMBMap;
	}
	return NULL;
}

