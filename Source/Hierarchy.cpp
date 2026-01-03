#include "Globals.h"
#include "Hierarchy.h"
#include "GameObject.h"

#include "Scene.h"

#include "Application.h"
#include "RenderModule.h"

Hierarchy::Hierarchy()
{

}

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

	//TODO: Load the assets by dragging it in the hierarchy
	scene = app->GetRenderModule()->GetScene();

	// Scene tree node
	if (ImGui::TreeNode("Scene#1")) {
		// Game Objects tree
		auto gameObjects = scene->GetGameObjects();
		for (int i = 0; i < gameObjects.size(); ++i) {
			if (ImGui::TreeNode(gameObjects[i]->GetName())) {
				if (OnSelectedGameObject.size() > 0) {
					for (auto event : OnSelectedGameObject) {
						event(gameObjects[i]);
					}
				}
				ImGui::TreePop();
			}
		}
		ImGui::TreePop();
	}
	ImGui::End();
}

void Hierarchy::AddGameObject()
{
	auto gameObjects = scene->GetGameObjects();
	int size = gameObjects.size();
	GameObject* gameObject = new GameObject(std::string("Game Object") + std::to_string(size));
	scene->Add(gameObject);
}
