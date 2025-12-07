#pragma once
#include "Globals.h"
namespace tinygltf { class Model;  struct Mesh; struct Primitive; }
namespace Emeika {
	class Mesh {
	public:
		void Load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);
	private:
		ComPtr<ID3D12Resource> _vertexBuffer;
		ComPtr<ID3D12Resource> _indexBuffer;
		std::vector<int> _materialIndexs;

		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};
	};
}

