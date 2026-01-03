#include "Globals.h"
#include "Inspector.h"
#include "GameObject.h"

void Inspector::Render()
{
    if (!ImGui::Begin(GetWindowName(), GetOpenPtr(),
        ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::End();
        return;
    }

    if (_selectedGameObject) {
        ImGui::Text(_selectedGameObject->GetName());
    }

    ImGui::End();
}

void Inspector::SetSelectedGameObject(GameObject* gameObject)
{
    _selectedGameObject = gameObject;
}
