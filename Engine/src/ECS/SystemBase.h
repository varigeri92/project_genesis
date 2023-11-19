#pragma once
#include "../Application.h"
class SystemBase
{
public:
	SystemBase();
	~SystemBase() = default;


protected:
	virtual void OnStart() = 0;
	virtual void OnUpdate() = 0;
	virtual void OnLateUpdate() = 0;
	virtual void OnDestroy() = 0;
};

