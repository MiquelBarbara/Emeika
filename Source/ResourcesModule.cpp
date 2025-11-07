#include "ResourcesModule.h"

void ResourcesModule::CreateUploadBuffer(ID3D12Device10* _device)
{
	// 1. Describe the buffer
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(_bufferSize);
	// 2. Specify UPLOAD heap properties
	CD3DX12_HEAP_PROPERTIES heapProps(D3D12_HEAP_TYPE_UPLOAD);
	// 3. Create the resource
	_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&_buffer));
}

void ResourcesModule::CreateDefaultBuffer(ID3D12Device10* _device, ID3D12GraphicsCommandList* _commandList)
{
	// --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(_bufferSize);
	_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(& _vertexBuffer));
	// --- CREATE THE STAGING BUFFER (UPLOAD HEAP) ---
	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&_stagingBuffer));
	// --- CPU: FILL STAGING BUFFER ---
	// Map the buffer: get a CPU pointer to its memory
	/*BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
	_buffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	// Copy our application data into the GPU buffer
	memcpy(pData, cpuData, dataSize);
	// Unmap the buffer (invalidate the pointer)
	_buffer->Unmap(0, nullptr);*/

	// --- GPU: COPY DATA ---
	_commandList->CopyResource(_vertexBuffer.Get(), _stagingBuffer.Get());
}
