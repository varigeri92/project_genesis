#pragma once


#ifdef BUILD_ENGINE_LIB
	#define GNS_API __declspec(dllexport)
#else
	#define GNS_API __declspec(dllimport)
#endif

#ifdef BUILD_GAME_LIB
	#define Application_API __declspec(dllexport)
#else
	#define Application_API __declspec(dllimport)
#endif

#ifdef BUILD_EEDITOR_LIB
	#define Editor_API __declspec(dllexport)
#else
	#define Editor_API __declspec(dllimport)
#endif


