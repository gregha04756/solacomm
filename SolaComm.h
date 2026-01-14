#pragma once

using namespace std;

#include "resource.h"

DWORD const SOLAREADSDTO = 5000L; /* Sola RTU direct connect disconnected if no data read for # of milliseconds */
DWORD const SOLAREADSGTO = 12000L; /* Sola RTU gateway connect disconnected if no data read for # of milliseconds */

extern "C++" const int i_SB_nWidth_factor;
extern "C++" const int i_SB_nMax_value;
extern "C++" const DWORD g_dwTimerInterval;
extern "C++" const DWORD g_dw1SecTimerInterval;

#define MAX_LOADSTRING 100

#define MBCRCSIZE 2
#define UNCONFIGUREDSENSOR1 0x7FFF
#define UNCONFIGUREDSENSOR2 0x8200
#define UNCONFIGUREDTEMP 0x8FFF
#define OUTSIDELOWRANGE 0x8400
//#define SOLAREADTIMEOUT 5000 /* Sola disconnected if no data read for # of milliseconds */
//#define SOLAREADTIMEOUT 16 /* Sola disconnected if no data read for # of ticks (approx. 15 milliseconds/tick) */
#define NUMRWEVENTS 3
#define NUMPOLLEVENTS 2
#define CELSIUSUNITS 1
#define FAHRENHEITUNITS 0

#define WM_APPTIMER				WM_APP + 100
#define WM_APPTIMER5			WM_APP + 101
#define WM_APPTRENDUPD			WM_APP + 102
#define WM_APPRWHWNDUPDATE		WM_APP + 103
#define WM_APPQUITRW			WM_APP + 104
#define WM_APPRWQUIT			WM_APP + 105
#define WM_APPREADHRLIST		WM_APP + 106
#define WM_APPCHNGCOM			WM_APP + 107
#define WM_APPCHCONFIGREADDONE	WM_APP + 108
#define WM_APPREADHRSNGL		WM_APP + 109
#define WM_APPCONNUPD			WM_APP + 110
#define WM_APPCHCONFIGUPD		WM_APP + 111
#define WM_APPREADERR			WM_APP + 112
#define WM_APPREADOK			WM_APP + 113
#define WM_APPALERTUPD			WM_APP + 114
#define WM_APPDATAUPDSTART		WM_APP + 115
#define WM_APPSYSCONFIGUPD		WM_APP + 116
#define WM_APPSYSTEMIDUPD		WM_APP + 117
#define WM_APPMBRESPONSE		WM_APP + 118
#define WM_APPODRESETCFGUPD		WM_APP + 119
#define WM_APPODRESETLINEUPD	WM_APP + 120
#define WM_APPODRESETLINEQUIT	WM_APP + 121
#define WM_APPDHWCONFIGUPD		WM_APP + 122
#define WM_APPLLSTATUSUPD		WM_APP + 123
#define WM_APPLLCONFIGUPD		WM_APP + 124
#define WM_APPENDNOTICETHREAD	WM_APP + 125
#define WM_APPSOLAIDUPD			WM_APP + 126
#define WM_APPLOCKOUTUPD		WM_APP + 127
#define WM_APPENDLOCKOUTTHREAD	WM_APP + 128
#define WM_APPMBSOCKETEVENT		WM_APP + 129
#define WM_APPCREATEANDCONNECT	WM_APP + 130
#define WM_APPUSERQUIT			WM_APP + 131
#define WM_APPSOLAPOLLABORT		WM_APP + 132
#define WM_APPMODCONFIGUPD		WM_APP + 133
#define WM_APPBCCONFIGUPD		WM_APP + 134
#define WM_APPLIMCONFIGUPD		WM_APP + 135
#define WM_APPPUMPCONFIGUPD		WM_APP + 136
#define WM_APPSOLACONNECTING	WM_APP + 137
#define WM_APPENDPOLLINGDLGTHREAD	WM_APP + 138
#define WM_APPQUITPOLLINGDLG	WM_APP + 139
#define WM_APPSAVERESTOREUPD	WM_APP + 140
#define WM_APPTESTINGDLGENDING	WM_APP + 142
#define WM_APPTIMER1			WM_APP + 143

enum PageType {
	NoPage,
	TrendPage,
	StatusPage,
	ConfigPage
};

enum MBConnType {
	RTU_Serial_Direct,
	RTU_Serial_Gateway,
	TCP
};

enum TCP_Gateway_Type {
	TCP_Protonode=0,
	TCP_Lantronix=1,
	TCP_Other=2,
	TCP_Undefined=3
};

enum RTUCommResult {
	NoError,
	Timeout,
	HardError,
	CRCError
};

typedef unsigned short U16;
typedef unsigned long U32;

