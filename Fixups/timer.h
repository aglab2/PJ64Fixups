#pragma once

#include <windows.h>

namespace PJ64
{
struct Timer
{
public:
	void reset();
	BOOL process(DWORD* FrameRate);
	void adjust(DWORD amt);

private:
	DWORD frames_, lastTime_;
	DOUBLE ratio_;
};
}
