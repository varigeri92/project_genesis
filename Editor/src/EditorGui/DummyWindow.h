﻿#pragma once
#include "Engine.h"

class DummyWindow final : public gns::gui::GuiWindow
{
	struct IconDefine
	{
		std::string hexValue;
		std::string nameString;
		std::string unicode;
	};
	std::vector<IconDefine> icons;
public:
	DummyWindow(const std::string& name);

protected:
	void OnGUI() override;

public:
	
};