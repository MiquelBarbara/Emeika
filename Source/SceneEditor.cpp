#include "Globals.h"
#include "SceneEditor.h"
#include "ImGuizmo.h"
#include <imgui.h>
#include "Transform.h"

#include "Application.h"
#include "D3D12Module.h"
#include "RenderModule.h"
#include "DebugDrawPass.h"

SceneEditor::SceneEditor()
{
    m_Camera = app->GetCameraModule();
    auto d3d12Module = app->GetD3D12Module();
    debugDrawPass = std::make_unique<DebugDrawPass>(d3d12Module->GetDevice(), d3d12Module->GetCommandQueue()->GetD3D12CommandQueue().Get(), false);
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
        ImTextureID textureID = (ImTextureID)app->GetRenderModule()->GetGPUScreenRT().ptr;
        ImGui::Image(textureID, m_Size);
        
    }

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

    if (ImGui::IsKeyPressed(ImGuiKey_T))
            m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    if (ImGui::IsKeyPressed(ImGuiKey_E))
            m_CurrentGizmoOperation = ImGuizmo::ROTATE;
    if (ImGui::IsKeyPressed(ImGuiKey_R))
            m_CurrentGizmoOperation = ImGuizmo::SCALE;

    if (transform && m_Camera && ImGui::IsWindowHovered())
    {
        auto modelMatrix = transform->GetWorldMatrix();

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
            transform->SetWorldMatrix(modelMatrix);
        }
    }

    // Update viewport hover/focus state
    _isViewportHovered = ImGui::IsWindowHovered();
    _isViewportFocused = ImGui::IsWindowFocused();

    ImGui::End();
}

bool SceneEditor::Resize(ImVec2 contentRegion)
{
    if (abs(contentRegion.x - m_Size.x) > 1.0f ||
        abs(contentRegion.y - m_Size.y) > 1.0f) {
        SetSize(contentRegion);
        return true;
    }
    return false;
}

void SceneEditor::RenderDebugDrawPass(ID3D12GraphicsCommandList* commandList)
{
    if (_showGrid) {
        dd::xzSquareGrid(-10.0f, 10.f, 0.0f, 1.0f, dd::colors::LightGray);
    }

    debugDrawPass->record(commandList, GetSize().x, GetSize().y, m_Camera->GetViewMatrix(), m_Camera->GetProjectionMatrix());
}
