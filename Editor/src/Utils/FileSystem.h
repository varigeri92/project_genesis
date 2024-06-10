#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include "Log.h"

#define PROJECT_DIR "ProjectDir"

namespace gns::editor::fs
{
	class FileSystem
	{
		friend class AssetImporter;
		friend class AssetDatabase;
	public:
		static std::string GetAbsolutePath(const std::string relativePath);
		static std::string GetRelativePath(const std::string fullPath);
		static bool WriteFile(const std::string& source, const std::string& path);
		static std::string GetFileExtension(const std::string& path);
		static std::string AppendExtension(const std::string& filePath, const std::string& extension);
		static std::string ReplaceOrRemoveExtension(const std::string& filePath, const std::string& newExtension = "");
		static std::string GetFileName(const std::string& filePath);
		static bool HasExtension(const std::string& filePath, const std::string& extension);


	private:
		static std::vector<std::string> GetDirectoryContent(const std::string& path);
		static bool GetFile(const std::string& path, void* bytes, bool isRelative = true);
		static bool IsGnsFile(const std::string& path);
		static bool ScanDirectoryFor(
			const std::string& extension, const std::string& RootPath = PROJECT_DIR);
	};
}
