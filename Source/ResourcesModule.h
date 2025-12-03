#pragma once

#include "Module.h"
#include "CommandQueue.h"
#include <filesystem>

using namespace std::filesystem;

struct Vertex
{
	Vector3 position;
	Vector2 uv;
};

class ResourcesModule : public Module
{
public:
	bool init() override;
	bool postInit() override;

	ComPtr<ID3D12Resource> CreateUploadBuffer(size_t size);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* data, size_t size);
	ComPtr<ID3D12Resource> CreateDepthBuffer(float windowWidth, float windowHeight);
	ComPtr<ID3D12Resource> CreateTexture2DFromFile(const path& filePath);
	
private:
	ComPtr<ID3D12Device4> device;
	CommandQueue* queue;
};