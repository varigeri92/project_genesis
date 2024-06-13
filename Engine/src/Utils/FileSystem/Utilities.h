#include "Engine.h"
#include <string>
#include <filesystem>
#include <fstream>

namespace gns::utils
{
	std::string toLowerCase(const std::string& str);
	std::vector<std::string> split(const std::string& str, char delimiter);
}
