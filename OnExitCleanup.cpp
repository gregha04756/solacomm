
#include "stdafx.h"
#include "SolaComm.h"
#include "SolaMBMap.h"
#include "SolaMBMaps.h"
#include "SolaLockout.h"
#include "SolaAlert.h"
#include "SolaPage.h"
#include "SolaChangeCode.h"

extern "C++" CSolaMBMap* pcCHConfiguration;
extern "C++" CSolaMBMap* pcModConfiguration;
extern "C++" CSolaMBMap* pcSystemIDProductType;
extern "C++" CSolaMBMap* pcSystemIDBurnerName;
extern "C++" CSolaMBMap* pcSystemIDInstallationData;
extern "C++" CSolaMBMap* pcSystemIDOEMID;
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
extern "C++" CSolaMBMap* pcExtendedSensorStatus;
extern "C++" CSolaMBMap* pcDemandModulationStatus;
extern "C++" CSolaMBMap* pcCHStatus;
extern "C++" CSolaMBMap* pcDHWStatus;
extern "C++" CSolaMBMap* pcPumpStatus;
extern "C++" CSolaMBMap* pcSystemConfiguration;
extern "C++" CSolaMBMap* pcAlarmCode;
extern "C++" CSolaMBMap* pcStatistics;
extern "C++" CSolaMBMaps* pcTrendMaps;
extern "C++" CSolaMBMaps* pcAllSolaMaps;
extern "C++" CSolaMBMaps* pcSystemIDMaps;
extern "C++" CSolaMultiValue* pcBurnerControlStateValues;
extern "C++" CSolaMultiValue* pcBurnerControlStatusValues;
extern "C++" CSolaLockout* pcLockoutLog;
extern "C++" CSolaAlert* pcAlertLog;
extern "C++" CSolaPage* pcSummaryPage;
extern "C++" HANDLE* g_hPageUpdEvents;
extern "C++" LPPAGEDATAEVENT g_lpPageDataEvents;
extern "C++" CSolaMBMap* pcBurnerControlConfig;
extern "C++" CSolaPage* pcBCConfigPage;
extern "C++" std::list<CSolaMBMap*>* p_Reg_Group_List;
extern "C++" unsigned char* g_pMBResponse;
extern "C++" unsigned char* g_pMBRequest;
extern "C++" CSolaChangeCode* pcStatusChangeCodes;
extern "C++" CSolaChangeCode* pcConfigChangeCodes;

