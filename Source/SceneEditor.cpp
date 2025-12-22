#include "Globals.h"
#include "SceneEditor.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ImGuizmo.h"
#include <imgui.h>

SceneEditor::SceneEditor(RenderTexture* renderTexture)
{
    m_Camera = app->GetCameraModule();
}

void SceneEditor::Render()
{
    if (!ImGui::Begin(GetWindowName(), GetOpenPtr(),
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    ImVec2 windowPos = ImGui::GetWindowPos();
    windowX = windowPos.x;
    windowY = windowPos.y;

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

    // ===== CRITICAL: Setup ImGuizmo for THIS window =====
    // Setup must happen AFTER ImGui::Image() to be on top
    ImGuizmo::SetOrthographic(false);
    ImGuizmo::SetDrawlist();

    // Get the content region coordinates (excluding scrollbars)
    ImVec2 contentMin = ImGui::GetWindowContentRegionMin();
    ImVec2 contentMax = ImGui::GetWindowContentRegionMax();
    ImVec2 contentPos(windowPos.x + contentMin.x, windowPos.y + contentMin.y);
    ImVec2 contentSize(contentMax.x - contentMin.x, contentMax.y - contentMin.y);

    ImGuizmo::SetRect(contentPos.x, contentPos.y, contentSize.x, contentSize.y);
    ImGuizmo::Enable(true);

    // ===== SHOW GIZMO CONTROLS INSIDE SCENE EDITOR =====
    if (ImGui::BeginChild("GizmoControls", ImVec2(200, 80), true))
    {
        ImGui::Text("Gizmo Controls:");

        if (ImGui::IsKeyPressed(ImGuiKey_T))
            m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        if (ImGui::IsKeyPressed(ImGuiKey_E))
            m_CurrentGizmoOperation = ImGuizmo::ROTATE;
        if (ImGui::IsKeyPressed(ImGuiKey_R))
            m_CurrentGizmoOperation = ImGuizmo::SCALE;

        if (ImGui::RadioButton("Translate", m_CurrentGizmoOperation == ImGuizmo::TRANSLATE))
            m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Rotate", m_CurrentGizmoOperation == ImGuizmo::ROTATE))
            m_CurrentGizmoOperation = ImGuizmo::ROTATE;
        ImGui::SameLine();
        if (ImGui::RadioButton("Scale", m_CurrentGizmoOperation == ImGuizmo::SCALE))
            m_CurrentGizmoOperation = ImGuizmo::SCALE;

        // Mode selection (only for translate/rotate)
        if (m_CurrentGizmoOperation != ImGuizmo::SCALE)
        {
            if (ImGui::RadioButton("Local", m_CurrentGizmoMode == ImGuizmo::LOCAL))
                m_CurrentGizmoMode = ImGuizmo::LOCAL;
            ImGui::SameLine();
            if (ImGui::RadioButton("World", m_CurrentGizmoMode == ImGuizmo::WORLD))
                m_CurrentGizmoMode = ImGuizmo::WORLD;
        }

        ImGui::EndChild();
    }

    // ===== APPLY GIZMO TO SELECTED MODEL =====
    if (m_SelectedModel && m_Camera && ImGui::IsWindowHovered())
    {
        auto modelMatrix = m_SelectedModel->GetWorldMatrix();

        // Apply the gizmo manipulation
        ImGuizmo::Manipulate(
            (float*)&m_Camera->GetViewMatrix(),
            (float*)&m_Camera->GetProjectionMatrix(),
            m_CurrentGizmoOperation,
            m_CurrentGizmoMode,
            (float*)&modelMatrix,
            NULL,
            NULL
        );

        // If gizmo was used, update the model
        if (ImGuizmo::IsUsing())
        {
            m_SelectedModel->SetWorldMatrix(modelMatrix);
        }
    }

    // Update viewport hover/focus state
    _isViewportHovered = ImGui::IsWindowHovered();
    _isViewportFocused = ImGui::IsWindowFocused();

    ImGui::End();
}

void SceneEditor::Resize(ImVec2 contentRegion)
{
    if (abs(contentRegion.x - m_Size.x) > 1.0f ||
        abs(contentRegion.y - m_Size.y) > 1.0f) {
        SetSize(contentRegion);
    }
}