#include "TestClass.h"

#include "Log.h"

TestClass::TestClass()
{
	LOG_INFO("Test class _ctr");
}

void TestClass::OnUpdate()
{
	LOG_INFO("Test class update");
}