extern "C++" CSolaMBMap* pc_Dup_Burner_Name;
extern "C++" CSolaMBMap* pc_Dup_Installation_Data;
extern "C++" CSolaMBMap* pc_Dup_OEMID;
extern "C++" CSolaMBMap* pcAnnuncConfig1;
extern "C++" CSolaMBMap* pcAnnuncConfig2;
extern "C++" CSolaMBMap* pcAnnuncConfig3;
extern "C++" CSolaMBMap* pcAnnuncConfig4;
extern "C++" CSolaMBMap* pcAnnuncConfig5;
extern "C++" CSolaMBMap* pcAnnuncConfig6;
extern "C++" CSolaMBMap* pcAnnuncConfig7;
extern "C++" CSolaMBMap* pcAnnuncConfig8;
extern "C++" CSolaMBMap* pcAnnuncConfigGen;
extern "C++" CSolaMBMap* pcConnectorConfig;
extern "C++" CSolaMBMap* pcDHWStorageConfig;
extern "C++" CSolaMBMap* pcFanConfig;
extern "C++" CSolaMBMap* pcFlapValveConfig;
extern "C++" CSolaMBMap* pcFrostProtConfig;
extern "C++" CSolaMBMap* pcILKAnnuncConfig;
extern "C++" CSolaMBMap* pcLCIAnnuncConfig;
extern "C++" CSolaMBMap* pcLimitsConfig;
extern "C++" CSolaMBMap* pcPIIAnnuncConfig;
extern "C++" CSolaMBMap* pcPumpConfig;
extern "C++" CSolaMBMap* pcSystemIDResetRestart;
extern "C++" CSolaMBMap* pcX2CHConfig;
extern "C++" CSolaMBMap* pcX2ModConfig;
extern "C++" CSolaMBMap* pcXCHConfig;
extern "C++" CSolaMBMap* pcXLimitsConfig;
extern "C++" CSolaMBMap* pcXLLConfig;
extern "C++" CSolaMBMap* pcXModConfig;
extern "C++" CSolaMBMaps* pcAnnuncMaps;
extern "C++" CSolaMBMaps* pcStatusMaps;
extern "C++" CSolaMultiValue* pcBurnerAlarmReasonCodes;
extern "C++" CSolaMultiValue* pcBurnerAnnunFirstOutCodes;
extern "C++" CSolaMultiValue* pcBurnerAnnunHoldCodes;
extern "C++" CSolaMultiValue* pcBurnerControlFlags;
extern "C++" CSolaMultiValue* pcBurnerRemoteStatCodes;
extern "C++" CSolaMultiValue* pcDHWEnableList;
extern "C++" CSolaMultiValue* pcDigitalIOCodes;
extern "C++" CSolaMultiValue* pcRegisterAccess;
extern "C++" CSolaMultiValue* pcSolaLockoutDesc;
extern "C++" CSolaPage* pc_Digital_IO_Page;
extern "C++" CSolaPage* pcCHConfigPage;
extern "C++" CSolaPage* pcDHWConfigPage;
extern "C++" extern "C++" CSolaPage* pcLimitsConfigPage;
extern "C++" CSolaPage* pcLLConfigPage;
extern "C++" CSolaPage* pcModConfigPage;
extern "C++" CSolaPage* pcODResetConfigPage;
extern "C++" CSolaPage* pcPumpConfigPage;
extern "C++" CSolaPage* pcSaveRestorePage;
extern "C++" CSolaPage* pcSystemIDPage;

