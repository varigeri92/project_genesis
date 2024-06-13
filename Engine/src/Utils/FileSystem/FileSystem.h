#pragma once
#include <string>
#include <filesystem>
#include <fstream>
#include <vector>
#include "Log.h"

#define PROJECT_DIR "ProjectDir"

namespace gns::fileSystem
{
	class FileSystem
	{
		friend class AssetImporter;
		friend class AssetDatabase;
	public:
		GNS_API static std::string GetAbsolutePath(const std::string relativePath);
		GNS_API static std::string GetRelativePath(const std::string fullPath);
		GNS_API static bool WriteFile(const std::string& source, const std::string& path);
		GNS_API static std::string ReadFile(const std::string& path);
		GNS_API static std::string GetFileExtension(const std::string& path);
		GNS_API static std::string AppendExtension(const std::string& filePath, const std::string& extension);
		GNS_API static std::string ReplaceOrRemoveExtension(const std::string& filePath, const std::string& newExtension = "");
		GNS_API static std::string GetFileName(const std::string& filePath);
		GNS_API static bool HasExtension(const std::string& filePath, const std::string& extension);
		GNS_API static std::vector<std::string> GetAllFileInDirectory(const std::string& path);
		GNS_API static bool Exists(const std::string& file);
	private:
		static std::vector<std::string> GetDirectoryContent(const std::string& path);
		static bool GetFile(const std::string& path, void* bytes, bool isRelative = true);
		static bool IsGnsFile(const std::string& path);
		static bool ScanDirectoryFor(
			const std::string& extension, const std::string& RootPath = PROJECT_DIR);
	};
}
