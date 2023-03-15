#define _CRT_SECURE_NO_WARNINGS

#include "config.h"

#include "yaml-cpp/yaml.h"

#include <io.h>
#include <fcntl.h>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

static const char* getConfigPath()
{
	static char path[MAX_PATH]{ 0 };
	if ('\0' == *path)
	{
		SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
		PathAppend(path, "PJ64Fixups");
		CreateDirectory(path, nullptr);
		PathAppend(path, "cfg.yaml");
	}
	return path;
}

void Config::load()
{
#define CONFIG(name, desc) name = true;
#include "xconfig.h"
#undef CONFIG
	overclockVI = false;
	cf0 = false;

	alwaysOnTop = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'A' } };
	cheats	     = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'C' } };
	openROM     = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'O' } };
	cpuLoad     = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'L' } };
	saveAs      = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'S' } };
	settings    = std::vector{ HotKey{ HK_MODIFIED_CTRL, 'T' } };
	startEmulation = std::vector{ HotKey{ 0, VK_F11 } };
	refreshROMList = std::vector{ HotKey{ 0, VK_F5 } };
	currentSave = std::vector{ HotKey{ 0, VK_OEM_3 } };
	currentSave0 = std::vector{ HotKey{ 0, '0' } };
	currentSave1 = std::vector{ HotKey{ 0, '1' } };
	currentSave2 = std::vector{ HotKey{ 0, '2' } };
	currentSave3 = std::vector{ HotKey{ 0, '3' } };
	currentSave4 = std::vector{ HotKey{ 0, '4' } };
	currentSave5 = std::vector{ HotKey{ 0, '5' } };
	currentSave6 = std::vector{ HotKey{ 0, '6' } };
	currentSave7 = std::vector{ HotKey{ 0, '7' } };
	currentSave8 = std::vector{ HotKey{ 0, '8' } };
	currentSave9 = std::vector{ HotKey{ 0, '9' } };
	fullScreen = std::vector{ HotKey{ 0, VK_ESCAPE }, HotKey{ HK_MODIFIED_ALT, VK_RETURN } };
	cpuReset = std::vector{ HotKey{ 0, VK_F1 } };
	endEmulation = std::vector{ HotKey{ 0, VK_F12 } };
	cpuPause = std::vector{ HotKey{ 0, VK_F2 }, HotKey{ 0, VK_PAUSE } };
	generateBitMap = std::vector{ HotKey{ 0, VK_F3 } };
	limitFPS = std::vector{ HotKey{ 0, VK_F4 } };
	cpuSave = std::vector{ HotKey{ 0, VK_F5 } };
	cpuRestore = std::vector{ HotKey{ 0, VK_F7 } };
	gsButton = std::vector{ HotKey{ 0, VK_F9 } };

	try
	{
		auto config = YAML::LoadFile(getConfigPath());

#define CONFIG(name, desc) name = config[#name].as<bool>();
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, view, name, cmd, desc) name = config[#name].as<std::vector<HotKey>>();
#include "xhotkeys.h"
#undef HOTKEY
	}
	catch (...)
	{
	}

	compileAccel();
}

void Config::save()
{
	try
	{
		YAML::Node config;

#define CONFIG(name, desc) config[#name] = name;
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, view, name, cmd, desc) config[#name] = name;
#include "xhotkeys.h"
#undef HOTKEY

		YAML::Emitter emitter;
		emitter << config;

		int fd = _open(getConfigPath(), _O_WRONLY | _O_CREAT | _O_TRUNC, 0666);
		if (-1 != fd)
		{
			_write(fd, emitter.c_str(), emitter.size());
			_close(fd);
		}
	}
	catch (...)
	{
	}

	compileAccel();
}

Config& Config::get()
{
	static Config cfg;
	return cfg;
}

void Config::compileAccel()
{
	// TODO: Use x-macro
	if (accelRomBrowser) DestroyAcceleratorTable(accelRomBrowser);
	if (accelCPURunning) DestroyAcceleratorTable(accelCPURunning);
	if (accelWinMode) DestroyAcceleratorTable(accelWinMode);

	accelRomBrowser = NULL;
	accelCPURunning = NULL;
	accelWinMode = NULL;

	std::vector<ACCEL> romBrowserAccels;
	romBrowserAccels.reserve(30);
	std::vector<ACCEL> cpuRunningAccels;
	cpuRunningAccels.reserve(30);
	std::vector<ACCEL> winModeAccels;
	winModeAccels.reserve(30);

	const auto compileHotkeys = [&](int flags, const std::vector<HotKey>& keys, int cmd)
	{
		if (flags & HKV_ROM_BROWSER) 
		{
			for (const auto& key : keys)
			{
				romBrowserAccels.push_back(key.toAccel(cmd));
			}
		}
		if (flags & HKV_CPU_RUNNING)
		{
			for (const auto& key : keys)
			{
				cpuRunningAccels.push_back(key.toAccel(cmd));
			}
		}
		if (flags & HKV_WINDOW_MODE)
		{
			for (const auto& key : keys)
			{
				winModeAccels.push_back(key.toAccel(cmd));
			}
		}
	};

#define HOTKEY(row, view, name, cmd, desc) compileHotkeys(view, name, cmd);
#include "xhotkeys.h"
#undef HOTKEY

	if (!romBrowserAccels.empty())
		accelRomBrowser = CreateAcceleratorTable(romBrowserAccels.data(), romBrowserAccels.size());
	
	if (!cpuRunningAccels.empty())
		accelCPURunning = CreateAcceleratorTable(cpuRunningAccels.data(), cpuRunningAccels.size());

	if (!winModeAccels.empty())
		accelWinMode = CreateAcceleratorTable(winModeAccels.data(), winModeAccels.size());
}

