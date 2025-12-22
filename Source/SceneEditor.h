#pragma once
#include "Resources.h"
#include "CameraModule.h"
#include "EditorWindow.h"
#include "Model.h"
#include "ImGuizmo.h"

class SceneEditor: public EditorWindow
{
public:
    SceneEditor(RenderTexture* renderTexture);
    const char* GetWindowName() const override { return "Scene Editor"; }
    void Render() override;
    void Resize(ImVec2 contentRegion);

    void SetSelectedModel(Emeika::Model* model) { m_SelectedModel = model; }

    ImGuizmo::OPERATION GetCurrentOperation() const { return m_CurrentGizmoOperation; }
    ImGuizmo::MODE GetCurrentMode() const { return m_CurrentGizmoMode; }

private:
    CameraModule* m_Camera;

    Emeika::Model* m_SelectedModel;
    ImGuizmo::OPERATION m_CurrentGizmoOperation = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE m_CurrentGizmoMode = ImGuizmo::LOCAL;
};

