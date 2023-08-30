#pragma once

#include "hotkey.h"

#include <vector>

class Config
{
public:
	Config() { load(); }

	void save();

	static Config& get();

#define CONFIG(name, desc) bool name;
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, view, name, cmd, desc) std::vector<HotKey> name;
#include "xhotkeys.h"
#undef HOTKEY

	std::set<WPARAM> ignoredSavestateSlotKeys;

	HACCEL accelRomBrowser = NULL;
	HACCEL accelCPURunning = NULL;
	HACCEL accelWinMode = NULL;

	int inputDelay = 0;

private:
	void load();
	void compileAccel();
};
