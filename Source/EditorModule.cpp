#include "Globals.h"
#include "EditorModule.h"
#include "D3D12Module.h"
#include "CameraModule.h"
#include "vector"
#include <backends/imgui_impl_dx12.h>
#include "Resources.h"
#include "SceneEditor.h"
#include "HardwareWindow.h"
#include "PerformanceWindow.h"
#include "EditorWindow.h"
#include "EditorTransform.h"
#include "ImGuizmo.h"

using namespace std;

Logger* logger = nullptr;

// In the future this will be also a EditorWindow
void MainMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Edit"))
        {
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}


void Style() {
    ImGuiStyle& style = ImGui::GetStyle();
    style.WindowRounding = 5.0f;
    style.FrameRounding = 4.0f;
    style.ScrollbarRounding = 5.0f;
    style.GrabRounding = 4.0f;

    style.Colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);
    style.Colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    style.Colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);
    style.Colors[ImGuiCol_Button] = ImVec4(0.2f, 0.205f, 0.21f, 1.0f);
    style.Colors[ImGuiCol_ButtonHovered] = ImVec4(0.3f, 0.305f, 0.31f, 1.0f);
    style.Colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.1505f, 0.151f, 1.0f);

}


void EditorModule::MainDockspace(bool* p_open)
{
    // Fullscreen window flags
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiViewportFlags_NoAutoMerge |
        ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockspaceWindow", p_open, window_flags);

    ImGui::PopStyleVar(2);

    MainMenuBar();
    // Create the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0, 0), ImGuiDockNodeFlags_None);

    if (_firstFrame) {
        for (auto it = _editorWindows.begin(); it != _editorWindows.end(); ++it)
            (*it)->Render();

        SetupDockLayout(dockspace_id);
        Style();
        _firstFrame = false;
    }

    ImGui::End();
}


void EditorModule::SetupDockLayout(ImGuiID dockspace_id)
{

    // Clear previous layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->Size);

    ImGuiID dock_main = dockspace_id;

    // Split main dock into left (for Grid/Axis + Texture) and right (for everything else)
    ImGuiID dock_left, dock_right;
    ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.25f, &dock_left, &dock_right);

    ImGuiID dock_right_top, dock_right_bottom;
    ImGui::DockBuilderSplitNode(dock_right, ImGuiDir_Down, 0.40f, &dock_right_bottom, &dock_right_top);

    ImGuiID dock_bottom_left, dock_bottom_right;
    ImGui::DockBuilderSplitNode(dock_right_bottom, ImGuiDir_Left, 0.5f, &dock_bottom_left, &dock_bottom_right);

    ImGuiID dock_bottom_right_top, dock_bottom_right_bottom;
    ImGui::DockBuilderSplitNode(dock_bottom_right, ImGuiDir_Up, 0.5f, &dock_bottom_right_top, &dock_bottom_right_bottom);

    ImGui::DockBuilderDockWindow("Grid/Axis", dock_left);
    ImGui::DockBuilderDockWindow("Textures", dock_left);

    ImGui::DockBuilderDockWindow("Scene Editor", dock_right_top);

    ImGui::DockBuilderDockWindow("Console", dock_bottom_left);

    ImGui::DockBuilderDockWindow("Hardware Info", dock_bottom_right_top);

    ImGui::DockBuilderDockWindow("Performance", dock_bottom_right_top);

    ImGui::DockBuilderFinish(dockspace_id);
}

EditorModule::EditorModule()
{
    //_console = Console();
    _editorWindows.push_back(_logger = new Logger());
    _editorWindows.push_back(_hardwareWindow = new HardwareWindow());
    _editorWindows.push_back(_performanceWindow = new PerformanceWindow());
    _configurationView = new ConfigurationView();
}

EditorModule::~EditorModule()
{
	_gui->~ImGuiPass();
    //_sceneView->~SceneView();
    _debugDrawPass->~DebugDrawPass();
}

bool EditorModule::postInit()
{
	D3D12Module* _d3d12 = app->GetD3D12Module();
	_gui = new ImGuiPass(_d3d12->GetDevice(), _d3d12->GetWindowHandle(), app->GetDescriptorsModule()->GetSRV()->GetCPUHandle(0), app->GetDescriptorsModule()->GetSRV()->GetGPUHandle(0));
    _editorWindows.push_back(_sceneView = new SceneEditor(_d3d12->GetOffscreenRenderTarget()));
    
    auto duckModel = app->GetD3D12Module()->GetDuck();
    // Set the duck model in SceneEditor
    _sceneView->SetSelectedModel(duckModel);
    // Create EditorTransform and pass the SceneEditor reference
    _editorWindows.push_back(new EditorTransform(duckModel, _sceneView));

	return true;
}

void EditorModule::update()
{
    for (auto it = _editorWindows.begin(); it != _editorWindows.end(); ++it)
        (*it)->Update();
}

void EditorModule::preRender()
{
	_gui->startFrame();
    ImGuizmo::BeginFrame();

    MainMenuBar();
    MainDockspace(&_showMainDockspace);

    for (auto it = _editorWindows.begin(); it != _editorWindows.end(); ++it)
        (*it)->Render();

    _configurationView->Update();
    _configurationView->Render();

    ImGui::EndFrame();
}

void EditorModule::render()
{
	_gui->record(app->GetD3D12Module()->GetCommandList());
}

void EditorModule::postRender()
{
}

bool EditorModule::cleanUp()
{
    for (auto it = _editorWindows.begin(); it != _editorWindows.end(); ++it)
        (*it)->CleanUp();
    return true;
}
