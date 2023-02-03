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
try
{
#define CONFIG(name, desc) name = true;
#include "xconfig.h"
#undef CONFIG

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
	auto config = YAML::LoadFile(getConfigPath());

#define CONFIG(name, desc) name = config[#name].as<bool>();
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, name, desc) name = config[#name].as<std::vector<HotKey>>();
#include "xhotkeys.h"
#undef HOTKEY
}
catch (...)
{
}

void Config::save()
{
	YAML::Node config;

#define CONFIG(name, desc) config[#name] = name;
#include "xconfig.h"
#undef CONFIG

#define HOTKEY(row, name, desc) config[#name] = name;
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

Config& Config::get()
{
	static Config cfg;
	return cfg;
}

