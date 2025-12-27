#include "Globals.h"
#include "Hierarchy.h"
#include "GameObject.h"

void Hierarchy::Render()
{
	if (!ImGui::Begin(GetWindowName(), GetOpenPtr(),
		ImGuiWindowFlags_AlwaysAutoResize))
	{
		ImGui::End();
		return;
	}
	//Add gameObjects / filter by name
	if (ImGui::Button("+")) {
		AddGameObject();
	}

	ImGui::Separator();

	// Scene tree node
	if (ImGui::TreeNode("Scene#1")) {
		// Game Objects tree
		for (int i = 0; i < gameObjects.size(); ++i) {
			if (ImGui::TreeNode(gameObjects[i]->GetName())) {
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();
}

void Hierarchy::AddGameObject()
{
	int size = gameObjects.size();
	GameObject* gameObject = new GameObject(std::string("Game Object") + std::to_string(size));
	gameObjects.emplace_back(gameObject);
}
