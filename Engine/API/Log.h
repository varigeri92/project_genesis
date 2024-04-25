#pragma once
#include <iostream>
#include <cstring>

#define LOG_ENABLE


#define RED     "\x1B[31m"
#define GREEN   "\x1B[32m"
#define BLUE    "\x1B[34m"
#define YELLOW  "\x1B[33m"
#define AQUA    "\x1B[36m"
#define GRAY    "\x1B[90m"
#define DEFAULT "\x1B[0m"

#define FILE_NAME (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)


#define PROJECT_SRC GREEN << "[SANDBOX]:"

#ifdef BUILD_ENGINE_LIB
#define PROJECT_SRC GREEN << "[ENGINE]:"
#endif

#ifdef BUILD_GAME_LIB
#define PROJECT_SRC GREEN << "[APPLICATION]:"
#endif

#ifdef BUILD_EEDITOR_LIB
#define PROJECT_SRC GREEN << "[EDITOR]:"

#endif

#ifdef LOG_ENABLE
//-----------------
#define LOG_TRACE(msg) \
	std::cout<< PROJECT_SRC << AQUA <<"[Trace]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_INFO(msg) \
	std::cout<< PROJECT_SRC  << BLUE <<"[Info]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_WARNING(msg) \
    std::cout<< PROJECT_SRC  << YELLOW << "[Warning]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_ERROR(msg) \
    std::cout<< PROJECT_SRC  << RED << "[Error]: " << DEFAULT << msg << " "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define GNS_ASSERT(cond, message)  LOG_ERROR(message) assert(cond)
//-----------------


# else

#define LOG_TRACE(msg)
#define LOG_INFO(msg)
#define LOG_WARNING(msg)
#define LOG_ERROR(msg)

#define GNS_ASSERT(cond, message)

#endif

