#pragma once
#include "DescriptorHeap.h"

class Texture {
public:
	constexpr static uint32_t maxMips{ 14 };
	ComPtr<ID3D12Resource> _resource{ nullptr };
	DescriptorHandle _srv{};
};

class RenderTexture {
	Texture _texture;
	DescriptorHandle _rtv[Texture::maxMips]{};
};

class DepthBuffer {
public:
	Texture _texture;
	DescriptorHandle _dsv{};
};

struct RenderTarget {
	ComPtr<ID3D12Resource> resource{ nullptr };
	DescriptorHandle rtv{};
};
