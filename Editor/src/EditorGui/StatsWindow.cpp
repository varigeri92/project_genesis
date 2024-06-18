#include "StatsWindow.h"

static float updateFreq = 0.5f;
static float _t = 0;
static float drawtime = 0.f;
static ImGuiTableFlags table_flags = ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_Borders;
gns::editor::StatsWindow::StatsWindow() : GuiWindow("Stats")
{
	drawtime = Time::GetDelta();
}

gns::editor::StatsWindow::~StatsWindow()
{
}

void gns::editor::StatsWindow::AssignStat(Stats* stat)
{
	stats_vector.push_back(stat);
}

void gns::editor::StatsWindow::OnGUI()
{	
	float frameTime = Time::GetDelta();
	_t += frameTime;
	if (_t >= 0.15f) {
		drawtime = frameTime;
		_t = 0;
	}
	
	if (ImGui::BeginTable("stats_window_table", 3, table_flags))
	{
		ImGui::TableNextRow();
		ImGui::TableNextColumn(); 
		ImGui::Text("Frame Time:");
		ImGui::TableNextColumn();
		ImGui::Text("%f ms", (drawtime * 1000.f));
		ImGui::TableNextColumn();
		ImGui::Text("%f fps", (1.f / drawtime));
		ImGui::EndTable();
	}
}
