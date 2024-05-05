#pragma once
#include "../Utils/Debuging/Instrumentor.h"

#ifdef ENABLE_PROFILER

#define START_PROFILER(session_Name)  Instrumentor::Get().BeginSession("Session Name");

#define PROFILE_FUNC  InstrumentationTimer timer(__FUNCSIG__);
#define PROFILE_SCOPE(label) InstrumentationTimer timer(label);

#define END_PROFILER  Instrumentor::Get().EndSession();;


#else

#define START_PROFILER(session_Name)
#define PROFILE_FUNC
#define PROFILE_SCOPE(X)
#define END_PROFILER

#endif

