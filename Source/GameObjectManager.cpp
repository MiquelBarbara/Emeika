#include "Globals.h"
#include "GameObjectManager.h"
#include "GameObject.h"

GameObjectManager* GameObjectManager::singleton = nullptr;

GameObjectManager::GameObjectManager()
{
    singleton = this;
}

void GameObjectManager::CreateGameObject(GameObject* gameObject)
{
    int numGameObjects = objects.size();
    ID_TYPE id = ID_TYPE(numGameObjects);
    gameObject->_id = id;
    objects.emplace_back(gameObject);
}

GameObjectManager* GameObjectManager::Instance()
{
    return singleton;
}



