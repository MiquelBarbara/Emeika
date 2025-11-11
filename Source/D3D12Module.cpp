#include "Globals.h"
#include "D3D12Module.h"
#include "Application.h"
#include "ResourcesModule.h"
#include <d3dcompiler.h>

D3D12Module::D3D12Module(HWND hwnd) 
{
    _hwnd = hwnd;
}

bool D3D12Module::init()
{
    LoadPipeline();

    return true;
}

bool D3D12Module::postInit() {
    LoadAssets();
    return true;
}

void D3D12Module::preRender()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    WaitForFence();

    // Reset command list and allocator
    m_commandAllocators[m_frameIndex]->Reset();
    m_commandList->Reset(m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get());

    // Bind root signature (must be set before any draw calls)
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    // Setup viewport & scissor
    D3D12_VIEWPORT viewport = {};
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<float>(windowWidth);
    viewport.Height = static_cast<float>(windowHeight);
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;

    D3D12_RECT scissorRect = { 0, 0, (LONG)windowWidth, (LONG)windowHeight };

    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissorRect);

    // Transition back buffer to render target
    auto barrier = CD3DX12_RESOURCE_BARRIER::Transition(
        m_renderTargets[m_frameIndex].Get(),
        D3D12_RESOURCE_STATE_PRESENT,
        D3D12_RESOURCE_STATE_RENDER_TARGET
    );
    m_commandList->ResourceBarrier(1, &barrier);

    // Set render target
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart(),
        m_frameIndex,
        m_rtvDescriptorSize
    );
    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Clear + draw
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);

    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);
    m_commandList->DrawInstanced(3, 1, 0, 0);
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
    m_swapChain->Present(1, 0);
}

void D3D12Module::postRender()
{
    // Move swap chain index to next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    // Store the fence value associated with this back buffer index
    m_fenceValues[m_frameIndex] = Signal();
}

bool D3D12Module::cleanUp()
{
    WaitForFence();

    CloseHandle(m_fenceEvent);

    return true;
}


void D3D12Module::LoadPipeline() {

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

    CreateSwapChain();
    CreateDescriptorHeap();
    CreateRTVs();
    CreateCommandAllocators();
}

void D3D12Module::CreateRootSignature() {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    DXCall(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    DXCall(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void D3D12Module::CreatePipelineStateObject() {
    ComPtr<ID3DBlob> vertexShader;
    ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    DXCall(D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    DXCall(D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;


    DXCall(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void D3D12Module::CreateCommandList() {
    DXCall(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocators[m_frameIndex].Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
    m_commandList->Close();

    // Create synchronization objects and wait until assets have been uploaded to the GPU.
    m_fence = CreateFence(m_device);
}

ComPtr<ID3D12Fence> D3D12Module::CreateFence(ComPtr<ID3D12Device> device) {
    ComPtr<ID3D12Fence> fence;
    DXCall(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));

    // Create an event handle to use for frame synchronization.
    m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
    if (m_fenceEvent == nullptr)
    {
        DXCall(HRESULT_FROM_WIN32(GetLastError()));
    }

    return fence;
}

void D3D12Module::LoadAssets()
{
    // Create an empty root signature.
    CreateRootSignature();

    // Create the pipeline state, which includes compiling and loading shaders.
    CreatePipelineStateObject();

    // Create the command list.
    CreateCommandList();

    // Create the vertex buffer.
    {
        unsigned width, height;
        GetWindowSize(width, height);
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { 0.25f , 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.25f * aspectRatio, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { -0.25f * aspectRatio, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        buffer = app->getResourcesModule()->CreateDefaultBuffer(triangleVertices, vertexBufferSize);

        m_vertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }
}

void D3D12Module::GetWindowSize(unsigned& width, unsigned& height) {
    RECT rect = {};
    GetClientRect(_hwnd, &rect);

    width = unsigned(rect.right - rect.left);
    height = unsigned(rect.bottom - rect.top);
}

void D3D12Module::Flush() {
    // Signal a new fence value
    const UINT64 fenceToSignal = ++m_currentFenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToSignal);

    // Wait until GPU reaches it
    m_fence->SetEventOnCompletion(fenceToSignal, m_fenceEvent);
    WaitForSingleObject(m_fenceEvent, INFINITE);
}

void D3D12Module::Resize()
{
    unsigned width, height;
    GetWindowSize(width, height);

    if (width != windowWidth || height != windowHeight) {

        // Ensure GPU is finished with ALL pending work
        Flush();

        // Release the render targets
        for (UINT n = 0; n < FrameCount; n++)
        {
            m_renderTargets[n].Reset();
            m_fenceValues[n] = 0;
        }

        // Resize the swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        DXCall(m_swapChain->GetDesc(&swapChainDesc));

		DXCall(m_swapChain->ResizeBuffers(FrameCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

		m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Recreate the render target views
		CreateRTVs();

		windowWidth = width;
		windowHeight = height;
    }
}

void D3D12Module::CreateSwapChain() {

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

void D3D12Module::CreateDescriptorHeap() {

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FrameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    DXCall(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));

    m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void D3D12Module::CreateCommandAllocators()
{
    for (UINT n = 0; n < FrameCount; n++)
    {
        DXCall(m_device->CreateCommandAllocator(
            D3D12_COMMAND_LIST_TYPE_DIRECT,
            IID_PPV_ARGS(&m_commandAllocators[n])
        ));
    }
}

void D3D12Module::CreateRTVs()
{
    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(
        m_rtvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    for (UINT n = 0; n < FrameCount; n++)
    {
        DXCall(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));

        m_device->CreateRenderTargetView(
            m_renderTargets[n].Get(),
            nullptr,
            rtvHandle
        );

        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

bool D3D12Module::IsFenceComplete(UINT16 fenceValue) {
    return m_fence->GetCompletedValue() >= fenceValue;
}

// Wait for pending GPU work to complete.
void D3D12Module::WaitForFence() {
    const UINT64 fenceValue = m_fenceValues[m_frameIndex];
    if (!IsFenceComplete(fenceValue))
    {
        m_fence->SetEventOnCompletion(fenceValue, m_fenceEvent);
        WaitForSingleObject(m_fenceEvent, INFINITE);
    }
}

UINT64 D3D12Module::Signal() {
    const UINT64 fenceToSignal = ++m_currentFenceValue;
    m_commandQueue->Signal(m_fence.Get(), fenceToSignal);
    return fenceToSignal;
}



