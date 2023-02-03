#pragma once

#include "hotkey.h"

#include <vector>

class Config
{
public:
	Config() { load(); }

	void load();
	void save();

	static Config& get();

#define CONFIG(name, desc) bool name;
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, name, desc) std::vector<HotKey> name;
#include "xhotkeys.h"
#undef HOTKEY
};
