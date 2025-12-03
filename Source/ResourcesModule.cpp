#include "Globals.h"
#include "ResourcesModule.h"
#include "D3D12Module.h"
#include "Application.h"
#include "CommandQueue.h"
#include <DirectXTex.h>
#include <iostream>



bool ResourcesModule::init()
{
	device = app->getD3D12Module()->GetDevice();
	queue = app->getD3D12Module()->GetCommandQueue();
	return true;
}

bool ResourcesModule::postInit()
{

	return true;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateUploadBuffer(size_t size )
{
	ComPtr<ID3D12Resource> buffer;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Buffer(size);
	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	return buffer;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateDefaultBuffer(const void* data, size_t size)
{
	// --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
	ComPtr<ID3D12Resource> buffer;
	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
	// --- CREATE THE STAGING BUFFER (UPLOAD HEAP) ---
	ComPtr<ID3D12Resource> uploadBuffer = CreateUploadBuffer(size);

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
	if (!device) {
		device = app->getD3D12Module()->GetDevice();
	}

	ComPtr<ID3D12Resource> depthBuffer;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&depthBuffer));

	return depthBuffer;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateTexture2DFromFile(const path& filePath)
{
	ScratchImage image;
	const wchar_t* path = filePath.c_str();

	if (FAILED(LoadFromDDSFile(path, DDS_FLAGS_NONE, nullptr, image))) {
		if (FAILED(LoadFromTGAFile(path, nullptr, image))) {
			LoadFromWICFile(path, WIC_FLAGS_NONE, nullptr, image);
		}
	}

	if (image.GetImageCount() == 0) {
		return nullptr;
	}

	TexMetadata metaData = image.GetMetadata();
	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(metaData.format, UINT64(metaData.width),UINT(metaData.height), UINT16(metaData.arraySize),UINT16(metaData.mipLevels));

	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
	ComPtr<ID3D12Resource> texture;
	device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture));

	UINT64 size = GetRequiredIntermediateSize(texture.Get(), 0, image.GetImageCount());
	ComPtr<ID3D12Resource> stagingBuffer = CreateUploadBuffer(size);

	std::vector<D3D12_SUBRESOURCE_DATA> subData;
	subData.reserve(image.GetImageCount());
	// Note we are iteration over mipLevels of each array item to respect Subresource index order
	for (size_t item = 0; item < metaData.arraySize; ++item)
	{
		for (size_t level = 0; level < metaData.mipLevels; ++level)
		{
			const Image* subImg = image.GetImage(level, item, 0);
			D3D12_SUBRESOURCE_DATA data = { subImg->pixels, subImg->rowPitch, subImg->slicePitch };
			subData.push_back(data);
		}
	}
	ComPtr<ID3D12GraphicsCommandList4> _commandList = queue->GetCommandList();
	UpdateSubresources(_commandList.Get(), texture.Get(), stagingBuffer.Get(), 0, 0, UINT(image.GetImageCount()), subData.data());

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	_commandList->ResourceBarrier(1, &barrier);
	queue->ExecuteCommandList(_commandList);
	queue->Flush();

	return texture;
}


