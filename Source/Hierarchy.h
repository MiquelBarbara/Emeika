#pragma once
#include "EditorWindow.h"
#include "GameObject.h"

namespace Emeika { class Scene; }

class Hierarchy: public EditorWindow
{
public:
	Hierarchy();
	void SetOnSelectedGameObject(std::function<void(GameObject*)> f) {
		OnSelectedGameObject.push_back(f);
	}
	void Render() override;
	const char* GetWindowName() const override { return "Hierarchy"; }
	void AddGameObject();
private:
	//TODO: This should be a vector
	Emeika::Scene* scene;
	std::vector<std::function<void(GameObject*)>> OnSelectedGameObject;
};

