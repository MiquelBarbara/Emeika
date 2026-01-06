#pragma once
#include "GameObject.h"

struct SceneData {
	Vector3 lightDirection;
	float pad0;
	Vector3 lightColor;
	float pad1;
	Vector3 ambientColor;
	float pad2;
	Vector3 view;
	float pad3;
};

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

		SceneData& GetData();
	private:
		std::vector<GameObject*> gameObjects;
		SceneData sceneData;
	};
}


