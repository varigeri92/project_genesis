#pragma once
#include <filesystem>
#include <map>
#include <memory>
#include <genesis.h>

namespace gns::rendering
{
	class Texture;
}

namespace gns::gui
{

	class ContentBrowser : public GuiWindow
	{

		struct File
		{
			std::string path;
			std::string name;
			std::string ext;
		};
		struct Directory
		{
			std::string path;
			std::string name;
			std::vector<Directory> directories;
			std::vector<File> files;
			Directory() = default;
			Directory(std::string path, std::string name) :
				path(path), name(name), directories({}), files({}) {}
		};

	private:
		std::map<std::string, std::shared_ptr<gns::rendering::Texture>> icon_map;
		std::string m_assetsPath;
		Directory m_workingDirectory;
		Directory SearchDirectory(Directory& path);
		Directory DrawDirectory(Directory& path);
		Directory DrawLeftPane(Directory& path);

	public:
		ContentBrowser();

	protected:
		void OnGUI() override;
	};
}

