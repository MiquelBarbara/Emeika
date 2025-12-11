#include "Globals.h"
#include "ResourcesModule.h"
#include "D3D12Module.h"
#include "Application.h"
#include "CommandQueue.h"
#include <DirectXTex.h>
#include <iostream>

bool ResourcesModule::init()
{
	_device = app->GetD3D12Module()->GetDevice();
	_queue = app->GetD3D12Module()->GetCommandQueue();
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
	_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));

	return buffer;
}

ComPtr<ID3D12Resource> ResourcesModule::CreateDefaultBuffer(const void* data, size_t size, const char* name)
{
	// --- CREATE THE FINAL GPU BUFFER (DEFAULT HEAP) ---
	ComPtr<ID3D12Resource> buffer;
	auto defaultHeap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	auto bufferDesc = CD3DX12_RESOURCE_DESC::Buffer(size);
	_device->CreateCommittedResource(&defaultHeap, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&buffer));
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

	ComPtr<ID3D12GraphicsCommandList4> _commandList = _queue->GetCommandList();

	_commandList->CopyResource(buffer.Get(), uploadBuffer.Get());

	_queue->ExecuteCommandList(_commandList);

	_queue->Flush();

	buffer->SetName(std::wstring(name, name + strlen(name)).c_str());
	return buffer;
}

DepthBuffer ResourcesModule::CreateDepthBuffer(float windowWidth, float windowHeight)
{
	_device = app->GetD3D12Module()->GetDevice();

	ComPtr<ID3D12Resource> resource;

	CD3DX12_HEAP_PROPERTIES heapProps = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
	CD3DX12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, windowWidth, windowHeight, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL);
	D3D12_CLEAR_VALUE clearValue = CD3DX12_CLEAR_VALUE(DXGI_FORMAT_D32_FLOAT, 1.0f, 0);

	_device->CreateCommittedResource(&heapProps, D3D12_HEAP_FLAG_NONE, &desc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &clearValue, IID_PPV_ARGS(&resource));

	resource->SetName(L"DepthBuffer");

	auto dsvHeap = app->GetDescriptorsModule()->GetDSV()->Allocate();
	_device->CreateDepthStencilView(resource.Get(), nullptr, dsvHeap.cpu);

	DepthBuffer depthBuffer = DepthBuffer();
	depthBuffer._texture._resource = resource;
	depthBuffer._dsv = dsvHeap;

	return depthBuffer;
}


Texture ResourcesModule::CreateTexture2DFromFile(const path& filePath, const char* name)
{
	Texture texture = Texture();
	ScratchImage image;
	const wchar_t* path = filePath.c_str();

	if (FAILED(LoadFromDDSFile(path, DDS_FLAGS_NONE, nullptr, image))) {
		if (FAILED(LoadFromTGAFile(path, nullptr, image))) {
			LoadFromWICFile(path, WIC_FLAGS_NONE, nullptr, image);
		}
	}

	if (image.GetImageCount() == 0) {
		return texture;
	}

	TexMetadata metaData = image.GetMetadata();
	if (metaData.dimension != TEX_DIMENSION_TEXTURE2D) {
		return texture;
	}

	if (metaData.mipLevels == 1 && (metaData.width > 1 || metaData.height > 1))
	{
		ScratchImage mipImages;
		if (FAILED(GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), TEX_FILTER_FANT | TEX_FILTER_SEPARATE_ALPHA, 0, mipImages))) {
			// Try Nvidia tool?
		}
		else {
			image = std::move(mipImages);
			metaData = image.GetMetadata();
		}
	}

	D3D12_RESOURCE_DESC desc = CD3DX12_RESOURCE_DESC::Tex2D(metaData.format, UINT64(metaData.width),UINT(metaData.height), UINT16(metaData.arraySize),UINT16(metaData.mipLevels));
	CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_DEFAULT);
	_device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,D3D12_RESOURCE_STATE_COPY_DEST, nullptr, IID_PPV_ARGS(&texture._resource));

	ComPtr<ID3D12Resource> stagingBuffer = CreateUploadBuffer(GetRequiredIntermediateSize(texture._resource.Get(), 0, image.GetImageCount()));

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
	ComPtr<ID3D12GraphicsCommandList4> commandList = _queue->GetCommandList();
	UpdateSubresources(commandList.Get(), texture._resource.Get(), stagingBuffer.Get(), 0, 0, UINT(image.GetImageCount()), subData.data());

	CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(texture._resource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	commandList->ResourceBarrier(1, &barrier);
	_queue->ExecuteCommandList(commandList);
	_queue->Flush();

	texture._srv = app->GetDescriptorsModule()->GetSRV()->Allocate();
	_device->CreateShaderResourceView(texture._resource.Get(), nullptr, texture._srv.cpu);

	texture._resource->SetName(std::wstring(name, name + strlen(name)).c_str());
	return texture;
}


