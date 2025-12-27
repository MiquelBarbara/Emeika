#pragma once
#include "Id.h"
#include <string>

class Component;
class GameObjectManager;

// Following GameObject Unity API: https://docs.unity3d.com/6000.3/Documentation/ScriptReference/GameObject.html
class GameObject {
public:
	friend class GameObjectManager;

	GameObject();
	GameObject(const std::string& name);

	template<class... T>
	GameObject(const std::string& name, T*... initialComponents);

	template<class T>
	T* AddComponent();

	// For now we iterate throw all the vector, which is not optimal
	template<class T>
	T* GetComponent();

	ID_TYPE GetId() const { return _id; }
	const char* GetName() { return (char*)_name.c_str(); }
protected:
private:
	std::vector<Component*> components;
	ID_TYPE _id;
	std::string _name;
};


