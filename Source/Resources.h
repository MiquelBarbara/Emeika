#pragma once
#include "DescriptorHeap.h"


#ifndef DISABLE_COPY
#define DISABLE_COPY(T) \
	explicit T(const T&) = delete; \
	T& operator=(const T&) = delete; 
#endif

#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T) \
	explicit T(T&&) = delete; \
	T& operator=(T&&) = delete;
#endif

#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif

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

	~Texture() {
		Release();
	}
	void Release();
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
	

	~RenderTexture() {
		Release();
	}

	void Release();
	uint32_t MipCount() const { return _mipCount; }
	DescriptorHandle RTV(uint32_t mipIndex) const { assert(mipIndex < _mipCount); return _rtv[mipIndex]; }
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

	~DepthBuffer() {
		Release();
	}
	void Release();
	DescriptorHandle DSV(){ return _dsv; }
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
