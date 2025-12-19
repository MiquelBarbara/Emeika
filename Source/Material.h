#pragma once
#include "Globals.h"
#include "Resources.h"

namespace tinygltf { class Model; struct Material; struct PbrMetallicRoughness; }
namespace Emeika {
	class Material
	{
	public:
		struct MaterialData {
			Vector4 baseColor;
			BOOL hasColourTexture;  // use BOOL (4 bytes) instead of c++ bool (1 byte) as HLSL bool is 4 bytes long
		};

		void Load(const tinygltf::Model& model, const tinygltf::PbrMetallicRoughness& material, const char* basePath);
		ComPtr<ID3D12Resource> GetMaterialBuffer() const { return materialBuffer; }
		Texture* GetTexture() const { return _textureColor.get(); }
	private:
		uint32_t index;
		Vector4 _color;
		std::unique_ptr<Texture> _textureColor;
		ComPtr<ID3D12Resource> materialBuffer;
	};
}
	