BOOL OnExitCleanup(void)
{
	BOOL b_r = true;
	int iNdx = 0;

	if (!(NULL == pcCHConfiguration))
	{
		delete pcCHConfiguration;
		pcCHConfiguration = NULL;
	}
	if (!(NULL == pcModConfiguration))
	{
		delete pcModConfiguration;
		pcModConfiguration = NULL;
	}
	if (!(NULL == pcSystemIDProductType))
	{
		delete pcSystemIDProductType;
		pcSystemIDProductType = NULL;
	}
	if (!(NULL ==  pcSystemIDBurnerName))
	{
		for (iNdx = 0; iNdx < pcSystemIDBurnerName->GetSize(); iNdx++)
		{
			if (!(NULL == pcSystemIDBurnerName->GetLPMap(iNdx)))
			{
				delete[] pcSystemIDBurnerName->GetLPMap(iNdx)->pchStr;
				pcSystemIDBurnerName->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcSystemIDBurnerName;
		pcSystemIDBurnerName = NULL;
	}
	if (!(NULL ==  pcSystemIDInstallationData))
	{
		for (iNdx = 0; iNdx < pcSystemIDInstallationData->GetSize(); iNdx++)
		{
			if (!(NULL == pcSystemIDInstallationData->GetLPMap(iNdx)))
			{
				delete[] pcSystemIDInstallationData->GetLPMap(iNdx)->pchStr;
				pcSystemIDInstallationData->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcSystemIDInstallationData;
		pcSystemIDInstallationData = NULL;
	}
	if (!(NULL ==  pcSystemIDOEMID))
	{
		for (iNdx = 0; iNdx < pcSystemIDOEMID->GetSize(); iNdx++)
		{
			if (!(NULL == pcSystemIDOEMID->GetLPMap(iNdx)))
			{
				delete[] pcSystemIDOEMID->GetLPMap(iNdx)->pchStr;
				pcSystemIDOEMID->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcSystemIDOEMID;
		pcSystemIDOEMID = NULL;
	}
	if (!(NULL ==  pcSystemIDOSNumber))
	{
		for (iNdx = 0; iNdx < pcSystemIDOSNumber->GetSize(); iNdx++)
		{
			if (!(NULL == pcSystemIDOSNumber->GetLPMap(iNdx)))
			{
				delete[] pcSystemIDOSNumber->GetLPMap(iNdx)->pchStr;
				pcSystemIDOSNumber->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcSystemIDOSNumber;
		pcSystemIDOSNumber = NULL;
	}
	if (!(NULL ==  pcSystemIDDateCode))
	{
		for (iNdx = 0; iNdx < pcSystemIDDateCode->GetSize(); iNdx++)
		{
			if (!(NULL == pcSystemIDDateCode->GetLPMap(iNdx)))
			{
				delete[] pcSystemIDDateCode->GetLPMap(iNdx)->pchStr;
				pcSystemIDDateCode->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcSystemIDDateCode;
		pcSystemIDDateCode = NULL;
	}
	if (!(NULL ==  pcSystemIDSafetyProcesorBuild))
	{
		delete pcSystemIDSafetyProcesorBuild;
		pcSystemIDSafetyProcesorBuild = NULL;
	}
	if (!(NULL ==  pcSystemIDAppProcessorBuild))
	{
		delete pcSystemIDAppProcessorBuild;
		pcSystemIDAppProcessorBuild = NULL;
	}
	if (!(NULL ==  pcODResetConfig))
	{
		delete pcODResetConfig;
		pcODResetConfig = NULL;
	}
	if (!(NULL ==  pcDHWConfiguration))
	{
		delete pcDHWConfiguration;
		pcDHWConfiguration = NULL;
	}
	if (!(NULL ==  pcLLStatus))
	{
		delete pcLLStatus;
		pcLLStatus = NULL;
	}
	if (!(NULL ==  pcXLLStatus))
	{
		delete pcXLLStatus;
		pcXLLStatus = NULL;
	}
	if (!(NULL ==  pcLLConfig))
	{
		delete pcLLConfig;
		pcLLConfig = NULL;
	}
	if (!(NULL ==  pcXSystemConfig))
	{
		delete pcXSystemConfig;
		pcXSystemConfig = NULL;
	}
	if (!(NULL ==  pcBurnerControlStatus))
	{
		delete pcBurnerControlStatus;
		pcBurnerControlStatus = NULL;
	}
	if (!(NULL ==  pcTrendStatus))
	{
		delete pcTrendStatus;
		pcTrendStatus = NULL;
	}
	if (!(NULL ==  pcSystemStatus))
	{
		delete pcSystemStatus;
		pcSystemStatus = NULL;
	}
	if (!(NULL ==  pcSensorStatus))
	{
		delete pcSensorStatus;
		pcSensorStatus = NULL;
	}
	if (!(NULL ==  pcExtendedSensorStatus))
	{
		delete pcExtendedSensorStatus;
		pcExtendedSensorStatus = NULL;
	}
	if (!(NULL ==  pcDemandModulationStatus))
	{
		delete pcDemandModulationStatus;
		pcDemandModulationStatus = NULL;
	}
	if (!(NULL ==  pcCHStatus))
	{
		delete pcCHStatus;
		pcCHStatus = NULL;
	}
	if (!(NULL ==  pcDHWStatus))
	{
		delete pcDHWStatus;
		pcDHWStatus = NULL;
	}
	if (!(NULL ==  pcPumpStatus))
	{
		delete pcPumpStatus;
		pcPumpStatus = NULL;
	}
	if (!(NULL ==  pcSystemConfiguration))
	{
		delete pcSystemConfiguration;
		pcSystemConfiguration = NULL;
	}
	if (!(NULL ==  pcAlarmCode))
	{
		delete pcAlarmCode;
		pcAlarmCode = NULL;
	}
	if (!(NULL ==  pcStatistics))
	{
		delete pcStatistics;
		pcStatistics = NULL;
	}
	if (!(NULL ==  pcTrendMaps))
	{
		delete pcTrendMaps;
		pcTrendMaps = NULL;
	}
	if (!(NULL ==  pcAllSolaMaps))
	{
		delete pcAllSolaMaps;
		pcAllSolaMaps = NULL;
	}
	if (!(NULL ==  pcSystemIDMaps))
	{
		delete pcSystemIDMaps;
		pcSystemIDMaps = NULL;
	}
	if (!(NULL ==  pcBurnerControlStateValues))
	{
		delete pcBurnerControlStateValues;
		pcBurnerControlStateValues = NULL;
	}
	if (!(NULL ==  pcLockoutLog))
	{
		for (iNdx = 0; iNdx < pcLockoutLog->GetSize(); iNdx++)
		{
			delete pcLockoutLog->GetLPMap(iNdx)->pLockoutUnion;
			pcLockoutLog->GetLPMap(iNdx)->pLockoutUnion = NULL;
		}
		delete pcLockoutLog;
		pcLockoutLog = NULL;
	}
	if (!(NULL ==  pcAlertLog))
	{
		for (iNdx = 0; iNdx < pcAlertLog->GetSize(); iNdx++)
		{
			delete pcAlertLog->GetLPMap(iNdx)->pAlertRecord;
			pcAlertLog->GetLPMap(iNdx)->pAlertRecord = NULL;
		}
		delete pcAlertLog;
		pcAlertLog = NULL;
	}
	if (!(NULL ==  pcSummaryPage))
	{
		delete pcSummaryPage;
		pcSummaryPage = NULL;
	}
	if (!(NULL ==  pcBurnerControlConfig))
	{
		delete pcBurnerControlConfig;
		pcBurnerControlConfig = NULL;
	}
	if (!(NULL ==  pcBCConfigPage))
	{
		delete pcBCConfigPage;
		pcBCConfigPage = NULL;
	}
	if (!(NULL == pcBurnerControlStatusValues))
	{
		delete pcBurnerControlStatusValues;
		pcBurnerControlStatusValues = NULL;
	}
	if (!(NULL == p_Reg_Group_List))
	{
		delete p_Reg_Group_List;
		p_Reg_Group_List = NULL;
	}
	if (!(NULL == g_pMBResponse))
	{
		delete[] g_pMBResponse;
		g_pMBResponse = NULL;
	}
	if (!(NULL == g_pMBRequest))
	{
		delete[] g_pMBRequest;
		g_pMBRequest = NULL;
	}
	if (!(NULL == pcStatusChangeCodes))
	{
		delete pcStatusChangeCodes;
		pcStatusChangeCodes = NULL;
	}
	if (!(NULL == pcConfigChangeCodes))
	{
		delete pcConfigChangeCodes;
		pcConfigChangeCodes = NULL;
	}



	if (!(NULL == pc_Dup_Burner_Name))
	{
		for (iNdx = 0; iNdx < pc_Dup_Burner_Name->GetSize(); iNdx++)
		{
			if (!(NULL == pc_Dup_Burner_Name->GetLPMap(iNdx)))
			{
				delete[] pc_Dup_Burner_Name->GetLPMap(iNdx)->pchStr;
				pc_Dup_Burner_Name->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pc_Dup_Burner_Name;
		pc_Dup_Burner_Name = NULL;
	}
	if (!(NULL == pc_Dup_Installation_Data))
	{
		for (iNdx = 0; iNdx < pc_Dup_Installation_Data->GetSize(); iNdx++)
		{
			if (!(NULL == pc_Dup_Installation_Data->GetLPMap(iNdx)))
			{
				delete[] pc_Dup_Installation_Data->GetLPMap(iNdx)->pchStr;
				pc_Dup_Installation_Data->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pc_Dup_Installation_Data;
		pc_Dup_Installation_Data = NULL;
	}
	if (!(NULL == pc_Dup_OEMID))
	{
		for (iNdx = 0; iNdx < pc_Dup_OEMID->GetSize(); iNdx++)
		{
			if (!(NULL == pc_Dup_OEMID->GetLPMap(iNdx)))
			{
				delete[] pc_Dup_OEMID->GetLPMap(iNdx)->pchStr;
				pc_Dup_OEMID->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pc_Dup_OEMID;
		pc_Dup_OEMID = NULL;
	}
	if (!(NULL == pcAnnuncConfig1))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig1->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig1->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig1->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig1->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig1;
		pcAnnuncConfig1 = NULL;
	}
	if (!(NULL == pcAnnuncConfig2))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig2->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig2->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig2->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig2->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig2;
		pcAnnuncConfig2 = NULL;
	}
	if (!(NULL == pcAnnuncConfig3))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig3->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig3->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig3->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig3->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig3;
		pcAnnuncConfig3 = NULL;
	}
	if (!(NULL == pcAnnuncConfig4))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig4->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig4->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig4->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig4->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig4;
		pcAnnuncConfig4 = NULL;
	}
	if (!(NULL == pcAnnuncConfig5))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig5->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig5->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig5->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig5->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig5;
		pcAnnuncConfig5 = NULL;
	}
	if (!(NULL == pcAnnuncConfig6))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig6->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig6->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig6->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig6->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig6;
		pcAnnuncConfig6 = NULL;
	}
	if (!(NULL == pcAnnuncConfig7))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig7->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig7->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig7->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig7->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig7;
		pcAnnuncConfig7 = NULL;
	}
	if (!(NULL == pcAnnuncConfig8))
	{
		for (iNdx = 0; iNdx < pcAnnuncConfig8->GetSize(); iNdx++)
		{
			if (!(NULL == pcAnnuncConfig8->GetLPMap(iNdx)))
			{
				delete[] pcAnnuncConfig8->GetLPMap(iNdx)->pchStr;
				pcAnnuncConfig8->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcAnnuncConfig8;
		pcAnnuncConfig8 = NULL;
	}
	if (!(NULL == pcAnnuncConfigGen))
	{
		delete pcAnnuncConfigGen;
		pcAnnuncConfigGen = NULL;
	}
	if (!(NULL == pcConnectorConfig))
	{
		delete pcConnectorConfig;
		pcConnectorConfig = NULL;
	}
	if (!(NULL == pcDHWStorageConfig))
	{
		delete pcDHWStorageConfig;
		pcDHWStorageConfig = NULL;
	}
	if (!(NULL == pcFanConfig))
	{
		delete pcFanConfig;
		pcFanConfig = NULL;
	}
	if (!(NULL == pcFlapValveConfig))
	{
		delete pcFlapValveConfig;
		pcFlapValveConfig = NULL;
	}
	if (!(NULL == pcFrostProtConfig))
	{
		delete pcFrostProtConfig;
		pcFrostProtConfig = NULL;
	}
	if (!(NULL == pcILKAnnuncConfig))
	{
		for (iNdx = 0; iNdx < pcILKAnnuncConfig->GetSize(); iNdx++)
		{
			if (!(NULL == pcILKAnnuncConfig->GetLPMap(iNdx)))
			{
				delete[] pcILKAnnuncConfig->GetLPMap(iNdx)->pchStr;
				pcILKAnnuncConfig->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcILKAnnuncConfig;
		pcILKAnnuncConfig = NULL;
	}
	if (!(NULL == pcLCIAnnuncConfig))
	{
		for (iNdx = 0; iNdx < pcLCIAnnuncConfig->GetSize(); iNdx++)
		{
			if (!(NULL == pcLCIAnnuncConfig->GetLPMap(iNdx)))
			{
				delete[] pcLCIAnnuncConfig->GetLPMap(iNdx)->pchStr;
				pcLCIAnnuncConfig->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcLCIAnnuncConfig;
		pcLCIAnnuncConfig = NULL;
	}
	if (!(NULL == pcLimitsConfig))
	{
		delete pcLimitsConfig;
		pcLimitsConfig = NULL;
	}
	if (!(NULL == pcPIIAnnuncConfig))
	{
		for (iNdx = 0; iNdx < pcPIIAnnuncConfig->GetSize(); iNdx++)
		{
			if (!(NULL == pcPIIAnnuncConfig->GetLPMap(iNdx)))
			{
				delete[] pcPIIAnnuncConfig->GetLPMap(iNdx)->pchStr;
				pcPIIAnnuncConfig->GetLPMap(iNdx)->pchStr = NULL;
			}
		}
		delete pcPIIAnnuncConfig;
		pcPIIAnnuncConfig = NULL;
	}
	if (!(NULL == pcPumpConfig))
	{
		delete pcPumpConfig;
		pcPumpConfig = NULL;
	}
	if (!(NULL == pcSystemIDResetRestart))
	{
		delete pcSystemIDResetRestart;
		pcSystemIDResetRestart = NULL;
	}
	if (!(NULL == pcX2CHConfig))
	{
		delete pcX2CHConfig;
		pcX2CHConfig = NULL;
	}
	if (!(NULL == pcX2ModConfig))
	{
		delete pcX2ModConfig;
		pcX2ModConfig = NULL;
	}
	if (!(NULL == pcXCHConfig))
	{
		delete pcXCHConfig;
		pcXCHConfig = NULL;
	}
	if (!(NULL == pcXLimitsConfig))
	{
		delete pcXLimitsConfig;
		pcXLimitsConfig = NULL;
	}
	if (!(NULL == pcXLLConfig))
	{
		delete pcXLLConfig;
		pcXLLConfig = NULL;
	}
	if (!(NULL == pcXModConfig))
	{
		delete pcXModConfig;
		pcXModConfig = NULL;
	}
	if (!(NULL == pcAnnuncMaps))
	{
		delete pcAnnuncMaps;
		pcAnnuncMaps = NULL;
	}
	if (!(NULL == pcStatusMaps))
	{
		delete pcStatusMaps;
		pcStatusMaps = NULL;
	}
	if (!(NULL == pcBurnerAlarmReasonCodes))
	{
		delete pcBurnerAlarmReasonCodes;
		pcBurnerAlarmReasonCodes = NULL;
	}
	if (!(NULL == pcBurnerAnnunFirstOutCodes))
	{
		delete pcBurnerAnnunFirstOutCodes;
		pcBurnerAnnunFirstOutCodes = NULL;
	}
	if (!(NULL == pcBurnerAnnunHoldCodes))
	{
		delete pcBurnerAnnunHoldCodes;
		pcBurnerAnnunHoldCodes = NULL;
	}
	if (!(NULL == pcBurnerControlFlags))
	{
		delete pcBurnerControlFlags;
		pcBurnerControlFlags = NULL;
	}
	if (!(NULL == pcBurnerRemoteStatCodes))
	{
		delete pcBurnerRemoteStatCodes;
		pcBurnerRemoteStatCodes = NULL;
	}
	if (!(NULL == pcDHWEnableList))
	{
		delete pcDHWEnableList;
		pcDHWEnableList = NULL;
	}
	if (!(NULL == pcDigitalIOCodes))
	{
		delete pcDigitalIOCodes;
		pcDigitalIOCodes = NULL;
	}
	if (!(NULL == pcRegisterAccess))
	{
		delete pcRegisterAccess;
		pcRegisterAccess = NULL;
	}
	if (!(NULL == pcSolaLockoutDesc))
	{
		delete pcSolaLockoutDesc;
		pcSolaLockoutDesc = NULL;
	}
	if (!(NULL == pc_Digital_IO_Page))
	{
		delete pc_Digital_IO_Page;
		pc_Digital_IO_Page = NULL;
	}
	if (!(NULL == pcCHConfigPage))
	{
		delete pcCHConfigPage;
		pcCHConfigPage = NULL;
	}
	if (!(NULL == pcDHWConfigPage))
	{
		delete pcDHWConfigPage;
		pcDHWConfigPage = NULL;
	}
	if (!(NULL == pcLimitsConfigPage))
	{
		delete pcLimitsConfigPage;
		pcLimitsConfigPage = NULL;
	}
	if (!(NULL == pcLLConfigPage))
	{
		delete pcLLConfigPage;
		pcLLConfigPage = NULL;
	}
	if (!(NULL == pcModConfigPage))
	{
		delete pcModConfigPage;
		pcModConfigPage = NULL;
	}
	if (!(NULL == pcODResetConfigPage))
	{
		delete pcODResetConfigPage;
		pcODResetConfigPage = NULL;
	}
	if (!(NULL == pcPumpConfigPage))
	{
		delete pcPumpConfigPage;
		pcPumpConfigPage = NULL;
	}
	if (!(NULL == pcSaveRestorePage))
	{
		delete pcSaveRestorePage;
		pcSaveRestorePage = NULL;
	}
	if (!(NULL == pcSystemIDPage))
	{
		delete pcSystemIDPage;
		pcSystemIDPage = NULL;
	}


	return b_r;
}