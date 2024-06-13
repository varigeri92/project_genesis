#include "gnspch.h"
#include "NativeFileBrowser.h"

#include "Engine.h"
#include "../Window/Window.h"
#define WINDOWS_BUILD
#ifdef WINDOWS_BUILD
#include <windows.h>
#include <commdlg.h>
#endif


std::string NativeFileBrowser::OpenBrowser(std::string filter, FileBrowserMode mode)
{
	OPENFILENAMEA ofn;
	CHAR szFile[260] = { 0 };
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = gns::Window::getInstance()->hwndHandle;
	ofn.lpstrFile = szFile;
	ofn.nMaxFile = sizeof(szFile);
	ofn.lpstrFilter = filter.c_str();
	ofn.nFilterIndex = 1;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
	if(mode == FileBrowserMode::OpenFile)
	{
		if(GetOpenFileNameA(&ofn)==TRUE)
		{
			return ofn.lpstrFile;
		}
	}
	else
	{
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			std::string path = ofn.lpstrFile;
			if(gns::fileSystem::FileSystem::HasExtension(path, filter))
			{
				return path;
			}
			return path + filter;
		}
	}
	return {};
}
