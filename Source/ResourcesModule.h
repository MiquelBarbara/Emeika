#pragma once

#include "Module.h"

class ResourcesModule : public Module
{
public:
	void CreateUploadBuffer(ID3D12Device10* _device);
	void CreateDefaultBuffer(ID3D12Device10* _device, ID3D12GraphicsCommandList* _commandList);
private:
	UINT16 _bufferSize;

	ComPtr<ID3D12Resource> _buffer;
	ComPtr<ID3D12Resource> _vertexBuffer;
	ComPtr<ID3D12Resource> _stagingBuffer;
};