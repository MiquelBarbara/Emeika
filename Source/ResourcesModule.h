#pragma once

#include "Module.h"


struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

class ResourcesModule : public Module
{
public:
	bool init() override;

	ComPtr<ID3D12Resource> CreateUploadBuffer(const void* data, size_t size);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* data, size_t size);
private:
	ComPtr<ID3D12CommandAllocator> _commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> _commandList;
};