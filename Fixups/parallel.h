#pragma once

#include <stdint.h>
#include "Rsp #1.1.h"
#include <Windows.h>

namespace RSP
{
	namespace Zilmar
	{
		enum SettingLocation
		{
			SettingType_ConstString = 0,
			SettingType_ConstValue = 1,
			SettingType_CfgFile = 2,
			SettingType_Registry = 3,
			SettingType_RelativePath = 4,
			TemporarySetting = 5,
			SettingType_RomDatabase = 6,
			SettingType_CheatSetting = 7,
			SettingType_GameSetting = 8,
			SettingType_BoolVariable = 9,
			SettingType_NumberVariable = 10,
			SettingType_StringVariable = 11,
			SettingType_SelectedDirectory = 12,
			SettingType_RdbSetting = 13,
		};

		enum SettingDataType
		{
			Data_DWORD = 0, Data_String = 1, Data_CPUTYPE = 2, Data_SelfMod = 3, Data_OnOff = 4, Data_YesNo = 5, Data_SaveChip = 6
		};

		typedef struct
		{
			uint32_t dwSize;
			int DefaultStartRange;
			int SettingStartRange;
			int MaximumSettings;
			int NoDefault;
			int DefaultLocation;
			void* handle;

			unsigned int(CALL* GetSetting)      (void* handle, int ID);
			const char* (CALL* GetSettingSz)    (void* handle, int ID, char* Buffer, int BufferLen);
			void(CALL* SetSetting)      (void* handle, int ID, unsigned int Value);
			void(CALL* SetSettingSz)    (void* handle, int ID, const char* Value);
			void(CALL* RegisterSetting) (void* handle, int ID, int DefaultID, SettingDataType Type,
				SettingLocation Location, const char* Category, const char* DefaultStr, uint32_t Value);
			void(CALL* UseUnregisteredSetting) (int ID);
		} PLUGIN_SETTINGS;

		typedef struct
		{
			unsigned int(CALL* FindSystemSettingId) (void* handle, const char* Name);
		} PLUGIN_SETTINGS2;

		static PLUGIN_SETTINGS  g_PluginSettings;
		static PLUGIN_SETTINGS2 g_PluginSettings2;
		static inline unsigned int GetSystemSetting(short SettingID)
		{
			return g_PluginSettings.GetSetting(g_PluginSettings.handle, SettingID);
		}

		static inline short FindSystemSettingId(const char* Name)
		{
			if (g_PluginSettings2.FindSystemSettingId && g_PluginSettings.handle)
			{
				return (short)g_PluginSettings2.FindSystemSettingId(g_PluginSettings.handle, Name);
			}
			return 0;
		}
	}
}

namespace Parallel
{
	unsigned int CALL DoRspCycles(unsigned int cycles);
	void CALL GetDllInfo(PLUGIN_INFO* PluginInfo);
	void CALL RomClosed(void);
	void CALL SetSettingInfo(RSP::Zilmar::PLUGIN_SETTINGS* info);
	void CALL SetSettingInfo2(RSP::Zilmar::PLUGIN_SETTINGS2* info);
	void CALL PluginLoaded(void);
	void CALL InitiateRSP(RSP_INFO Rsp_Info, unsigned int* CycleCount);
	void CALL CloseDLL(void);
	void CALL DllConfig(int hWnd);
}
