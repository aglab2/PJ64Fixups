#include "timer.h"

namespace PJ64
{
	void Timer::reset()
	{
		frames_ = 0;
		lastTime_ = timeGetTime();
	}
}
