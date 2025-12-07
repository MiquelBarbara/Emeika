#pragma once

//
// Represents a pair of CPU/GPU descriptor handles along with the descriptor index
// in the descriptor heap. This acts as a lightweight utility type returned by
// DescriptorHeap::Allocate().
//
struct DescriptorHandle {
	D3D12_CPU_DESCRIPTOR_HANDLE cpu{};
	D3D12_GPU_DESCRIPTOR_HANDLE gpu{};
	uint32_t index{ (uint32_t)-1 };

	constexpr bool IsValid() const {
		return cpu.ptr != 0;
	}

	constexpr bool IsShaderVisible() const {
		return gpu.ptr != 0;
	}
};

// -----------------------------------------------------------------------------
// DescriptorHeap
// -----------------------------------------------------------------------------
// A convenience wrapper around ID3D12DescriptorHeap that manages descriptor
// allocation in a simple linear fashion. This class is intended to handle both
// CPU-only and shader-visible descriptor heaps.
//
// Notes:
// - DescriptorHandle may evolve into a more robust "DescriptorAllocation" class
//   similar to what is described in: 
//   https://www.3dgep.com/learning-directx-12-3/#DescriptorAllocation_Class
//
class DescriptorHeap
{
public:
	DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE type, const uint32_t numDescriptors);
	DescriptorHandle Allocate();
	void Reset();

	ID3D12DescriptorHeap* GetHeap() { return _heap.Get(); }

	D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHandle(int index) {
		return CD3DX12_CPU_DESCRIPTOR_HANDLE(_heap->GetCPUDescriptorHandleForHeapStart(), index, _descriptorSize);
	}
	D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHandle(int index) {
		return CD3DX12_GPU_DESCRIPTOR_HANDLE(_heap->GetGPUDescriptorHandleForHeapStart(), index, _descriptorSize);
	}

private:
	ComPtr<ID3D12DescriptorHeap> _heap;
	D3D12_CPU_DESCRIPTOR_HANDLE _cpuStart{};
	D3D12_GPU_DESCRIPTOR_HANDLE _gpuStart{};
	uint32_t _descriptorSize;
	uint32_t _numDescriptors;
	uint32_t _nextFreeIndex = 0;

	const D3D12_DESCRIPTOR_HEAP_TYPE _type{};
};

