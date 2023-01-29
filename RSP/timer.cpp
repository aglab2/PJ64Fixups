#define _CRT_SECURE_NO_WARNINGS

#include "timer.h"

// #define DEBUG_ENABLE_TIMER_TRACING

#ifdef DEBUG_ENABLE_TIMER_TRACING
#include <vector>
#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>
#endif

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
		lastTime_ = timeGetTime();
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
			lastTime_ = timeGetTime();
		}

		RecordedFrame record{};

		double CalculatedTime;
		DWORD CurrentTime;

		record.lastFrames = frames_++;
		record.lastTime = lastTime_;
		record.currentTime = (CurrentTime = timeGetTime());

		/* Calculate time that should of elapsed for this frame */
		record.calculatedTime = (CalculatedTime = (double)lastTime_ + (ratio_ * (double)frames_));
		if ((double)CurrentTime < CalculatedTime) {
			long time = (int)(CalculatedTime - (double)CurrentTime);
			if (time > 0) {
				Sleep(time);
			}

			/* Refresh current time */
			CurrentTime = timeGetTime();
		}

		bool reset = CurrentTime - lastTime_ >= 1000;
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
}
