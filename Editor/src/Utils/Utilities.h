#include "Engine.h"
#include <string>
#include <filesystem>
#include <fstream>

namespace gns::editor::utils
{
	inline std::string toLowerCase(const std::string& str) {
		std::string result;
		for (char c : str) {
			result += std::tolower(c);
		}
		return result;
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

}
