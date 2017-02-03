#pragma once
#include "SolaComm.h"
#include "SolaMBMap.h"

typedef struct __tagSolaMBMaps { CSolaMBMap* lpCSolaMBMap; } SOLAMBMAPS, *LPSOLAMBMAPS;

class CSolaMBMaps
{
public:
	CSolaMBMaps(void);
	CSolaMBMaps(LPSOLAMBMAPS p, int nSize) : m_lpSolaMBMaps(p), m_nSize(nSize){}
	CSolaMBMap* GetLPMap(int index);
//	int GetMapSize(int index);
	inline int GetSize(){ return m_nSize;};
	~CSolaMBMaps(void);
private:
	LPSOLAMBMAPS m_lpSolaMBMaps;
	int m_nSize;
};
