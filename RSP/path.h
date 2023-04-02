#pragma once

#include <filesystem>
#include <string>

struct CPath
{
public:
	static const int MODULE_DIRECTORY = 0;

	CPath(const char* path) : path_(path)
	{}
	CPath(std::string path) : path_(path)
	{}
	CPath(int, std::string name) : path_(name)
	{}
	~CPath() = default;

	void AppendDirectory(std::string)
	{}

private:
	std::filesystem::path path_;
};
