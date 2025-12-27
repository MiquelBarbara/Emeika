#include "Globals.h"
#include "GameObject.h"
#include "GameObjectManager.h"


GameObject::GameObject()
{
    GameObjectManager::Instance()->CreateGameObject(this);
}

GameObject::GameObject(const std::string& name): _name(name)
{
	GameObjectManager::Instance()->CreateGameObject(this);
}

template<class ...T>
GameObject::GameObject(const std::string& name, T * ...initialComponents)
{
	GameObjectManager::Instance()->CreateGameObject(*this);

	(AddComponent<T>(), ...);
}

template<class T>
T* GameObject::AddComponent()
{
	T* component = new T();
	components.emplace_back(component);
	return component;
}

template<class T>
T* GameObject::GetComponent()
{
    for (auto& component : components)
    {
        T* casted = dynamic_cast<T*>(component.get());
        if (casted != nullptr)
        {
            return casted;
        }
    }

    return nullptr;
}
