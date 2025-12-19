#pragma once
#include "Globals.h"
namespace tinygltf { class Model;  struct Mesh; struct Primitive; }
namespace Emeika {
	class Mesh {
	public:
		void Load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive);
		D3D12_VERTEX_BUFFER_VIEW* GetVertexBufferView() { return &_vertexBufferView; }
		D3D12_INDEX_BUFFER_VIEW* GetIndexBufferView() { return &_indexBufferView; }

		bool HasIndexBuffer() const { return _indexBuffer.Get() != nullptr; }

		ComPtr<ID3D12Resource> GetVertexBuffer() const { return _vertexBuffer; }
		uint32_t GetNumVertices() const { return numVertices; }
		ComPtr<ID3D12Resource> GetIndexBuffer() const { return _indexBuffer; }
		uint32_t GetNumIndices() const { return numIndices; }

		int32_t GetMaterialIndex() const { return _materialIndex; }

	private:
		ComPtr<ID3D12Resource> _vertexBuffer;
		ComPtr<ID3D12Resource> _indexBuffer;
		int32_t _materialIndex{ -1 };

		uint32_t numVertices;
		uint32_t numIndices;

		D3D12_VERTEX_BUFFER_VIEW _vertexBufferView = {};
		D3D12_INDEX_BUFFER_VIEW _indexBufferView = {};
	};
}

