#include "Globals.h"
#include "EditorModule.h"
#include "D3D12Module.h"
#include "CameraModule.h"
#include "vector"
#include <backends/imgui_impl_dx12.h>
#include "Resources.h"

using namespace std;

Logger* logger = nullptr;

void EditorModule::RenderSceneEditorWindow()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene Editor", &_showSceneEditor,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    // Get available content region for the scene view
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    // Only resize if we have valid dimensions
    if (contentRegion.x > 0 && contentRegion.y > 0) {
        // Check if we need to resize (with some threshold to avoid constant resizing)
        if (abs(contentRegion.x - _sceneViewSize.x) > 1.0f ||
            abs(contentRegion.y - _sceneViewSize.y) > 1.0f) {
            _sceneViewSize = contentRegion;

            // Request resize of scene texture in D3D12Module
            D3D12Module* d3d12 = app->GetD3D12Module();
            // You'll need to add a method to set the resize flag
            // d3d12->SetSceneTextureSize(_sceneViewSize);
        }

        // Display the scene texture if available
        D3D12Module* d3d12 = app->GetD3D12Module();
        RenderTexture* sceneTexture = d3d12->GetOffscreenRenderTarget();

        if (sceneTexture && sceneTexture->GetResource()) {
            // Convert the SRV GPU handle to ImTextureID
            DescriptorHandle srvHandle = sceneTexture->SRV();
            ImTextureID textureID = (ImTextureID)srvHandle.gpu.ptr;

            // Display the texture
            ImGui::Image(textureID, _sceneViewSize);
        }
        else {
            // Fallback: draw a placeholder
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p0 = ImGui::GetCursorScreenPos();
            ImVec2 p1 = ImVec2(p0.x + contentRegion.x, p0.y + contentRegion.y);
            drawList->AddRectFilled(p0, p1, IM_COL32(30, 30, 40, 255));
            drawList->AddText(ImVec2(p0.x + 10, p0.y + 10), IM_COL32(255, 255, 255, 255),
                "Scene View - Render Texture Not Ready");
        }
    }

    // Update viewport hover/focus state
    _isViewportHovered = ImGui::IsWindowHovered();
    _isViewportFocused = ImGui::IsWindowFocused();

    ImGui::End();
    ImGui::PopStyleVar();
}

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
            if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
            if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {} // Disabled item
            ImGui::Separator();
            if (ImGui::MenuItem("Cut", "CTRL+X")) {}
            if (ImGui::MenuItem("Copy", "CTRL+C")) {}
            if (ImGui::MenuItem("Paste", "CTRL+V")) {}
            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
}

void MainDockspace(bool* p_open)
{
    // Fullscreen window flags
    ImGuiWindowFlags window_flags =
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus;

    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);
    ImGui::SetNextWindowViewport(viewport->ID);

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);

    ImGui::Begin("MainDockspaceWindow", p_open, window_flags);

    ImGui::PopStyleVar(2);

    // Create the DockSpace
    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), ImGuiDockNodeFlags_None);

    ImGui::End();
}

void SetupDockLayout()
{
    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");

    // Clear previous layout
    ImGui::DockBuilderRemoveNode(dockspace_id);
    ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

    ImGui::DockBuilderFinish(dockspace_id);
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


EditorModule::EditorModule()
{
    //_console = Console();
    logger = new Logger();
    _configurationView = new ConfigurationView();

}

EditorModule::~EditorModule()
{
	_gui->~ImGuiPass();
    //_sceneView->~SceneView();
    logger->~Logger();
    _debugDrawPass->~DebugDrawPass();
}

bool EditorModule::postInit()
{
	D3D12Module* _d3d12 = app->GetD3D12Module();
    
	_gui = new ImGuiPass(_d3d12->GetDevice(), _d3d12->GetWindowHandle(), app->GetDescriptorsModule()->GetSRV()->GetCPUHandle(0), app->GetDescriptorsModule()->GetSRV()->GetGPUHandle(0));

	return true;
}

void EditorModule::preRender()
{
	_gui->startFrame();

    if(_firstFrame){
		SetupDockLayout();
        Style();
		_firstFrame = false;
	}

    RenderSceneEditorWindow();

    _configurationView->Update();
    _configurationView->Render();

    D3D12Module* _d3d12 = app->GetD3D12Module();


    // TODO: Main Menu Bar, Inspector, Asset Browser, hierarchy, scene.
    //MainMenuBar();
    //MainDockspace(&_showMainDockspace);

	ImGui::ShowDemoWindow();
    logger->Draw("Console");

    ImGui::EndFrame();
}

void EditorModule::render()
{

	_gui->record(app->GetD3D12Module()->GetImGuiCommandList());

}

void EditorModule::postRender()
{
}
