#include "Globals.h"
#include "Material.h"
#include "Application.h"
#include "ResourcesModule.h"
#include "tiny_gltf.h"

namespace Emeika {
	void Material::Load(const tinygltf::Model& model, const tinygltf::PbrMetallicRoughness& material,
		const char* basePath)
	{
		_color = Vector4(float(material.baseColorFactor[0]), float(material.baseColorFactor[1]),
			float(material.baseColorFactor[2]), float(material.baseColorFactor[3]));
		if (material.baseColorTexture.index >= 0)
		{
			const tinygltf::Texture& texture = model.textures[material.baseColorTexture.index];
			const tinygltf::Image& image = model.images[texture.source];
			if (!image.uri.empty())
			{
				_textureColor = app->GetResourcesModule()->CreateTexture2DFromFile(std::string(basePath) + image.uri, "Texture")->GetResource();
			}
		}
	}
}

