#pragma once

#include "stdafx.h"
#include "solacomm.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "NoticeDialog.h"
#include "SolaChangeCode.h"
#include "SolaAlert.h"
#include "SolaLockout.h"
#include "SolaMultiValue.h"
#include "SolaPage.h"
#include <iostream>
#include <fstream>

using namespace std;

#define SOLAPRODUCTID 0x79
#define SOLABURNERNAMELEN 20
#define SOLAINSTDATALEN 20
#define SOLAOEMIDLEN 20
#define SOLAOSNUMBERLEN 16
#define IDT_TIMER1 1

extern "C++" HINSTANCE g_hInst;
extern "C++" TCHAR szTitle[];
extern "C++" BOOL g_bQuit;
extern "C++" BOOL bSolaConnected;
extern "C++" int g_nActivePages;
extern "C++" int g_nActiveTrendPages;
extern "C++" int g_nActiveStatusPages;
extern "C++" int g_nActiveConfigPages;
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" HANDLE g_hReadQuitEvent;
extern "C++" CRITICAL_SECTION gRWDataCritSect;
extern "C++" int g_nActiveTrendPages;
extern "C++" int g_nActiveStatusPages;
extern "C++" int g_nActiveConfigPages;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" DWORD g_dwTotalSent;
extern "C++" DWORD g_dwTotalRcvd;
extern "C++" DWORD g_dwTotalCRCErrors;
extern "C++" LARGE_INTEGER g_liPollTime;
extern "C++" DWORD g_dwConnectTime;
extern "C++" double g_dErrorRate;
extern "C++" HANDLE g_hReSyncReqEvent;
extern "C++" CRITICAL_SECTION gCOMCritSect;
extern "C++" CRITICAL_SECTION gTimeCritSect;
extern "C++" HANDLE g_hReSyncReqEvent;

extern "C++" unsigned char SOLAMBAddress;
extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcModConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap *pc_Dup_Burner_Name;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap *pc_Dup_Installation_Data;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap *pc_Dup_OEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcODResetConfig;
extern "C++" CSolaMBMap* pcDHWConfiguration;
extern "C++" CSolaMBMap* pcLLStatus;
extern "C++" CSolaMBMap* pcXLLStatus;
extern "C++" CSolaMBMap* pcLLConfig;
extern "C++" CSolaMBMap* pcXSystemConfig;
extern "C++" CSolaMBMap* pcBurnerControlStatus;
extern "C++" CSolaMBMap* pcTrendStatus;
extern "C++" CSolaMBMap* pcSystemStatus;
extern "C++" CSolaMBMap* pcSensorStatus;
extern "C++" CSolaMBMap* pcDemandModulationStatus;
extern "C++" CSolaMBMap* pcCHStatus;
extern "C++" CSolaMBMap* pcDHWStatus;
extern "C++" CSolaMBMap* pcPumpStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
extern "C++" CSolaMBMap* pcSystemIDOSNumber;
extern "C++" CSolaMBMap* pcSystemIDDateCode;
extern "C++" CSolaMBMap* pcSystemIDSafetyProcesorBuild;
extern "C++" CSolaMBMap* pcSystemIDAppProcessorBuild;
extern "C++" CSolaMBMap* pcAlarmCode;
extern "C++" CSolaMBMap* pcStatistics;
extern "C++" CSolaMBMaps* pcTrendMaps;
extern "C++" CSolaMBMaps* pcAllSolaMaps;
extern "C++" CSolaMBMaps* pcStatusMaps;
extern "C++" CSolaMBMaps* pcSystemIDMaps;
extern "C++" CSolaMBMap* pcAnnuncConfigGen;
extern "C++" CSolaMBMap* pcAnnuncConfig1;
extern "C++" CSolaMBMap* pcAnnuncConfig2;
extern "C++" CSolaMBMap* pcAnnuncConfig3;
extern "C++" CSolaMBMap* pcAnnuncConfig4;
extern "C++" CSolaMBMap* pcAnnuncConfig5;
extern "C++" CSolaMBMap* pcAnnuncConfig6;
extern "C++" CSolaMBMap* pcAnnuncConfig7;
extern "C++" CSolaMBMap* pcAnnuncConfig8;
extern "C++" CSolaMBMap* pcPIIAnnuncConfig;
extern "C++" CSolaMBMap* pcLCIAnnuncConfig;
extern "C++" CSolaMBMap* pcILKAnnuncConfig;
extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerControlStatusValues;
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" CSolaAlert* pcAlertLog;
extern "C++" unsigned char chMBSndBuf[64];
extern "C++" unsigned char chMBRcvBuf[64];
extern "C++" std::queue<MBSNDRCVREQ> g_MBSndRcvReqQ;
extern "C++" int nUpdCount;
extern "C++" CSolaChangeCode* pcConfigChangeCodes;
extern "C++" CSolaChangeCode* pcStatusChangeCodes;
extern "C++" std::list<CSolaMBMap*> *p_Reg_Group_List;
extern "C++" std::list<CSolaMBMap*> *Make_Reg_Group_List(CSolaPage* p_page);
extern "C++" HWND g_hPropSheet;

