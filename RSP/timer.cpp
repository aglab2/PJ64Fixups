#define _CRT_SECURE_NO_WARNINGS

#include "timer.h"

#include <vector>

#include <Shlobj.h>
#include <Shlobj_core.h>
#include <shlwapi.h>

namespace PJ64
{
#define FRAMES_TO_RECORD 200
	struct RecordedFrame
	{
		double calculatedTime;
		DWORD lastTime;
		DWORD currentTime;
		DWORD lastFrames;
		bool reset;
	};

	std::vector<RecordedFrame> sRecordedFrames;
	int sFramesLeftToRecord = 0;

	void Timer::reset()
	{
		frames_ = -2;
		lastTime_ = timeGetTime();
		sFramesLeftToRecord = FRAMES_TO_RECORD;
	}

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
