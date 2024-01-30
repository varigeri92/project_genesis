#include "core.h"
#include "Log.h"
#include "paths.h"
int main()
{
    LOG_INFO("HELLO WORLD");
	return CreateApplication(ASSETS_PATH);
}

