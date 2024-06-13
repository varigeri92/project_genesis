#pragma once
#include <string>

class NativeFileBrowser
{
public:
	enum class FileBrowserMode
	{
		OpenFile, SaveFile,
	};
	GNS_API static std::string OpenBrowser(std::string filter, FileBrowserMode mode);
};
