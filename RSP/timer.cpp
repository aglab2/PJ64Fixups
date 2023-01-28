#include "timer.h"

namespace PJ64
{
	void Timer::reset()
	{
		frames_ = -1;
		lastTime_ = timeGetTime();
	}
}
