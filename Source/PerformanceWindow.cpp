#include "Globals.h"
#include "PerformanceWindow.h"
#include "Application.h"
#include "TimeModule.h"
#include "PerformanceProfiler.h"

void PerformanceWindow::Update()
{
	if (fps_log.size() >= 100)
	{
		fps_log.pop_front();
		ms_log.pop_front();
	}

	fps_log.push_back(1.0f / app->GetTimeModule()->deltaTime());
	ms_log.push_back(app->GetTimeModule()->deltaTime() * 1000.0f);
}

void PerformanceWindow::Render()
{
	if (!ImGui::Begin(GetWindowName(), GetOpenPtr(),
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}

	char title[64];
	sprintf_s(title, sizeof(title), "Framerate %.1f", fps_log[fps_log.size() - 1]);
	ImGui::PlotHistogram("##framerate", &fps_log[0], fps_log.size(), 0, title, 0.0f, 100.0f, ImVec2(310, 100));
	sprintf_s(title, sizeof(title), "Milliseconds %0.1f", ms_log[ms_log.size() - 1]);
	ImGui::PlotHistogram("##milliseconds", &ms_log[0], ms_log.size(), 0, title, 0.0f, 50.0f, ImVec2(310, 100));

    ImGui::Separator();
    ImGui::TextDisabled("CPU Profiling");

    const auto& data = GetPerfData();

    if (ImGui::BeginTable("PerfTable", 4,
        ImGuiTableFlags_RowBg |
        ImGuiTableFlags_Borders |
        ImGuiTableFlags_SizingStretchProp))
    {
        ImGui::TableSetupColumn("Scope");
        ImGui::TableSetupColumn("Last (ms)");
        ImGui::TableSetupColumn("Avg (ms)");
        ImGui::TableSetupColumn("Max (ms)");
        ImGui::TableHeadersRow();

        for (const auto& [name, d] : data)
        {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text("%s", name.c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%.2f", d.lastMs);

            ImGui::TableSetColumnIndex(2);
            ImGui::Text("%.2f", d.avgMs);

            ImGui::TableSetColumnIndex(3);
            ImGui::Text("%.2f", d.maxMs);
        }

        ImGui::EndTable();
    }

	ImGui::End();
}
