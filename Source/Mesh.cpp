#include "Globals.h"
#include "UtilityGLFT.h"
#include "Application.h"
#include "ResourcesModule.h"
#include "Mesh.h"

D3D12_INDEX_BUFFER_VIEW CreateIndexBufferView(D3D12_GPU_VIRTUAL_ADDRESS adress, UINT size, uint32_t numIndices) {
	static const DXGI_FORMAT formats[3] = { DXGI_FORMAT_R8_UINT, DXGI_FORMAT_R16_UINT, DXGI_FORMAT_R32_UINT };
	D3D12_INDEX_BUFFER_VIEW indexBufferView = {};

	indexBufferView.BufferLocation = adress;
	indexBufferView.Format = formats[size >> 1];
	indexBufferView.SizeInBytes = numIndices * size;

	return indexBufferView;
}

D3D12_VERTEX_BUFFER_VIEW CreteVertexBufferView(D3D12_GPU_VIRTUAL_ADDRESS adress, UINT size, uint32_t numVertices) {
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView = {};

	vertexBufferView.BufferLocation = adress;
	vertexBufferView.StrideInBytes = size;
	vertexBufferView.SizeInBytes = numVertices * size;

	return vertexBufferView;
}

void Emeika::Mesh::Load(const tinygltf::Model& model, const tinygltf::Mesh& mesh, const tinygltf::Primitive& primitive)
{
	const auto& itPos = primitive.attributes.find("POSITION");
	if (itPos != primitive.attributes.end()) // If no position no geometry data
	{
		uint32_t numVertices = uint32_t(model.accessors[itPos->second].count);
		Vertex* vertices = new Vertex[numVertices];
		uint8_t* vertexData = (uint8_t*)vertices; // Casts Vertex Buffer to Bytes (uint8_t*) buffer
		LoadAccessorData(vertexData + offsetof(Vertex, position), sizeof(Vector3), sizeof(Vertex),
			numVertices, model, itPos->second);
		LoadAccessorData(vertexData + offsetof(Vertex, texCoord0), sizeof(Vector2), sizeof(Vertex),
			numVertices, model, primitive.attributes, "TEXCOORD_0");

		_vertexBuffer = app->GetResourcesModule()->CreateDefaultBuffer(vertices, numVertices * sizeof(Vertex));
		_vertexBufferView = CreteVertexBufferView(_vertexBuffer->GetGPUVirtualAddress(), sizeof(Vertex), numVertices);

		if (primitive.indices > 0) {
			const tinygltf::Accessor& indAcc = model.accessors[primitive.indices];
			if (indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_INT ||
				indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_SHORT ||
				indAcc.componentType == TINYGLTF_PARAMETER_TYPE_UNSIGNED_BYTE)
			{
				uint32_t indexElementSize = tinygltf::GetComponentSizeInBytes(indAcc.componentType);
				uint32_t numIndices = uint32_t(indAcc.count);
				uint8_t* indices = new uint8_t[numIndices * indexElementSize];
				LoadAccessorData(indices, indexElementSize, indexElementSize, numIndices, model, primitive.indices);

				if (numIndices > 0) {
					_indexBuffer = app->GetResourcesModule()->CreateDefaultBuffer(indices, numIndices * indexElementSize);
					_indexBufferView = CreateIndexBufferView(_indexBuffer->GetGPUVirtualAddress(), indexElementSize, numIndices);
				}
			}
		}
	}
}
