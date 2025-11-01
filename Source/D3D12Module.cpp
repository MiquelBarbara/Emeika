#include "Globals.h"
#include "D3D12Module.h"
#include "Application.h"
#include <d3d12.h>

D3D12Module::D3D12Module(HWND hwnd) 
{
    _hwnd = hwnd;
}



bool D3D12Module::init()
{
    loadPipeline();
    return true;
}

void D3D12Module::loadPipeline() {

    UINT dxgiFactoryFlags = 0;

#if defined(_DEBUG)
    {
        ComPtr<ID3D12Debug> debugController;
        if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController))))
        {
            debugController->EnableDebugLayer();
        }
    }
#endif

#if defined(_DEBUG)
    CreateDXGIFactory2(DXGI_CREATE_FACTORY_DEBUG, IID_PPV_ARGS(&m_dxgiFactory));
#else
    CreateDXGIFactory2(0, IID_PPV_ARGS(&m_dxgiFactory));
#endif
    ComPtr<IDXGIAdapter1> adapter;
    DXCall(m_dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&adapter)));
    D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

#ifdef  _DEBUG
    //Enable debugging layer. Requires Graphics Tools optional feature in Windows 10 SDK
    {
        ComPtr<ID3D12InfoQueue> info_queue;
        DXCall(m_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
    }
#endif

    // Describe and create the command queue.
    D3D12_COMMAND_QUEUE_DESC queueDesc{};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    DXCall(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

    createSwapChain();
    createDescriptorHeap();
    createDescriptorHandle();

    // Create the command list.
    DXCall(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
    DXCall(m_commandList->Close());

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    {
        DXCall(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));

        // Create an event handle to use for frame synchronization.
        m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
        if (m_fenceEvent == nullptr)
        {
            DXCall(HRESULT_FROM_WIN32(GetLastError()));
        }
    }
}

void D3D12Module::getWindowSize(unsigned& width, unsigned& height) {
    RECT rect = {};
    GetClientRect(_hwnd, &rect);

    width = unsigned(rect.right - rect.left);
    height = unsigned(rect.bottom - rect.top);
}

void D3D12Module::flush() {
    // Signal a new fence value
    const UINT64 fenceToSignal = ++m_currentFenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToSignal);

    // Wait until GPU reaches it
    m_fence->SetEventOnCompletion(fenceToSignal, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

void D3D12Module::resize()
{
    unsigned width, height;
    getWindowSize(width, height);

    if (width != windowWidth || height != windowHeight) {
        // Wait until all previous GPU work is complete.
        //waitForFence(m_fenceValues[m_frameIndex]);
        
        flush();

        // Release the render targets
        for (UINT n = 0; n < FrameCount; n++)
        {
            m_renderTargets[n].Reset();
        }

        // Resize the swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        DXCall(m_swapChain->GetDesc(&swapChainDesc));

		DXCall(m_swapChain->ResizeBuffers(FrameCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Recreate the render target views
		createDescriptorHandle();

		windowWidth = width;
		windowHeight = height;
    }
}

void D3D12Module::createSwapChain() {

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = windowWidth; 
    swapChainDesc.Height = windowHeight;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA format (8 bits per channel)
    // UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0)
    swapChainDesc.Stereo = FALSE; // Set to TRUE for stereoscopic 3D rendering (VR/3D Vision)
    swapChainDesc.SampleDesc = { 1, 0 }; // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel)
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // This buffer will be used as a render target
    swapChainDesc.BufferCount = FrameCount; // Double buffering:
    // - 1 front buffer (displayed)
   // - 1 back buffer (rendering)
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // How to scale when window size doesn't match buffer size:
    // STRETCH = Stretch the image to fit the window
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern efficient swap method:
    // - FLIP: Uses page flipping (no copying)
   // - DISCARD: Discard previous back buffer contents
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
    swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING; // Additional swap chain options: 0 = No special flags
    // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Allow full-screen mode switches
   //DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING: Allow tearing in windowed mode (VSync off)

    ComPtr<IDXGISwapChain1> swapChain;

    assert(m_dxgiFactory != nullptr);

    DXCall(m_dxgiFactory->CreateSwapChainForHwnd(
        m_commandQueue.Get(),
        _hwnd,
        &swapChainDesc,
        nullptr, // fullscreen desc
        nullptr, // restrict output
        &swapChain
    ));

    swapChain.As(&m_swapChain);
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3D12Module::createDescriptorHeap() {

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DXCall(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void D3D12Module::createDescriptorHandle() {
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV and a command allocator for each frame.
    for (UINT n = 0; n < FrameCount; n++)
    {
        DXCall(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
        m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
        rtvHandle.Offset(1, m_rtvDescriptorSize);

        DXCall(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocators[n])));
    }
}

// Wait for pending GPU work to complete.
void D3D12Module::waitForFence() {

    // If the GPU has not finished processing the commands of the current frame
    if (m_fence->GetCompletedValue() < m_fenceValues[m_frameIndex])
    {
        m_fence->SetEventOnCompletion(m_fenceValues[m_frameIndex], m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

void D3D12Module::preRender()
{
    waitForFence();
    // Reset command list and allocator
    m_commandAllocators[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), nullptr);

    // Indicate that the back buffer will be used as a render target.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET);

    m_commandList->ResourceBarrier(1, &barrier);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);
}

void D3D12Module::render()
{

    // Indicate that the back buffer will now be used to present.
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_RENDER_TARGET,
        D3D12_RESOURCE_STATE_PRESENT);

    m_commandList->ResourceBarrier(1, &barrier);

    // Close the command list.
    m_commandList->Close();

    // Execute the command list.
    ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
    m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

    // Present the frame.
    m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}

void D3D12Module::postRender()
{
    // Advance fence value for this frame
    const UINT64 fenceToSignal = ++m_currentFenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToSignal);

    // Store the fence value associated with this back buffer index
    m_fenceValues[m_frameIndex] = fenceToSignal;

    // Move swap chain index to next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

bool D3D12Module::cleanUp()
{
    waitForFence();

    CloseHandle(m_fenceEvent);

    return true;
}
