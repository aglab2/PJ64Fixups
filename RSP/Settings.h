#pragma once

#include "Types.h"

void SetSetting(int cmd, int val);
int GetSystemSetting(int cmd);
int GetSetting(int cmd);
void SetModuleName(const char*);
int FindSystemSettingId(const char*);

enum Data
{
	Data_DWORD_General,
	Data_DWORD_Game,
};

void RegisterSetting(int cmd, enum Data, const char* name, const char* category, unsigned int dflt, const char* dfltStr);
