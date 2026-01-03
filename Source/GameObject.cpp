#include "Globals.h"
#include "GameObject.h"
#include "GameObjectManager.h"
#include "Transform.h"

#include "Component.h"

GameObject::GameObject()
{
    GameObjectManager::Instance()->CreateGameObject(this);
}

GameObject::GameObject(const std::string& name): _name(name)
{
	GameObjectManager::Instance()->CreateGameObject(this);

    AddComponent<Transform>();
}


