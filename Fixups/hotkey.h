#pragma once

#include <windows.h>

#include <string>

#include "pj64_resources.h"

#define HK_MODIFIED_CTRL FCONTROL
#define HK_MODIFIED_ALT FALT
#define HK_MODIFIED_SHIFT FSHIFT

#define HKV_WINDOW_MODE 1
#define HKV_ROM_BROWSER 2
#define HKV_CPU_RUNNING 4

std::string vkToString(WPARAM vk);
WPARAM toVk(std::string_view);

struct HotKey
{
public:
	HotKey() : HotKey(0, 0)
	{ }
	HotKey(int mods, WPARAM code)
		: mods_(mods), code_(code)
	{ }

	static bool virtualCodeAllowed(WPARAM);

	std::string encode() const;
	static HotKey decode(std::string_view);

	WPARAM code() const { return code_; }
	bool ctrl() const { return !!(mods_ & HK_MODIFIED_CTRL); }
	bool alt() const { return !!(mods_ & HK_MODIFIED_ALT); }
	bool shift() const { return !!(mods_ & HK_MODIFIED_SHIFT); }

	ACCEL toAccel(WORD cmd) const;

private:
	void decodeToken(std::string_view sv);

	int mods_;
	WPARAM code_;
};

#include <yaml-cpp/yaml.h>

namespace YAML
{
	template<>
	struct convert<HotKey>
	{
		static Node encode(const HotKey&);
		static bool decode(const Node& node, HotKey&);
	};
}
