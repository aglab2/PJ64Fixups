#pragma once

#include <windows.h>

#include <string>

#define HK_MODIFIED_CTRL 1
#define HK_MODIFIED_ALT 2
#define HK_MODIFIED_SHIFT 4

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
