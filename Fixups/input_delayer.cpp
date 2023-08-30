#include "input_delayer.h"

#include "config.h"
#include "pj64_globals.h"

#define MINIMAL_DURATION_MS 5
#define MINIMAL_ERROR_MS 3

bool InputDelayer::SampleInputLessThan::operator() (const SampledInput& left, const SampledInput& right)
{
	return left.time < right.time;
}
bool InputDelayer::SampleInputLessThan::operator() (const SampledInput& left, InputDelayer::TimePoint right)
{
	return left.time < right;
}
bool InputDelayer::SampleInputLessThan::operator() (InputDelayer::TimePoint left, const SampledInput& right)
{
	return left < right.time;
}

void InputDelayer::start()
{
	std::lock_guard<std::mutex> lck1(startStopMutex_);
	std::lock_guard<std::mutex> lck2(mutex_);
	if (running_)
		return;

	sampledInputs_ = {};
	delayedKeysLastRequestTime_ = Clock::now();
	delayedKeysRequestDurations_ = {};
	running_ = true;
	expectedWakeUpTime_ = expectedPollingTime_ = delayedKeysLastRequestTime_ + estimateFrameTime();
	worker_ = std::thread{ &InputDelayer::work, this };
}

void InputDelayer::stop()
{
	std::lock_guard<std::mutex> lck1(startStopMutex_);
	if (!worker_.joinable())
		return;

	{
		std::lock_guard<std::mutex> lck2(mutex_);
		running_ = false;
	}
	cv_.notify_one();
	worker_.join();
}

void InputDelayer::work()
{
	std::unique_lock<std::mutex> lck(mutex_);
	while (running_)
	{
		auto res = cv_.wait_until(lck, expectedWakeUpTime_);
		if (res == std::cv_status::no_timeout)
		{
			// woken up
			if (expectedPollingTime_ < expectedWakeUpTime_)
			{
				auto diff = expectedWakeUpTime_ - expectedPollingTime_;
				if (diff > std::chrono::milliseconds(MINIMAL_ERROR_MS))
				{
					expectedWakeUpTime_ = expectedPollingTime_;
					continue;
				}
			}
		}

		auto now = Clock::now();
		DWORD keys;
		// can technically unlock mutex for GetKeys but should not matter much
		PJ64::Globals::ControllerGetKeysFn()(0, &keys);
		sampledInputs_.push({ now, keys });

		if (expectedWakeUpTime_ < now + std::chrono::milliseconds(MINIMAL_ERROR_MS))
		{
			expectedWakeUpTime_ = expectedPollingTime_ = now + estimateFrameTime();
		}
		else
		{
			expectedWakeUpTime_ = expectedPollingTime_;
		}
	}
}

InputDelayer::Duration InputDelayer::absDiff(TimePoint t0, TimePoint t1)
{
	if (t0 > t1)
	{
		return t0 - t1;
	}
	else
	{
		return t1 - t0;
	}
}

DWORD InputDelayer::findBestKeys(TimePoint now)
{
	if (sampledInputs_.empty())
		return 0;

	auto it1 = std::lower_bound(sampledInputs_.begin(), sampledInputs_.end(), now, SampleInputLessThan{});
	if (it1 == sampledInputs_.begin())
	{
#ifdef DEBUG_RECORD_ERROR_DURATIONS
		recordedErrorDurations_.push(absDiff(now, it1->time));
#endif
		return it1->key;
	}

	auto it2 = it1 - 1;
	if (it1 == sampledInputs_.end())
	{
#ifdef DEBUG_RECORD_ERROR_DURATIONS
		recordedErrorDurations_.push(absDiff(now, it2->time));
#endif
		return it2->key;
	}

	auto d1 = absDiff(it1->time, now);
	auto d2 = absDiff(it2->time, now);
	if (d2 > d1)
	{
#ifdef DEBUG_RECORD_ERROR_DURATIONS
		recordedErrorDurations_.push(d1);
#endif
		return it1->key;
	}
	else
	{
#ifdef DEBUG_RECORD_ERROR_DURATIONS
		recordedErrorDurations_.push(d2);
#endif
		return it2->key;
	}
}

InputDelayer::Duration InputDelayer::estimateFrameTime()
{
	if (delayedKeysRequestDurations_.empty())
	{
		return std::chrono::duration_cast<Duration>(std::chrono::milliseconds(1000 / 30));
	}

	// copy
	std::vector<Duration> buf;
	if (delayedKeysRequestDurations_.full())
	{
		buf = delayedKeysRequestDurations_.buffer();
	}
	else
	{
		const Duration* data = delayedKeysRequestDurations_.buffer().data();
		buf = { data, data + delayedKeysRequestDurations_.head() };
	}

	auto m = buf.begin() + buf.size() / 2;
	std::nth_element(buf.begin(), m, buf.end());
	return buf[buf.size() / 2];
}

DWORD InputDelayer::getDelayedKeys()
{
	std::lock_guard<std::mutex> lck(mutex_);
	auto now = Clock::now();
	int inputDelay = Config::get().inputDelay;
	if (inputDelay < MINIMAL_ERROR_MS)
		inputDelay = MINIMAL_ERROR_MS;

	auto keys = findBestKeys(now - std::chrono::milliseconds(inputDelay));
	auto duration = now - delayedKeysLastRequestTime_;
	if (duration < std::chrono::milliseconds(MINIMAL_DURATION_MS))
		return keys;

	delayedKeysRequestDurations_.push(duration);
	delayedKeysLastRequestTime_ = now;

	Duration frameTime = estimateFrameTime();
	Duration delay = std::chrono::duration_cast<Duration>(std::chrono::milliseconds(inputDelay));
	while (delay > frameTime)
	{
		delay -= frameTime;
	}

	expectedPollingTime_ = delay + now;
	if (expectedPollingTime_ < expectedWakeUpTime_)
	{
		auto diff = expectedWakeUpTime_ - expectedPollingTime_;
		if (diff > std::chrono::milliseconds(MINIMAL_ERROR_MS))
		{
			cv_.notify_one();
		}
	}
	return keys;
}