class CSolaTCPComm
{
public:
	CSolaTCPComm(void);
	CSolaTCPComm(HWND hwndParent,enum TCP_Gateway_Type gw);
	~CSolaTCPComm(void);
	BOOL IsConnected();
	BOOL IsSolaConnected();
	int CreateSocket(addrinfo* lpAddrInfo);
	inline TCHAR* GetBurnerName() {return m_szBurnerName;};
	inline TCHAR* GetOSNumber() {return m_szOSNumber;};
private:
	enum TCP_Gateway_Type m_gwtype;
	uint8_t m_ui8fc;
	unsigned short m_usti;
	BOOL m_bQuit;
	BOOL m_bConnected;
	BOOL m_bSolaConnected;
	SOCKET m_MBSocket;
	HWND m_hwndParent;
	BOOL Set_Sola_Connected(BOOL b_connect_status);
	inline uint8_t Get_MB_Function_Code() {return m_ui8fc;};
	inline enum TCP_Gateway_Type Get_Gateway_Type() {return m_gwtype;};
	static DWORD WINAPI SolaPollThread(LPARAM lParam);
	static BOOL ProcessResponse(CSolaTCPComm* p);
	static DWORD WINAPI TCPSocketThread(LPARAM lParam);
	static LRESULT CALLBACK TCPSocketWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
	HWND m_hwndSolaTCPSocketWnd;
	HANDLE m_hTCPSocketThread;
	HANDLE m_hdupTCPSocketThread;
	DWORD m_dwTCPSocketThreadID;
	HANDLE m_hSolaPollThread;
	HANDLE m_hdupSolaPollThread;
	HANDLE m_hQuitEvent;
	HANDLE m_hTimerEvent;
	HANDLE m_hResponseEvent;
	DWORD m_dwSolaPollThreadID;
	addrinfo* m_lpAddrInfo;
	TCHAR* m_szBurnerName;
	TCHAR* m_szOSNumber;
	TCHAR* m_szOEMID;
	LPSOLAIDRESPONSE m_lpSolaID;
	LPCRITICAL_SECTION m_lpReqCritSect;
	LPCRITICAL_SECTION m_lpRespCritSect;
	std::list<MBMESSAGE>* m_lplReqMBMsgs;
	std::list<MBMESSAGE>::iterator m_itReqMBMsgs;
	std::list<MBRESPMESSAGE>* m_lplRespMBMsgs;
	std::list<MBRESPMESSAGE>::iterator m_itRespMBMsgs;
	static DWORD ReadSolaMap(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaMBMap *lp_map,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect);
	static DWORD ReadSolaMaps(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaMBMaps *lp_maps,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect,
		CNoticeDialog *p_nd);
	static DWORD Read_Alert_Log(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaAlert *lp_map,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect,
		CNoticeDialog *p_nd);
	static DWORD Read_Lockout_Log(
		CSolaTCPComm *p_this,
		unsigned char const ucMBAddr,
		CSolaLockout *lp_map,
		LPMBMESSAGE lp_mbm,
		MBReqType req_type,
		LPHANDLE ph_events,
		int i_ec,
		unsigned long &ulSecsSinceLastResp,
		BOOL &b_reconnect,
		CNoticeDialog *p_nd);
#if TCPDEBUGGING
	ofstream* m_lpdbgfile;
	LPCRITICAL_SECTION m_lpDbgFileCritSect;
	static int ReqFormat(LPMBMESSAGE lp,char* buf,int cbbufsiz);
	static int RespFormat(LPMBRESPMESSAGE lp,char* buf,int cbbufsiz);
	static int CSolaTCPComm::RespFormat(std::list<MBRESPMESSAGE>::iterator ity,char* buf,int cbbufsiz);
#endif
};
