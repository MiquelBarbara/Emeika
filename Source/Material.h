#pragma once
#include "Globals.h"

namespace tinygltf { class Model; struct Material; struct PbrMetallicRoughness; }
namespace Emeika {
	class Material
	{
	public:
		void Load(const tinygltf::Model& model, const tinygltf::PbrMetallicRoughness& material, const char* basePath);
	private:
		Vector4 _color;
		ComPtr<ID3D12Resource> _textureColor;
	};
}
	



