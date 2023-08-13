#pragma once

#include <windows.h>

namespace PJ64
{
struct Timer
{
public:
	static DWORD now()
	{
		return timeGetTime();
	}

	void reset();
	BOOL process(DWORD* FrameRate);
	void adjust(DWORD amt);

private:
	DWORD frames_, lastTime_;
	DOUBLE ratio_;
};
}

struct AccurateTimer
{
public:
	static LONGLONG now()
	{
		LARGE_INTEGER t;
		QueryPerformanceCounter(&t);
		return t.QuadPart;
	}

	AccurateTimer();

	void setFrameRate(double hz);
	void start();
	void reset();
	BOOL process(DWORD* FrameRate);
	void adjust(LONGLONG amt);

private:
	LONGLONG freq_, lastTime_;
	LONGLONG metricFrame_, metricReset_;
	DWORD frames_;
	HANDLE timer_ = 0;
};
