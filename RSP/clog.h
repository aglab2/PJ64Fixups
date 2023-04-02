#pragma once

#include <string>
#include "path.h"

struct CLog
{
public:
	CLog() = default;

	void Open(const CPath&) {}
	void SetMaxFileSize(int sz) {}
	std::string FileName() { return name_; }

	void Log(const char*)
	{ }
	template<typename... Args>
	void LogF(const char* fmt, Args... args)
	{ }

private:
	std::string name_;
};
