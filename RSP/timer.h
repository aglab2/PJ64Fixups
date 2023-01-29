#pragma once

#include <windows.h>

namespace PJ64
{
struct Timer
{
public:
	void reset();
	BOOL process(DWORD* FrameRate);

private:
	DWORD frames_, lastTime_;
	DOUBLE ratio_;
};
}
