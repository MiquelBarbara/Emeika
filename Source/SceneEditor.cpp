#include "Globals.h"
#include "SceneEditor.h"
#include "Application.h"
#include "D3D12Module.h"

SceneEditor::SceneEditor(RenderTexture* renderTexture)
{
}

void SceneEditor::Render()
{
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
    ImGui::Begin("Scene Editor", &m_IsOpen,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse);

    // Get available content region for the scene view
    ImVec2 contentRegion = ImGui::GetContentRegionAvail();

    // Only resize if we have valid dimensions
    if (contentRegion.x > 0 && contentRegion.y > 0) {
        Resize(contentRegion);
        // Display the scene texture if available
        D3D12Module* d3d12 = app->GetD3D12Module();
        RenderTexture* sceneTexture = d3d12->GetOffscreenRenderTarget();

        if (sceneTexture && sceneTexture->GetResource()) {
            // Convert the SRV GPU handle to ImTextureID
            DescriptorHandle srvHandle = sceneTexture->SRV();
            ImTextureID textureID = (ImTextureID)srvHandle.gpu.ptr;

            // Display the texture
            ImGui::Image(textureID, m_Size);
        }
    }

    // Update viewport hover/focus state
    _isViewportHovered = ImGui::IsWindowHovered();
    _isViewportFocused = ImGui::IsWindowFocused();

    ImGui::End();
    ImGui::PopStyleVar();
}

void SceneEditor::Resize(ImVec2 contentRegion)
{
    if (abs(contentRegion.x - m_Size.x) > 1.0f ||
        abs(contentRegion.y - m_Size.y) > 1.0f) {
        SetSize(contentRegion);
    }
}
