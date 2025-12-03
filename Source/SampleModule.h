#pragma once
#include "Module.h"

class SampleModule: public Module
{
public: 

	enum Type
	{
		LINEAR_WRAP,
		POINT_WRAP,
		LINEAR_CLAMP,
		POINT_CLAMP,
		COUNT
	};

	bool init();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(Type type) {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(samplerHeap->GetCPUDescriptorHandleForHeapStart(), type, descriptorSize);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(Type type) {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(samplerHeap->GetGPUDescriptorHandleForHeapStart(), type, descriptorSize);
	}

	ID3D12DescriptorHeap* GetSamplerHeap() {
		return samplerHeap.Get();
	}
private:
	void CreateDefaultSamplers(ID3D12Device* device);
	ComPtr<ID3D12DescriptorHeap> samplerHeap;
	D3D12_CPU_DESCRIPTOR_HANDLE cpuStart = { 0 };
	D3D12_GPU_DESCRIPTOR_HANDLE gpuStart = { 0 };
	UINT descriptorSize = 0;
	UINT count = 0;
};

