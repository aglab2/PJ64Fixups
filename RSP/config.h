#pragma once

class Config
{
public:
	Config() { load(); }

	void load();
	void save();

#define CONFIG(name, desc) bool name;
#include "xconfig.h"
#undef CONFIG
};

extern Config sConfig;
