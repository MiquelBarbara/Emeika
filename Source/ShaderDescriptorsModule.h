#pragma once
#include "Module.h"

class ShaderDescriptorsModule: public Module
{
public:
	bool init() override;
	bool postInit() override;
	UINT CreateSRV(ID3D12Resource* resource);
	void Allocate();
	void Reset();

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int index) {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(srvHeap->GetCPUDescriptorHandleForHeapStart(), index,
			srvDescriptorSize);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index) {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(srvHeap->GetGPUDescriptorHandleForHeapStart(), index,
			srvDescriptorSize);
	}
	ID3D12DescriptorHeap* GetSRVHeap() {
		return srvHeap.Get();
	}
private:
	ComPtr<ID3D12Device4> device;
	ComPtr<ID3D12DescriptorHeap> srvHeap;
	UINT srvDescriptorSize;
	uint32_t nextFreeDescriptorIndex = 0;
};

