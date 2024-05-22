#include "Engine.h"
#include <string>
#include <filesystem>
#include <fstream>

namespace gns::editor::utils
{

	inline bool hasExtension(const std::string& filePath, const std::string& extension) {
		std::filesystem::path path(filePath);

		// Extract the file extension from the path
		std::string fileExtension = path.extension().string();

		// Compare with the given extension
		return fileExtension == extension;
	}

	inline std::string GetFileName(const std::string& filePath) {
		std::filesystem::path path(filePath);
		std::string fileName = path.stem().string();
		return fileName;
	}

	inline std::string replaceOrRemoveExtension(const std::string& filePath, const std::string& newExtension = "") {
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

	inline std::string appendExtension(const std::string& filePath, const std::string& extension) {

		return filePath + extension;
	}

	inline std::string toLowerCase(const std::string& str) {
		std::string result;
		for (char c : str) {
			result += std::tolower(c);
		}
		return result;
	}

	inline std::string getFileExtension(const std::string& filePath) {
		size_t lastDotPos = filePath.find_last_of(".");
		if (lastDotPos != std::string::npos && lastDotPos != filePath.length() - 1) {
			return toLowerCase(filePath.substr(lastDotPos + 1));
		}
		return "";
	}

	inline bool writeFile(const std::string& source, const std::string& path) {
		namespace fs = std::filesystem;
		std::ofstream outputFile(path);

		if (!outputFile.is_open()) {
			// Failed to open file
			LOG_ERROR("Error: Failed to open file " << path << " for writing");
			return false;
		}

		outputFile << source;
		outputFile.close();

		if (!fs::exists(path)) {
			fs::create_directories(fs::path(path).parent_path());
			LOG_INFO("Created directory for file: " << path);
		}

		LOG_INFO("Data written to file: " << path);
		return true;
	}


	// Utility function to split a string by a delimiter and return a vector of parts
	inline std::vector<std::string> split(const std::string& str, char delimiter) {
		std::vector<std::string> parts;
		std::stringstream ss(str);
		std::string item;
		while (std::getline(ss, item, delimiter)) {
			if (!item.empty()) {
				parts.push_back(item);
			}
		}
		return parts;
	}

	// Function to convert absolute path to relative path
	inline std::string getRelativePath(const std::string& basePath, const std::string& absolutePath) {
		// Split the paths by the directory separator
		std::vector<std::string> baseParts = split(basePath, '\\');
		std::vector<std::string> absoluteParts = split(absolutePath, '\\');

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
}
