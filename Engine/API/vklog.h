#include "Log.h"
#include <vulkan/vulkan.h>

#define VK_LOG
#ifdef VK_LOG

#define LOG_VK_VERBOSE(msg) \
	std::cout << AQUA <<"[VK_Verbose]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_INFO(msg) \
	std::cout << BLUE <<"[VK_Info]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_WARNING(msg) \
    std::cout << YELLOW << "[VK_Warning]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_VK_ERROR(msg) \
    std::cout << RED << "[VK_Error]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define _VK_CHECK(result, msg)\
    if(result != VK_SUCCESS)LOG_VK_ERROR(msg << "Result: " << result)

//-----------------
#else

#define LOG_VK_INFO(msg)
#define LOG_VK_WARNING(msg)
#define LOG_VK_ERROR(msg)
#define _VK_CHECK(result, msg)

#endif