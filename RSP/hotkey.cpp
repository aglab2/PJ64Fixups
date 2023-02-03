#include "hotkey.h"

#include <assert.h>

#include <map>
#include <set>
#include <unordered_map>

static std::set<WPARAM> AllowedSpecialKeys =
{
	VK_BACK, VK_TAB, VK_RETURN, VK_CAPITAL, VK_ESCAPE, VK_SPACE, VK_PRIOR, VK_NEXT, VK_END, VK_HOME, VK_PAUSE,
	// VK_CLEAR, VK_SELECT, VK_EXECUTE, VK_PRINT, VK_HELP, VK_SLEEP, 
	VK_LEFT, VK_UP, VK_RIGHT, VK_DOWN, VK_SNAPSHOT, VK_INSERT, VK_DELETE, 
	VK_NUMPAD0, VK_NUMPAD1, VK_NUMPAD2, VK_NUMPAD3, VK_NUMPAD4, VK_NUMPAD5,
	VK_NUMPAD6, VK_NUMPAD7, VK_NUMPAD8, VK_NUMPAD9, VK_MULTIPLY, VK_ADD, VK_SEPARATOR, VK_SUBTRACT, VK_DECIMAL, VK_DIVIDE,
	VK_F1, VK_F2, VK_F3, VK_F4, VK_F5, VK_F6, VK_F7, VK_F8, VK_F9, VK_F10, VK_F11, VK_F12,
	VK_F13, VK_F14, VK_F15, VK_F16, VK_F17, VK_F18, VK_F19, VK_F20, VK_F21, VK_F22, VK_F23, VK_F24,
	VK_SCROLL,
	// VK_BROWSER_BACK, VK_BROWSER_FORWARD, VK_BROWSER_REFRESH, VK_BROWSER_HOME, VK_VOLUME_MUTE, VK_VOLUME_DOWN, VK_VOLUME_UP, VK_OEM_8
	// VK_MEDIA_NEXT_TRACK, VK_MEDIA_PREV_TRACK, VK_MEDIA_STOP, VK_MEDIA_PLAY_PAUSE, VK_LAUNCH_MAIL, VK_LAUNCH_MEDIA_SELECT, VK_LAUNCH_APP1, VK_LAUNCH_APP2
	VK_OEM_1, VK_OEM_PLUS, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD, VK_OEM_2, VK_OEM_3, VK_OEM_4, VK_OEM_5, VK_OEM_6, VK_OEM_7, VK_OEM_102
};

static std::map<WPARAM, std::string> SpecialKeyToName =
{
#define XVIRTKEY(v, n) { v, n },
#include "xvirtkey.h"
#undef XVIRTKEY
};

static std::unordered_map<std::string_view, WPARAM> NameToSpecialKey =
{
#define XVIRTKEY(v, n) { n, v },
#include "xvirtkey.h"
#undef XVIRTKEY
};

bool HotKey::virtualCodeAllowed(WPARAM vk)
{
	if ('0' <= vk && vk <= '9')
		return true;

	if ('A' <= vk && vk <= 'Z')
		return true;

	return AllowedSpecialKeys.find(vk) != AllowedSpecialKeys.end();
}

static std::string toString(WPARAM vk)
{
	if ('0' <= vk && vk <= '9')
		return std::string(1, (char)vk);

	if ('A' <= vk && vk <= 'Z')
		return std::string(1, (char)vk);

	auto it = SpecialKeyToName.find(vk);
	assert(it != SpecialKeyToName.end());
	return it->second;
}

std::string HotKey::encode() const
{
	std::string name = toString(code_);
	if (shift())
	{
		name = "Shift+" + name;
	}
	if (alt())
	{
		name = "Alt+" + name;
	}
	if (ctrl())
	{
		name = "Ctrl+" + name;
	}

	return name;
}

void HotKey::decodeToken(std::string_view tok)
{
	if (tok == "Ctrl")
	{
		mods_ |= HK_MODIFIED_CTRL;
	}
	else if (tok == "Alt")
	{
		mods_ |= HK_MODIFIED_ALT;
	}
	else if (tok == "Shift")
	{
		mods_ |= HK_MODIFIED_SHIFT;
	}
	else
	{
		if (tok.length() == 1)
		{
			char vk = tok[0];
			if (('0' <= vk && vk <= '9') || ('A' <= vk && vk <= 'Z'))
			{
				code_ = vk;
				return;
			}
		}

		auto it = NameToSpecialKey.find(tok);
		if (it != NameToSpecialKey.end())
		{
			code_ = it->second;
		}
		else
		{
			abort();
		}
	}
}

HotKey HotKey::decode(std::string_view sv)
{
	HotKey hotkey;

	while (true)
	{
		auto pos = sv.find('+');
		if (pos == -1)
		{
			hotkey.decodeToken(sv);
			break;
		}

		auto part = sv.substr(0, pos);
		hotkey.decodeToken(part);
		sv = sv.substr(pos + 1);
	}

	assert(0 != hotkey.code_);
	return hotkey;
}

namespace YAML
{
	Node convert<HotKey>::encode(const HotKey& k)
	{
		return YAML::Node{ k.encode() };
	}

	bool convert<HotKey>::decode(const Node& node, HotKey& k)
	{
		k = HotKey::decode(node.as<std::string>());
		return true;
	}
}
