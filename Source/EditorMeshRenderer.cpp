#include "Globals.h"
#include "EditorMeshRenderer.h"

void EditorMeshRenderer::Render()
{
    if (!ImGui::Begin(GetName(), GetOpenPtr(),
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }


    ImGui::End();
}
