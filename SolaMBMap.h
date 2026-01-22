#pragma once

#include "StdAfx.h"
#include "SolaMultiValue.h"

class CSolaMBMap
{
public:

/* 
   All data types are unsigned 16-bit except:
   Stringvalue - a multi-byte character string contained in a contiguous sequence of registers
   Unsigned32 - a 32-bit value contained in 2 successive 16-bit registers
*/
enum SolaType {
	Novalue,		// Value not defined
	Numericvalue,	// Generic numeric value
	Bitmask,			// Bit mask
	Digitalvalue,	// Boolean (digital) value, true/false, on/off, yes/no, etc.
	Temperature,	// Temperature value
	TemperatureSetpoint, // Temperature setpoint value
	Setpointvalue, // Generic setpoint value
	Hysteresis,		// Hysteresis value
	Timevalue,		// Time in hh:mm:ss
	Seconds,		// Time in seconds
	Minutes,		// Time in minutes
	Hours,			// Time in hours
	SensorMultivalue,	// Special sensor multiple string value
	Multivalue,			// Generic multiple string value
	Stringvalue,		// String Value
	Percentvalue,		// Percentage value
	Lockoutcode,		// Lockoutcode
	Holdcode,			// Hold code
	Alertcode,			// Alert code
	Alarmreason,		// Alarm reason
	Alarmcode,			// Alarm code
	Fanspeed,			// Fan speed
	Decimal2pl,			// Real value, 0.01 precision
	Decimal1pl,		// Real value, 0.1 precision
	Unsigned32,		// Unsigned 32-bit value (2 16-bit holding registers)
	ODTemperature,    /* Outdoor temperature is a special case because it may come from one of several sources */
	DupStringValue /* Special string value that duplicates Sola char[20] string in a group of 10 uint16_t registers */
};
typedef struct __tagSolaMBMap {	const TCHAR* ParmName;
								unsigned char uchDevAddr;
								const unsigned char uchFuncCode;
								const unsigned short usStartRegAddr;
								const unsigned short usRegCount;
								signed short sValue;
								BOOL bWrtable;
								BOOL bVisible;
								BOOL bNonSafety;
								char const *pchStr;
								const signed short cbStrLen;
								const SolaType valuetype;
								const LPSOLAMULTIVALUE lpValueList;
								const int nValueListSize;} SOLAMBMAP, *LPSOLAMBMAP;

typedef struct __tagSolaU32Map {const TCHAR* ParmName;
								unsigned char uchDevAddr;
								const unsigned char uchFuncCode;
								const unsigned short usStartRegAddr;
								const unsigned short usRegCount;
								U32 u32Value;
								BOOL bWrtable;
								BOOL bVisible;
								BOOL bNonSafety;
								char const *pchStr;
								const signed short cbStrLen;
								const SolaType valuetype;
								const LPSOLAMULTIVALUE lpValueList;
								const int nValueListSize;} SOLAU32MAP, *LPSOLAU32MAP;
typedef struct __tagNumericUIParms {signed short ssValue; SolaType st;const TCHAR* szParmName;} NUMERICUIPARMS, *LPNUMERICUIPARMS;

	CSolaMBMap();
	CSolaMBMap(LPSOLAMBMAP p,int nSize);
	CSolaMBMap(LPSOLAU32MAP p,int nSize);
#if 0
	CSolaMBMap(LPSOLAMBMAP p, int nSize) : m_lpSolaMBMap(p), m_cbSize(nSize)
	{
		this->m_lpSolaU32Map = NULL;
	}
	CSolaMBMap(LPSOLAU32MAP p, int nSize) : m_lpSolaU32Map(p), m_cbSize(nSize)
	{
		this->m_lpSolaMBMap = NULL;
	}
#endif
	inline int GetSize(){return m_cbSize;};
	LPSOLAMBMAP GetLPMap(int index);
	LPSOLAU32MAP GetLPU32Map(int index);
	unsigned short GetStartRegAddr(int index);
	unsigned char GetFuncCode(int index);
	unsigned char GetFuncCode(unsigned short addr);
	const TCHAR* GetParmName(int index);
	const TCHAR* GetParmName(unsigned short addr);
	SolaType GetType(unsigned short addr);
	SolaType GetType(int index);
	short GetValue(unsigned short addr);
	short GetValue(int index);
	U32 GetU32Value(unsigned short addr);
	U32 GetU32Value(int index);
	short SetValue(unsigned short addr, short sValue);
	short SetValue(int index, short sValue);
	U32 SetU32Value(unsigned short addr, U32 u32Value);
	U32 SetU32Value(int index, U32 u32Value);
	LPSOLAMULTIVALUE GetLPMulti(unsigned short addr);
	LPSOLAMULTIVALUE GetLPMulti(int index);
	int GetMultiListSize(unsigned short addr);
	int CSolaMBMap::GetMultiListSize(int index);
	TCHAR* GetMultiValueItem(unsigned short usAddr, unsigned short usValue);
	TCHAR* GetMultiValueItem(int nIndex, unsigned short usValue);
	TCHAR* GetMultiValueItem(int nIndex, int nBitNumber);
	int GetMultiItemValue(unsigned short addr,int listindex);
	int GetMultiItemValue(int mapindex,int listindex);
	unsigned char* GetStr(int index);
	unsigned char* GetStr(unsigned short addr);
	std::unique_ptr<unsigned char> SetStr(int index, std::unique_ptr<char> pchStr, short cbLen);
	unsigned char* SetStr(unsigned short addr,unsigned char* pchStr,short cbLen);
	short GetcbStrLen(int index);
	short GetcbStrLen(unsigned short addr);
	BOOL GetWrtable(int nIndex);
	BOOL GetVisible(int nIndex);
	BOOL GetNonSafety(int nIndex);
	int GetSetBitNumber(int nIndex);
	unsigned short GetU32RegTotal();
	unsigned short GetRegRequestLen();
	inline int GetRegGroupSize(){return (int)m_cbSize;};
public:
	~CSolaMBMap(void);
private:
	LPSOLAMBMAP m_lpSolaMBMap;
	LPSOLAU32MAP m_lpSolaU32Map;
	int m_cbSize;
};
