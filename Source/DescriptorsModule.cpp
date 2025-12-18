#include "Globals.h"
#include "DescriptorsModule.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ResourcesModule.h"

DescriptorsModule::~DescriptorsModule()
{
    delete _offscreenRtv;
    _offscreenRtv = nullptr;
    delete _rtv;
    _rtv = nullptr;
    delete _dsv;
    _dsv = nullptr;
    delete _srv;
    _srv = nullptr;
    delete _samplers;
    _samplers = nullptr;
    _defferedDescriptors.clear();
}

bool DescriptorsModule::init()
{
    _device = app->GetD3D12Module()->GetDevice();

    _rtv = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256);
    _dsv = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 256);
    _srv = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 4096 * 8);
    _samplers = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, SampleType::COUNT);

    _offscreenRtv = new DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, 256);

    CreateDefaultSamplers();
	return true;
}

void DescriptorsModule::preRender()
{
    // For now only the SRV heap is having deferred releases since it's the only one used for textures
	UINT lastCompletedFrame = app->GetD3D12Module()->GetLastCompletedFrame();
	for (int i = 0; i < _defferedDescriptors.size(); ++i) {

        if (lastCompletedFrame > _defferedDescriptors[i].frame)
        {
            GetSRV()->Free(_defferedDescriptors[i].handle);
            _defferedDescriptors[i] = _defferedDescriptors.back();
            _defferedDescriptors.pop_back();
        }
        else
        {
            ++i;
        }
	}
}

bool DescriptorsModule::cleanUp()
{

    return true;
}

void DescriptorsModule::CreateDefaultSamplers()
{
    D3D12_SAMPLER_DESC samplers[COUNT] = {
        {
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            0.0f, 16, D3D12_COMPARISON_FUNC_NEVER,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            D3D12_TEXTURE_ADDRESS_MODE_WRAP,
            0, 16, D3D12_COMPARISON_FUNC_NEVER,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_LINEAR,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            0, 16, D3D12_COMPARISON_FUNC_NEVER,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        },

        {
            D3D12_FILTER_MIN_MAG_MIP_POINT,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            D3D12_TEXTURE_ADDRESS_MODE_CLAMP,
            0, 16, D3D12_COMPARISON_FUNC_NEVER,
            {0.0f, 0.0f, 0.0f, 0.0f},
            0.0f, D3D12_FLOAT32_MAX
        }
    };

    for (uint32_t i = 0; i < COUNT; ++i)
    {
        auto handle = _samplers->Allocate();
        _device->CreateSampler(&samplers[i], handle.cpu);
    }
}

void DescriptorsModule::DefferDescriptorRelease(Handle handle)
{
	DefferedDescriptor defferedDescriptor;
	defferedDescriptor.frame = app->GetD3D12Module()->GetCurrentFrame();
	defferedDescriptor.handle = handle;
	_defferedDescriptors.push_back(defferedDescriptor);
}

