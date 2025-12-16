#include "Globals.h"
#include "ConfigurationView.h"
#include "Application.h"
#include "D3D12Module.h"
#include "DescriptorsModule.h"
#include "TimeModule.h"

void ConfigurationView::Render()
{
	ImGui::Begin("Grid/Axis");
	ImGui::Checkbox("Show/Hide grid and axis", app->GetD3D12Module()->GetShowDebugDrawBool());
	ImGui::End();

	ImGui::Begin("Textures");
	const char* sampleTypes[] = { "LINEAR_WRAP","POINT_WRAP","LINEAR_CLAMP","POINT_CLAMP" };
	if(ImGui::Combo("Texture filtering and adress mode", &currentType, sampleTypes, DescriptorsModule::SampleType::COUNT)){
		app->GetD3D12Module()->SetSampler(currentType);
	}
	ImGui::End();
}

void ConfigurationView::Update()
{
	if (fps_log.size() >= 100)
	{
		fps_log.erase(fps_log.begin());
		ms_log.erase(ms_log.begin());
	}

	fps_log.push_back(1.0f / app->GetTimeModule()->deltaTime());
	ms_log.push_back(app->GetTimeModule()->deltaTime() * 1000.0f);
}
