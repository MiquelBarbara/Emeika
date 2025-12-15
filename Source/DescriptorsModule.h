#pragma once
#include "Module.h"
#include "DescriptorHeap.h"
#include "Resources.h"

// -----------------------------------------------------------------------------
// DescriptorsModule
// -----------------------------------------------------------------------------
// Acts as a centralized manager for all descriptor heaps used by the renderer.
//
// This module abstracts the creation and lifetime management of the different
// descriptor heap types required by Direct3D 12:
//
// • RTV(Render Target Views)
// • DSV(Depth Stencil Views)
// • SRV(Shader Resource Views, e.g.textures, buffers)
// • Samplers(Sampler descriptors for filtering / wrapping modes)
class DescriptorsModule: public Module
{
public:
	enum SampleType
	{
		LINEAR_WRAP,
		POINT_WRAP,
		LINEAR_CLAMP,
		POINT_CLAMP,
		COUNT
	};

	bool init() override;
	bool cleanUp() override;
	void CreateDefaultSamplers();



	constexpr DescriptorHeap* GetRTV() const { return _rtv; }
	constexpr DescriptorHeap* GetDSV() const { return _dsv; }
	constexpr DescriptorHeap* GetSRV() const { return _srv; }
	constexpr DescriptorHeap* GetSamplers() const { return _samplers; }

	constexpr DescriptorHeap* GetOffscreenRTV() const { return _offscreenRtv; }


private:
	DescriptorHeap* _rtv{};
	DescriptorHeap* _dsv{};
	DescriptorHeap* _srv{};
	DescriptorHeap* _samplers{};
	DescriptorHeap* _offscreenRtv{};

	ComPtr<ID3D12Device4> _device{};
};

