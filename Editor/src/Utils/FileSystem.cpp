#include "FileSystem.h"
#include <string>
#include <filesystem>
#include <fstream>

#include "../../../Engine/src/AssetDatabase/AssetDatabase.h"
#include "../../../Engine/src/AssetDatabase/AssetLoader.h"

#include "Utilities.h"


namespace stdfs = std::filesystem;

bool gns::editor::fs::FileSystem::ScanDirectoryFor(const std::string& extension, const std::string& RootPath)
{
	return true;
}

std::string gns::editor::fs::FileSystem::GetAbsolutePath(const std::string relativePath)
{
	return { AssetLoader::GetAssetsPath() + relativePath};
}

std::string gns::editor::fs::FileSystem::GetRelativePath(const std::string absolutePath)
{
	// Split the paths by the directory separator
	std::vector<std::string> baseParts = utils::split(AssetLoader::GetAssetsPath(), '\\');
	std::vector<std::string> absoluteParts = utils::split(absolutePath, '\\');

	// Find the common prefix
	size_t commonLength = 0;
	while (commonLength < baseParts.size() && commonLength < absoluteParts.size() &&
		baseParts[commonLength] == absoluteParts[commonLength]) {
		++commonLength;
	}

	// Construct the relative path
	std::string relativePath;

	// Add ".." for each remaining part in the base path
	for (size_t i = commonLength; i < baseParts.size(); ++i) {
		if (!relativePath.empty()) {
			relativePath += "\\";
		}
	}

	// Add the remaining parts of the absolute path
	for (size_t i = commonLength; i < absoluteParts.size(); ++i) {
		if (!relativePath.empty()) {
			relativePath += "\\";
		}
		relativePath += absoluteParts[i];
	}

	return relativePath.empty() ? "." : relativePath; // If no relative path needed, return "."
}

bool gns::editor::fs::FileSystem::WriteFile(const std::string& source, const std::string& path)
{
	
	std::ofstream outputFile(path);

	if (!outputFile.is_open()) {
		// Failed to open file
		LOG_ERROR("Error: Failed to open file " << path << " for writing");
		return false;
	}

	outputFile << source;
	outputFile.close();

	if (!stdfs::exists(path)) {
		stdfs::create_directories(stdfs::path(path).parent_path());
		LOG_INFO("Created directory for file: " << path);
	}
	return true;
}

std::string gns::editor::fs::FileSystem::GetFileExtension(const std::string& path)
{
	size_t lastDotPos = path.find_last_of(".");
	if (lastDotPos != std::string::npos && lastDotPos != path.length() - 1) {
		return utils::toLowerCase(path.substr(lastDotPos + 1));
	}
	return "";
}

std::string gns::editor::fs::FileSystem::AppendExtension(const std::string& filePath, const std::string& extension)
{
	return filePath + extension;
}

std::string gns::editor::fs::FileSystem::ReplaceOrRemoveExtension(const std::string& filePath,
	const std::string& newExtension)
{
	size_t lastDotPos = filePath.find_last_of(".");
	if (lastDotPos != std::string::npos) {
		if (newExtension.empty()) {
			return filePath.substr(0, lastDotPos);
		}
		else {
			return filePath.substr(0, lastDotPos) + "." + newExtension;
		}
	}
	return filePath;
}

std::string gns::editor::fs::FileSystem::GetFileName(const std::string& filePath)
{
	std::filesystem::path path(filePath);
	std::string fileName = path.stem().string();
	return fileName;
}

bool gns::editor::fs::FileSystem::HasExtension(const std::string& filePath, const std::string& extension)
{
	std::filesystem::path path(filePath);

	// Extract the file extension from the path
	std::string fileExtension = path.extension().string();

	// Compare with the given extension
	return fileExtension == extension;
}
