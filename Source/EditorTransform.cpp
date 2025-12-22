#include "Globals.h"
#include "EditorTransform.h"
#include "Application.h"
#include "CameraModule.h"
#include "SceneEditor.h"
#include "D3D12Module.h"

EditorTransform::EditorTransform(Emeika::Model* model, SceneEditor* sceneEditor) :
    model(model), m_SceneEditor(sceneEditor)
{

}

void EditorTransform::Render()
{
    if (!ImGui::Begin(GetWindowName(), GetOpenPtr(),
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    // Get current model matrix
    auto modelMatrix = model->GetWorldMatrix();

    // Decompose matrix for manual editing
    float matrixTranslation[3], matrixRotation[3], matrixScale[3];
    ImGuizmo::DecomposeMatrixToComponents((float*)&modelMatrix, matrixTranslation, matrixRotation, matrixScale);

    // ===== MANUAL TRANSFORM CONTROLS =====
    ImGui::Text("Transform:");
    if (ImGui::InputFloat3("Position", matrixTranslation))
    {
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&modelMatrix);
        model->SetWorldMatrix(modelMatrix);
    }

    if (ImGui::InputFloat3("Rotation", matrixRotation))
    {
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&modelMatrix);
        model->SetWorldMatrix(modelMatrix);
    }

    if (ImGui::InputFloat3("Scale", matrixScale))
    {
        ImGuizmo::RecomposeMatrixFromComponents(matrixTranslation, matrixRotation, matrixScale, (float*)&modelMatrix);
        model->SetWorldMatrix(modelMatrix);
    }

    ImGui::End();
}