typedef struct __tagMBSndRcvReq {	HWND hPage;
									UINT nMsg;
									unsigned char* pchSndBuf;
									unsigned char** ppchToSnd;
									unsigned char** ppchEndSnd;
									int nSndBufSize;
									unsigned char* pchRcvBuf;
									unsigned char** ppchToRcv;
									unsigned char** ppchEndRcv;
									int nRcvBufSize;} MBSNDRCVREQ, *LPMBSNDRCVREQ;

typedef struct __tagPageUpdate { HWND hPage; UINT nMsg; } PAGEUPDATE, *LPPAGEUPDATE;

typedef struct __tagPageDataEvent { PageType typePage; HANDLE hEvent; } PAGEDATAEVENT, *LPPAGEDATAEVENT;

typedef struct __tagAlertDesc { const int nAlertCode; const TCHAR* szAlertText; } SOLAALERTDESC, *LPSOLAALERTDESC;

typedef struct __tagLockoutDesc { const int nLockoutCode; const TCHAR* szLockoutText; } SOLALOCKOUTDESC, *LPSOLALOCKOUTDESC;
										
typedef struct __tagSolaMultiValue { const int nValue; const TCHAR* szString; } SOLAMULTIVALUE, *LPSOLAMULTIVALUE;

typedef struct __tagSolaMultiVals { const LPSOLAMULTIVALUE lpSolaMultiValue; const int nSize;  } SOLAMULTIVALS, *LPSOLAMULTIVALS;

typedef struct __tagSolaAlertRecord {	unsigned short usAlertCode;
										unsigned long ulCycle;
										unsigned long ulHours;
										unsigned char uchUnused;
										unsigned char uchCount; } SOLAALERTRECORD, *LPSOLAALERTRECORD;

typedef union __tagSolaAlertUnion {		unsigned char cc[12];
										SOLAALERTRECORD aa; } SOLAALERTUNION, *LPSOLAALERTUNION;

typedef struct __tagSolaAnnuncRecord {	unsigned short usLocation;
										unsigned char uchShortname[3];
										unsigned char uchUnused;
										unsigned char uchName[20];} SOLAANNUNCRECORD, *LPSOLAANNUNCRECORD;

typedef struct __tagSolaAnnuncShort {	unsigned char uchShortname[3];
										unsigned char uchUnused;
										unsigned char uchName[20];} SOLAANNUNCSHORT, *LPSOLAANNUNCSHORT;

typedef union __tagSolaAnnuncUnion {unsigned char cc[sizeof(SOLAANNUNCRECORD)];
									SOLAANNUNCRECORD aa;
									SOLAANNUNCSHORT as;} SOLAANNUNCUNION, *LPSOLAANNUNCUNION;

typedef struct __tagSolaAlert {	unsigned char uchDevAddr;
								const unsigned char uchFuncCode;
								const unsigned short usStartRegAddr;
								const unsigned short usRegCount;
								LPSOLAALERTUNION pAlertRecord; } SOLAALERT, *LPSOLAALERT;

typedef struct __tagSolaLockoutRecord {	unsigned short usLockoutCode;			//0-1
										unsigned short usAnnunciatorFirstOut;	//2-3
										unsigned short usBurnerControlState;	//4-5
										unsigned short usSequenceTime;			//6-7
										unsigned long ulCycle;					//8-11
										unsigned long ulHours;					//12-15
										unsigned short usIO;					//16-17
										unsigned short usAnnunciator;			//18-19
										unsigned short usOutletTemperature;		//20-21
										unsigned short usInletTemperature;		//22-23
										unsigned short usDHWTemperature;		//24-25
										unsigned short usODTemperature;			//26-27
										unsigned short usStackTemperature;		//28-29
										unsigned short us4to20mAInput;			//30-31
										unsigned char ucFaultData0;				//32
										unsigned char ucFaultData1;				//33
} SOLALOCKOUTRECORD, *LPSOLALOCKOUTRECORD;

typedef union __tagSolaLockoutUnion {	unsigned char cc[sizeof(SOLALOCKOUTRECORD)];
										SOLALOCKOUTRECORD slr; } SOLALOCKOUTUNION, *LPSOLALOCKOUTUNION;

typedef struct __tagSolaLockout {	unsigned char uchDevAddr;
									const unsigned char uchFuncCode;
									const unsigned short usStartRegAddr;
									const unsigned short usRegCount;
									LPSOLALOCKOUTUNION pLockoutUnion; } SOLALOCKOUT, *LPSOLALOCKOUT;

typedef struct _tagRWThreadParms	{	HANDLE hCOM;
										unsigned char uchSolaAddr;
										HWND hParentDlg;
										LPCRITICAL_SECTION pCommCritSect;
										LPCRITICAL_SECTION pDataCritSect;
										HANDLE hReadEvent;
										HANDLE hQuitEvent; } RWTHREADPARMS, *LPRWTHREADPARMS;

