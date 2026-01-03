#pragma once
#include "Resources.h"
#include "CameraModule.h"
#include "EditorWindow.h"
#include "ImGuizmo.h"

class Transform;
class DebugDrawPass;

class SceneEditor: public EditorWindow
{
public:
    SceneEditor();
    const char* GetWindowName() const override { return "Scene Editor"; }
    void Render() override;
    bool Resize(ImVec2 contentRegion);

    void SetSelectedGameObject(Transform* transform) { this->transform = transform; }

    ImGuizmo::OPERATION GetCurrentOperation() const { return m_CurrentGizmoOperation; }
    ImGuizmo::MODE GetCurrentMode() const { return m_CurrentGizmoMode; }

    void RenderDebugDrawPass(ID3D12GraphicsCommandList* commandList);

private:
    //DebugDrawPass
    std::unique_ptr<DebugDrawPass> debugDrawPass;
    bool _showGrid = true;
    bool _showAxis = true;

    CameraModule* m_Camera;
    Transform* transform;

    ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::LOCAL;
};

