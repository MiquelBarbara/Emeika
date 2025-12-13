#pragma once
#include "DescriptorHeap.h"

struct TextureInitInfo {
	ID3D12Heap1* heap{ nullptr };
	ID3D12Resource* resource{ nullptr };
	D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc{};
	D3D12_RESOURCE_DESC* desc{};
	D3D12_RESOURCE_ALLOCATION_INFO1 allocInfo{};
	D3D12_RESOURCE_STATES initialState{ };
	D3D12_CLEAR_VALUE clearValue{  };
};

class Texture {
public:
	constexpr static uint32_t maxMips{ 14 };
	Texture() = default;
	explicit Texture(TextureInitInfo info);

	ID3D12Resource* GetResource() { return _resource.Get(); }
	DescriptorHandle SRV() { return _srv; }
private:
	ComPtr<ID3D12Resource> _resource{ nullptr };
	DescriptorHandle _srv{};
};

class RenderTexture {
public:
	RenderTexture() = default;
	explicit RenderTexture(TextureInitInfo info);

	uint32_t MipCount() const { return _mipCount; }
	D3D12_CPU_DESCRIPTOR_HANDLE RTV(uint32_t mipIndex) const { assert(mipIndex < _mipCount); return _rtv[mipIndex].cpu; }
	DescriptorHandle SRV() { return _texture.SRV(); }
	ID3D12Resource*  GetResource() { return _texture.GetResource(); }
private:
	Texture _texture;
	DescriptorHandle _rtv[Texture::maxMips]{};
	uint32_t _mipCount{ 0 };
};

class DepthBuffer {
public:
	DepthBuffer() = default;
	explicit DepthBuffer(TextureInitInfo info);

	D3D12_CPU_DESCRIPTOR_HANDLE DSV(){ return _dsv.cpu; }
	DescriptorHandle SRV() { return _texture.SRV(); }
	ID3D12Resource* GetResource() { return _texture.GetResource(); }
private:
	Texture _texture;
	DescriptorHandle _dsv{};
};

class RenderTarget {
public:
	ComPtr<ID3D12Resource> resource{ nullptr };
	DescriptorHandle rtv{};
};
