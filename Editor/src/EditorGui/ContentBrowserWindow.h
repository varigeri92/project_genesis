#pragma once
#include <filesystem>

#include "Engine.h"

namespace gns::editor
{
	class ContentBrowserWindow final : public gns::gui::GuiWindow
	{
	public:
		ContentBrowserWindow();
		~ContentBrowserWindow();
	protected:
		//void OnBeforeWindowDraw() override;
		void OnGUI() override;
		//void OnAfterWindowDraw() override;
	private:
		std::string m_assetsPath;
		std::string m_currentDirPath;
		std::string m_parentDirPath;

		void DrawDirectory(const std::filesystem::directory_entry& entry);
		void DrawFile(const std::filesystem::directory_entry& entry);
		void DrawDirectoryInHierarchy(const std::string& path);
	};
}
