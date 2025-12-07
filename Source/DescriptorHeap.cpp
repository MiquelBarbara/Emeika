#include "Globals.h"
#include "DescriptorHeap.h"
#include "Application.h"
#include "D3D12Module.h"

DescriptorHeap::DescriptorHeap(const D3D12_DESCRIPTOR_HEAP_TYPE type, const uint32_t numDescriptors): _type(type), _numDescriptors(numDescriptors)
{
	auto device = app->GetD3D12Module()->GetDevice();
	bool isShaderVisible = false;
	if (type == D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV || type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER) {
		isShaderVisible = true;
	}
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = numDescriptors;
	heapDesc.Type = type;
	heapDesc.Flags = isShaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&_heap));

	_descriptorSize = device->GetDescriptorHandleIncrementSize(_type);
	_cpuStart = _heap->GetCPUDescriptorHandleForHeapStart();
	_gpuStart = isShaderVisible ? _heap->GetGPUDescriptorHandleForHeapStart() : D3D12_GPU_DESCRIPTOR_HANDLE{ 0 };
}


DescriptorHandle DescriptorHeap::Allocate()
{
	if (_nextFreeIndex == _numDescriptors) {
		Reset();
	}

	DescriptorHandle handle{};
	const uint32_t offset{ _nextFreeIndex * _descriptorSize };
	handle.cpu.ptr = _cpuStart.ptr + offset;
	handle.gpu.ptr = _gpuStart.ptr + offset;
	handle.index = _nextFreeIndex;

	++_nextFreeIndex;

	return handle;
}

void DescriptorHeap::Reset()
{
	_nextFreeIndex = 0;
}
