#pragma once
#include "GameObject.h"

//TODO: Create a Asset class to be able to store Scenes, Prefabs, Models like in Unity
namespace Emeika {
	class Scene
	{
	public:
		Scene();
		void Add(GameObject* gameObject);
		void Remove(GameObject* gameObject);

		std::vector<GameObject*> GetGameObjects() { return gameObjects; }

		void Render(ID3D12GraphicsCommandList* commandList, Matrix& viewMatrix, Matrix& projectionMatrix);
	private:
		std::vector<GameObject*> gameObjects;
	};
}


