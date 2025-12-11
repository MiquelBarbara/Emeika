#pragma once

#include "Module.h"
#include "CommandQueue.h"
#include <filesystem>
#include "Resources.h"
#include "Application.h"
#include "DescriptorsModule.h"

using namespace std::filesystem;

struct Vertex
{
	Vector3 position;
	Vector2 texCoord0;
};

// -----------------------------------------------------------------------------
// ResourcesModule
// -----------------------------------------------------------------------------
// This module centralizes all resource creation
// 
class ResourcesModule : public Module
{
public:
	bool init() override;
	bool postInit() override;

	ComPtr<ID3D12Resource> CreateUploadBuffer(size_t size);
	ComPtr<ID3D12Resource> CreateDefaultBuffer(const void* data, size_t size, const char* name);
	DepthBuffer CreateDepthBuffer(float windowWidth, float windowHeight);
	Texture CreateTexture2DFromFile(const path& filePath, const char* name);
	
private:
	ComPtr<ID3D12Device4> _device;
	CommandQueue* _queue;
};