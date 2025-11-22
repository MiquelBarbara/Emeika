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
	ComPtr<ID3D12Resource> CreateDepthBuffer(float windowWidth, float windowHeight);
private:

};