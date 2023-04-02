#include "Settings.h"

#include <cassert>
#include <vector>
#include <string>
#include <unordered_set>

struct Config
{
	unsigned int value;
};

static std::vector<Config> gSettings;

void SetSetting(int cmd, int val)
{
	assert(cmd < gSettings.size());
	gSettings[cmd].value = val;
}

int GetSystemSetting(int cmd)
{
	return 1;
}

int GetSetting(int cmd)
{
	assert(cmd < gSettings.size());
	return gSettings[cmd].value;
}

void SetModuleName(const char* name)
{
}

int FindSystemSettingId(const char* name)
{
	return 0;
}

void RegisterSetting(int cmd, enum Data, const char* name, const char* category, unsigned int dflt, const char* dfltStr)
{
	assert(dfltStr == NULL);
	assert(cmd == gSettings.size());
	gSettings.push_back(Config{ dflt });
}
