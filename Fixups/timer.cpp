#define _CRT_SECURE_NO_WARNINGS

#include "timer.h"

// #define DEBUG_ENABLE_TIMER_TRACING

#ifdef DEBUG_ENABLE_TIMER_TRACING
#include <vector>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>
#endif

#define MS_RESET_TIME 70

namespace PJ64
{
	struct RecordedFrame
	{
		double calculatedTime;
		DWORD lastTime;
		DWORD currentTime;
		DWORD lastFrames;
		bool reset;
	};

#ifdef DEBUG_ENABLE_TIMER_TRACING
#define FRAMES_TO_RECORD 200

	std::vector<RecordedFrame> sRecordedFrames;
	int sFramesLeftToRecord = 0;

	static const char* getRecordsPath()
	{
		static char path[MAX_PATH]{ 0 };
		if ('\0' == *path)
		{
			SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, 0, path);
			PathAppend(path, "PJ64Fixups");
			CreateDirectory(path, nullptr);
			PathAppend(path, "records.txt");
		}
		return path;
	}

#endif

	void Timer::reset()
	{
		frames_ = -2;
#ifdef DEBUG_ENABLE_TIMER_TRACING
		sFramesLeftToRecord = FRAMES_TO_RECORD;
#endif
	}

	BOOL Timer::process(DWORD* FrameRate)
	{
		if (-2 == frames_)
		{
			frames_ = -1;
			return FALSE;
		}
		if (-1 == frames_)
		{
			frames_ = 0;
			lastTime_ = now();
		}

		RecordedFrame record{};

		double CalculatedTime;
		DWORD CurrentTime;

		record.lastFrames = frames_++;
		record.lastTime = lastTime_;
		record.currentTime = (CurrentTime = now());

		/* Calculate time that should of elapsed for this frame */
		record.calculatedTime = (CalculatedTime = (double)lastTime_ + (ratio_ * (double)frames_));
		bool reset = CurrentTime - lastTime_ >= 1000;
		if ((double)CurrentTime < CalculatedTime) {
			long time = (int)(CalculatedTime - (double)CurrentTime);
			if (time > 0) {
				Sleep(time);
			}

			/* Refresh current time */
			CurrentTime = now();
		}
		else
		{
			// this is a new code - if we are falling very behind, try to reset the timer
			long time = (int)((double)CurrentTime - CalculatedTime);
			reset = time > MS_RESET_TIME;
		}

		record.reset = reset;

#ifdef DEBUG_ENABLE_TIMER_TRACING
		if (sFramesLeftToRecord)
		{
			sRecordedFrames.push_back(record);
			sFramesLeftToRecord--;
		}
		else if (!sRecordedFrames.empty())
		{
			FILE* fd = fopen(getRecordsPath(), "w");
			if (fd)
			{
				for (const auto& record : sRecordedFrames)
				{
					fprintf(fd, "calc=%lf cur=%ld diff=%lf time=%ld frames=%ld%s\n", record.calculatedTime, record.currentTime, record.calculatedTime - record.currentTime, record.lastTime, record.lastFrames, record.reset ? " reset" : "");
				}
				fclose(fd);
			}
			sRecordedFrames.clear();
		}
#endif

		if (reset) {
			/* Output FPS */
			if (FrameRate != NULL) { *FrameRate = frames_; }
			frames_ = 0;
			lastTime_ = CurrentTime;

			return TRUE;
		}
		else
			return FALSE;
	}

	void Timer::adjust(DWORD amt)
	{
		lastTime_ += amt;
	}
}

AccurateTimer::AccurateTimer()
{
	LARGE_INTEGER t;
	QueryPerformanceFrequency(&t);
	freq_ = t.QuadPart;
	metricFrame_ = freq_ / 60;
	metricReset_ = freq_ * 1000 / MS_RESET_TIME;
}

void AccurateTimer::setFrameRate(double hz)
{
	metricFrame_ = freq_ / hz;
	// TODO: should depend on 'hz'
	metricReset_ = freq_ * 1000 / MS_RESET_TIME;
}

void AccurateTimer::start()
{
	frames_ = 0;
	lastTime_ = now();
	if (!timer_)
		timer_ = CreateWaitableTimerExW(nullptr, nullptr, CREATE_WAITABLE_TIMER_HIGH_RESOLUTION, TIMER_ALL_ACCESS);
	
	// Fallback to low resolution timer if unsupported by the OS
	if (!timer_)
		timer_ = CreateWaitableTimer(nullptr, FALSE, nullptr);
}

void AccurateTimer::reset()
{
	frames_ = -2;
}

BOOL AccurateTimer::process(DWORD* FrameRate)
{
	if (-2 == frames_)
	{
		frames_ = -1;
		return FALSE;
	}
	if (-1 == frames_)
	{
		frames_ = 0;
		lastTime_ = now();
	}

	LONGLONG CalculatedTime{};
	LONGLONG CurrentTime;

	frames_++;
	CurrentTime = now();

	/* Calculate time that should of elapsed for this frame */
	CalculatedTime = lastTime_ + frames_ * metricFrame_;
	bool reset = (CurrentTime - lastTime_) >= freq_;
	LARGE_INTEGER left{};
	left.QuadPart = CurrentTime - CalculatedTime;
	if (left.QuadPart < 0)
	{
		SetWaitableTimer(timer_, &left, 0, nullptr, nullptr, false);
		WaitForSingleObject(timer_, INFINITE);

		/* Refresh current time */
		CurrentTime = now();
	}
	else
	{
		// this is a new code - if we are falling very behind, try to reset the timer
		reset = left.QuadPart > MS_RESET_TIME;
	}

	if (reset) {
		/* Output FPS */
		if (FrameRate != NULL) { *FrameRate = frames_; }
		frames_ = 0;
		lastTime_ = CurrentTime;

		return TRUE;
	}
	else
		return FALSE;
}

void AccurateTimer::adjust(LONGLONG amt)
{
	lastTime_ += amt;
}
