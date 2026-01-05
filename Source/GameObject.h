#pragma once
#include "Id.h"
#include <string>

class GameObjectManager;
class Component;

// Following GameObject Unity API: https://docs.unity3d.com/6000.3/Documentation/ScriptReference/GameObject.html
class GameObject {
public:
	friend class GameObjectManager;

	GameObject();
	GameObject(const std::string& name);

	template<class... Component>
	GameObject(const std::string& name, Component*... initialComponents);

	template<class Component>
	Component* AddComponent();

	template<class Component>
	Component* AddComponent(Component* component);

	// For now we iterate throw all the vector, which is not optimal
	template<class Component>
	Component* GetComponent();

	ID_TYPE GetId() const { return _id; }
	const char* GetName() { return (char*)_name.c_str(); }
	void SetName(const char* name) { _name = name; }
protected:
private:
	std::vector<Component*> components;
	ID_TYPE _id;
	std::string _name;
};

template<class ...Component>
GameObject::GameObject(const std::string& name, Component * ...initialComponents)
{
	GameObjectManager::Instance()->CreateGameObject(*this);

	(AddComponent<Component>(), ...);
}

template<class Component>
Component* GameObject::AddComponent()
{
	Component* component = new Component();
	components.emplace_back(component);
	return component;
}

template<class Component>
Component* GameObject::AddComponent(Component* component)
{
	components.emplace_back(component);
	return component;
}

template<class Component>
Component* GameObject::GetComponent()
{
	for (auto& component : components)
	{
		Component* casted = dynamic_cast<Component*>(component);
		if (casted != nullptr)
		{
			return casted;
		}
	}

	return nullptr;
}


