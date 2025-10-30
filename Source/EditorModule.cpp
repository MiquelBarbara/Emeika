#include "Globals.h"
#include "EditorModule.h"
#include "D3D12Module.h"


EditorModule::EditorModule()
{

}

EditorModule::~EditorModule()
{
	_gui->~ImGuiPass();
}

bool EditorModule::postInit()
{
	D3D12Module* _d3d12 = app->getD3D12Module();
	_gui = new ImGuiPass(_d3d12->getDevice(), _d3d12->getWindowHandle());
	return true;
}

void EditorModule::preRender()
{
	_gui->startFrame();

	ImGui::ShowDemoWindow();

	ImGui::EndFrame();
}

void EditorModule::render()
{

	_gui->record(app->getD3D12Module()->getCommandList());
}
