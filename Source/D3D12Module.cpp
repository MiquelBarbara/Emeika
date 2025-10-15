#include "Globals.h"
#include "D3D12Module.h"
#include <dxgi1_5.h>

bool D3D12Module::init()
{
#if defined(_DEBUG)
    D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface));
    debugInterface->EnableDebugLayer();
#endif

#if defined(_DEBUG)
    CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&factory));
#else
    CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
#endif
    factory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter));
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&device));

    device.As(&infoQueue);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
    infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);

    return false;
}
