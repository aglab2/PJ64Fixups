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
	auto config = YAML::LoadFile(getConfigPath());

#define CONFIG(name, desc) name = config[#name].as<bool>();
#include "xconfig.h"
#undef CONFIG
}
catch (...)
{
#define CONFIG(name, desc) name = true;
#include "xconfig.h"
#undef CONFIG
}

void Config::save()
{
	YAML::Node config;

#define CONFIG(name, desc) config[#name] = name;
#include "xconfig.h"
#undef CONFIG

	YAML::Emitter emitter;
	emitter << config;

	int fd = _open(getConfigPath(), _O_WRONLY | _O_CREAT | _O_TRUNC, 0666);
	if (-1 != fd)
	{
		_write(fd, emitter.c_str(), emitter.size());
		_close(fd);
	}
}

Config sConfig;
