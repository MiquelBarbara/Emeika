#pragma once
#include "EditorWindow.h"
#include "GameObject.h"

class Hierarchy: public EditorWindow
{
	void Render() override;
	const char* GetWindowName() const override { return "Hierarchy"; }
	void AddGameObject();
private:
	std::vector<GameObject*> gameObjects;
};

