#include "Globals.h"
#include "SampleModule.h"
#include "Application.h"
#include "D3D12Module.h"

bool SampleModule::init()
{
	auto device = app->getD3D12Module()->GetDevice();
	D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
	heapDesc.NumDescriptors = COUNT;
	heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER;
	heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&samplerHeap)); // Create Sampler Descriptor Heap
	CreateDefaultSamplers(device); // Create default samplers (e.g., Linear Wrap, Point Clamp)
	return true;
}

void SampleModule::CreateDefaultSamplers(ID3D12Device* device)
{
    cpuStart = samplerHeap->GetCPUDescriptorHandleForHeapStart();
    gpuStart = samplerHeap->GetGPUDescriptorHandleForHeapStart();

    D3D12_SAMPLER_DESC samplers[COUNT] = {
        {
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            0.0f, 16, D3D12_COMPARISON_FUNC_NONE,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            0, 16, D3D12_COMPARISON_FUNC_NONE,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            0, 16, D3D12_COMPARISON_FUNC_NONE,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            0, 16, D3D12_COMPARISON_FUNC_NONE,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        }
    };

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        device->CreateSampler(&samplers[i], CD3DX12_CPU_DESCRIPTOR_HANDLE(cpuStart, i, descriptorSize));
    }
}
