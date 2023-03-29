#pragma once
#include <string>
#include <Windows.h>
#include <iostream>

class WorkingDirectory
{
public:
	WorkingDirectory() {
		wchar_t path[260];
		if (!GetCurrentDirectory(260, path)) {
			std::cout << "Error Getting Curr Dir: " << GetLastError();
		}
		else {
			for (int i = 0; path[i] != '\0'; i++)
				this->path += path[i];
			this->path += '\\';
		}
	}

	inline const std::string& Get()
	{
		return path;
	}
private:
	std::string path;
};

namespace Wic {
	WorkingDirectory workingDir;
}