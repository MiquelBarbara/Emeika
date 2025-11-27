#pragma once
#include "Transform.h"

class Model
{
public:
	Transform& GetTransform() { return _transform; }
	Matrix& GetWorldMatrix() { return _transform.GetWorldMatrix(); }
private:
	// In future both both Transform and Model should be components in an ECS pattern
	Transform _transform;

	//Mesh buffers
	ComPtr<ID3D12Resource> _vertexBuffer;
	ComPtr<ID3D12Resource> _indexBuffer;
	D3D12_VERTEX_BUFFER_VIEW _vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW _indexBufferView;
};