typedef struct _tagTimerThreadParms { HWND hParentWnd; HANDLE hQuitEvent; int cchTimeString; } TimerThreadParms, *LPTimerThreadParms;
typedef struct _tagTimerParms { TCHAR* szTimeString; } TimerParms, *LPTimerParms;
typedef struct _tagODResetLineParms { HWND hODResetWnd; } ODRESETLINEPARMS, *LPODRESETLINEPARMS;
typedef struct _tagSummaryThreadParms { HWND hParentWnd;
										LPCRITICAL_SECTION pDataCritSect;
										HANDLE hReadEvent;
										HANDLE hQuitEvent;
										int nPg; } SUMMARYTHREADPARMS, *LPSUMMARYTHREADPARMS;

typedef struct _tagLockoutRecordDspParms {int nIndex;} LockoutRecordDspParms, *LPLOCKOUTRECORDDSPPARMS;

typedef struct __tagHoldingRegister {	unsigned char uchDeviceFuncCode[3];
										unsigned char uchValueHi;
										unsigned char uchValueLo; } HOLDINGREG;

typedef struct __tagSolaHRResponse {	unsigned char uchAddress;
										unsigned char uchFuncCode;
										unsigned char cbByteCount; } SOLAHRRESPONSE;

typedef struct __tagSolaIDResponse {	unsigned char SolaAddr;
										unsigned char FunctionCode;
										unsigned char ByteCount;
										unsigned char SolaID;
										unsigned char RunIndicator;
										char OSNumber[16];
										char BurnerName[20]; } SOLAIDRESPONSE, *LPSOLAIDRESPONSE;

typedef struct __tagStringUIParms {TCHAR* szTxt; size_t cchLen; const TCHAR* szParmName;} STRINGUIPARMS, *LPSTRINGUIPARMS;
typedef struct __tagMultiUIParms {LPSOLAMULTIVALUE lpMultiList; int nCurSel; int nMultiListSize; const TCHAR* szParmName;} MULTIUIPARMS, *LPMULTIUIPARMS;
typedef struct __tagBitMaskUIParms {unsigned short usValue;LPSOLAMULTIVALUE lpMulti;int nMultiSize;TCHAR* szParmName;} BITMASKUIPARMS, *LPBITMASKUIPARMS;
typedef struct __tagSolaODRCurvePoint {signed short x; signed short y;} SOLAODRCURVEPT, *LPSOLAODRCURVEPT;
typedef struct __tagMBServerIPParms {unsigned long ulIPAddress;} MBSERVERIPPARMS, *LPMBSERVERIPPARMS;
typedef struct __tagLoadFileParms {std::ifstream* lpifile;unsigned short* lpparms;short* lpvals;} LOADFILEPARMS, *LPLOADFILEPARMS;
typedef struct __tagCOMPortParm { int i_COM_port; } COMPORTPARMS, *LPCOMPORTPARMS;

char* const chHoldRegReqID = "struct __tagSolaMBMap *";
char* const chAlertLogReqID = "struct __tagSolaAlert *";
char* const chLockoutLogReqID = "struct __tagSolaLockout *";

char* const MBReqTypeIDs[] = {chHoldRegReqID,chAlertLogReqID,chLockoutLogReqID};

enum MBReqType {
	LPHoldRegReq,
	LPAlertLogReq,
	LPLockoutLogReq,
	LPSolaIDReq,
	WriteSingleRegReq,
	WriteMultiRegReq,
	InvalidMBReqType,
	LPInitIDreq
};

typedef DWORD (*sctf)(LPARAM lp);
typedef struct __tagTCPConnectInfo {u_long* lpulIPAddress; char* chPort; int cbPort;} TCPCONNECTINFO, *LPTCPCONNECTINFO;

#pragma pack(1)
typedef struct __tagMBAPHeader { unsigned short usTransactionIdentifier;
								unsigned short usProtocolIdentifier;
								unsigned short usLength;
								unsigned char uchUnitIdentifier; } MBAPHEADER, *LPMBAPHEADER;

#pragma pack(1)
typedef struct __tagMBRequest { unsigned char uchFunctionCode;
								unsigned short usStartAddr;
								unsigned short usRegisterCnt; } MBREQUEST, *LPMBREQUEST;

#pragma pack(1)
typedef struct __tagMBMessage { LPVOID lpMap; MBReqType rt; DWORD dwTimeInTicks; MBAPHEADER mbaphdr; MBREQUEST mbr; } MBMESSAGE, *LPMBMESSAGE;

#pragma pack(1)
typedef struct __tagMBResponse { unsigned char uchFunctionCode;
								unsigned short uscbRespLen;
								char* chResponse; } MBRESPONSE, *LPMBRESPONSE;

#pragma pack(1)
typedef struct __tagMBRespMessage { MBAPHEADER mbaphdr;MBRESPONSE mbr; } MBRESPMESSAGE, *LPMBRESPMESSAGE;
