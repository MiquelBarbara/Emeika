#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"

namespace tinygltf { class Model; }

namespace Emeika {
	class Model
	{
	public:
		void Load(const char* fileName, const char* basePath);
		Transform& GetTransform() { return _transform; }
		Matrix& GetWorldMatrix() { return _transform.GetWorldMatrix(); }
		void SetWorldMatrix(Matrix& matrix) { _transform.SetWorldMatrix(matrix); }

		std::vector<Emeika::Mesh*> GetMeshes() { return _meshes; }
		std::vector<Emeika::Material*> GetMaterials() { return _materials; }
	private:
		// In future both both Transform and Model should be components in an ECS pattern
		Transform _transform;

		std::vector<Emeika::Mesh*> _meshes;
		std::vector<Emeika::Material*> _materials;
	};
}




