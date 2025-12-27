#pragma once
#include "Id.h"
#include "GameObject.h"

class GameObjectManager
{
public:
	GameObjectManager();
	void CreateGameObject(GameObject* gameObject);
	void RemoveGameObject(const ID_TYPE id);

	static GameObjectManager* Instance();
private:
	std::vector<GameObject*> objects;

	static GameObjectManager* singleton;
};

