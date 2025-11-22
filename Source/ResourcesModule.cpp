#include "Globals.h"
#include "ResourcesModule.h"
#include "D3D12Module.h"
#include "Application.h"
#include "CommandQueue.h"


bool ResourcesModule::init()
{
	return true;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateUploadBuffer(const void* data, size_t size )
{
	ComPtr<ID3D12Resource> buffer;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	app->getD3D12Module()->GetDevice()->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	return buffer;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateDefaultBuffer(const void* data, size_t size)
{
	ID3D12Device* device = app->getD3D12Module()->GetDevice();
	CommandQueue* queue = app->getD3D12Module()->GetCommandQueue();

	// --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
	ComPtr<ID3D12Resource> buffer;
	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
	// --- CREATE THE STAGING BUFFER (UPLOAD HEAP) ---
	ComPtr<ID3D12Resource> uploadBuffer;
	auto uploadHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	device->CreateCommittedResource(&uploadHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&uploadBuffer));

	// --- CPU: FILL STAGING BUFFER ---
	// Map the buffer: get a CPU pointer to its memory
	BYTE* pData = nullptr;
	CD3DX12_RANGE readRange(0, 0); // We won't read from it, so range is (0,0)
	uploadBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pData));
	// Copy our application data into the GPU buffer
	memcpy(pData, data, size);
	// Unmap the buffer (invalidate the pointer)
	uploadBuffer->Unmap(0, nullptr);
	
	// Copy buffer commands

	ComPtr<ID3D12GraphicsCommandList4> _commandList = queue->GetCommandList();

	_commandList->CopyResource(buffer.Get(), uploadBuffer.Get());

	queue->ExecuteCommandList(_commandList);

	queue->Flush();

	return buffer;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateDepthBuffer(float windowWidth, float windowHeight)
{
	ID3D12Device* device = app->getD3D12Module()->GetDevice();
	ComPtr<ID3D12Resource> depthBuffer;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);

	D3D12_CLEAR_VALUE clearValue = {};
	clearValue.Format = DXGI_FORMAT_D32_FLOAT;
	clearValue.DepthStencil.Depth = 1.0f;
	clearValue.DepthStencil.Stencil = 0;

	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthBuffer));

	return depthBuffer;
}
