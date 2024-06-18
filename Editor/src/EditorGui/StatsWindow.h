#pragma once
#include "Engine.h"

namespace gns::editor
{
	class StatsWindow : public gns::gui::GuiWindow
	{
	public:
		StatsWindow();
		~StatsWindow();
		void AssignStat(Stats* stat);
	protected:
		void OnGUI() override;
		float DragValueSensitivity = 0.01f;
	private:

		std::vector<Stats*> stats_vector;
	};
}