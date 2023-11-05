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

#ifdef LOG_ENABLE
//-----------------
#define LOG_INFO(msg) \
	std::cout << BLUE <<"[Info]: "<< DEFAULT << msg <<" "<<\
	GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_WARNING(msg) \
    std::cout << YELLOW << "[Warning]: " << DEFAULT << msg <<" "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"

#define LOG_ERROR(msg) \
    std::cout << RED << "[Error]: " << DEFAULT << msg << " "<<\
    GRAY << FILE_NAME << "(" << __LINE__ << ")" << DEFAULT << "\n"
//-----------------


# else 

#define LOG_INFO(msg)
#define LOG_WARNING(msg)
#define LOG_ERROR(msg)
#define LOG_VK_INFO(msg)
#define LOG_VK_WARNING(msg)
#define LOG_VK_ERROR(msg)

#endif

