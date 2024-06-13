#include "gnspch.h"

namespace gns::utils
{
	std::string toLowerCase(const std::string& str) {
		std::string result;
		for (char c : str) {
			result += std::tolower(c);
		}
		return result;
	}

	std::vector<std::string> split(const std::string& str, char delimiter) {
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