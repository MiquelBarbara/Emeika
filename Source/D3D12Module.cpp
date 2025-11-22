#include "Globals.h"
#include "D3D12Module.h"
#include "Application.h"
#include "ResourcesModule.h"
#include <d3dcompiler.h>

D3D12Module::D3D12Module(HWND hwnd) 
{
    _hwnd = hwnd;
    GetWindowSize(windowWidth, windowHeight);
}

bool D3D12Module::init()
{
    LoadPipeline();
    debugDrawPass = std::make_unique<DebugDrawPass>(m_device.Get(), _commandQueue->GetD3D12CommandQueue().Get(), false);

    return true;
}

bool D3D12Module::postInit() {
    LoadAssets();
    return true;
}

void D3D12Module::preRender()
{
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    _commandQueue->WaitForFenceValue(m_fenceValues[m_frameIndex]);

    // Reset command list and allocator
    m_commandList = _commandQueue->GetCommandList();

    // Transition back buffer to render target
    TransitionResource(m_commandList, m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

    // Clear + draw
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0,0, nullptr);

    // Bind root signature (must be set before any draw calls)
    m_commandList->SetPipelineState(m_pipelineState.Get());
    m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    //Set input assembler
    m_commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    m_commandList->IASetVertexBuffers(0, 1, &m_vertexBufferView);

    // Setup viewport & scissor
    D3D12_VIEWPORT viewport{ 0.0, 0.0, float(windowWidth), float(windowHeight) , 0.0, 1.0 };
    D3D12_RECT scissor{ 0, 0, windowWidth, windowHeight };
    m_commandList->RSSetViewports(1, &viewport);
    m_commandList->RSSetScissorRects(1, &scissor);

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    //Assign composed MVP matrix
    Matrix mvp = (model * view * proj).Transpose();
    m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0);

    m_commandList->DrawInstanced(3, 1, 0, 0);

    dd::xzSquareGrid(-10.0f, 10.f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
    debugDrawPass->record(m_commandList.Get(), viewport.Width, viewport.Height, view, proj);

}


void D3D12Module::render()
{
    // Indicate that the back buffer will now be used to present.
    TransitionResource(m_commandList, m_renderTargets[m_frameIndex], D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Execute the command list.
    m_fenceValues[m_frameIndex] = _commandQueue->ExecuteCommandList(m_commandList);

    // Present the frame and allow tearing
    m_swapChain->Present(0, DXGI_PRESENT_ALLOW_TEARING);
}

void D3D12Module::postRender()
{
    // Move swap chain index to next frame
    m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    // Store the fence value associated with this back buffer index
    m_fenceValues[m_frameIndex] = _commandQueue->Signal();
}

bool D3D12Module::cleanUp()
{
    _commandQueue->WaitForFenceValue(m_fenceValues[m_frameIndex]);

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
    _commandQueue = std::make_unique<CommandQueue>(m_device, D3D12_COMMAND_LIST_TYPE_DIRECT);
    m_swapChain = CreateSwapChain(_hwnd, _commandQueue->GetD3D12CommandQueue(), windowWidth, windowHeight, FrameCount);
    m_rtvHeap = CreateDescriptorHeap(m_device,D3D12_DESCRIPTOR_HEAP_TYPE_RTV, FrameCount);
    m_dsvHeap = CreateDescriptorHeap(m_device, D3D12_DESCRIPTOR_HEAP_TYPE_DSV, 1);
    UpdateRenderTargetViews(m_device, m_swapChain, m_rtvHeap);
    UpdateDepthView(m_device, m_dsvHeap);
}

void D3D12Module::TransitionResource(ComPtr<ID3D12GraphicsCommandList> commandList,ComPtr<ID3D12Resource> resource,D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState)
{
    CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(resource.Get(),beforeState, afterState);
    commandList->ResourceBarrier(1, &barrier);
}

void D3D12Module::CreateRootSignature() {
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    CD3DX12_ROOT_PARAMETER rootParameters;
    rootParameters.InitAsConstants(sizeof(Matrix) / sizeof(UINT32), 0); //number of 32 bit elements in a matrix
    rootSignatureDesc.Init(1, &rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    DXCall(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    DXCall(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}

void D3D12Module::CreatePipelineStateObject() {
    //ComPtr<ID3DBlob> vertexShader;
    //ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    // Load the vertex shader.
    ComPtr<ID3DBlob> vertexShaderBlob;
    DXCall(D3DReadFileToBlob(L"VertexShader.cso", &vertexShaderBlob));

    // Load the pixel shader.
    ComPtr<ID3DBlob> pixelShaderBlob;
    DXCall(D3DReadFileToBlob(L"PixelShader.cso", &pixelShaderBlob));


    //DXCall(D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    //DXCall(D3DCompileFromFile(L"shaders.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

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
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
    psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc = { 1,0 };

    DXCall(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}



void D3D12Module::LoadAssets()
{
    // Create an empty root signature.
    CreateRootSignature();

    // Create the pipeline state, which includes compiling and loading shaders.
    CreatePipelineStateObject();

    // Create the vertex buffer.
    {
        unsigned width, height;
        GetWindowSize(width, height);
        float aspectRatio = static_cast<float>(width) / static_cast<float>(height);

        // Define the geometry for a triangle.
        Vertex triangleVertices[] =
        {
            { { -1.0f , -1.0f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
            { { 0.0f, 1.0f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
            { { 1.0f, -1.0f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
        };

        const UINT vertexBufferSize = sizeof(triangleVertices);

        buffer = app->getResourcesModule()->CreateDefaultBuffer(triangleVertices, vertexBufferSize);

        m_vertexBufferView.BufferLocation = buffer->GetGPUVirtualAddress();
        m_vertexBufferView.StrideInBytes = sizeof(Vertex);
        m_vertexBufferView.SizeInBytes = vertexBufferSize;
    }

    //Create the MVP
    {

        model = Matrix::Identity;
        view = Matrix::CreateLookAt(Vector3(0.0f, 10.f, 10.0f), Vector3::Zero, Vector3::Up);

        float aspect = float(windowWidth) / float(windowHeight);
        float fov = XM_PIDIV4;

        proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 1.0f, 1000.f);
    }
}

void D3D12Module::GetWindowSize(unsigned& width, unsigned& height) {
    RECT rect = {};
    GetClientRect(_hwnd, &rect);

    width = unsigned(rect.right - rect.left);
    height = unsigned(rect.bottom - rect.top);
}


void D3D12Module::Resize()
{
    unsigned width, height;
    GetWindowSize(width, height);

    if (width != windowWidth || height != windowHeight) {

        windowWidth = width;
        windowHeight = height;
        // Ensure GPU is finished with ALL pending work
        _commandQueue->Flush();

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
        UpdateRenderTargetViews(m_device, m_swapChain, m_rtvHeap);
        UpdateDepthView(m_device, m_dsvHeap);
    }
}

ComPtr<IDXGISwapChain4> D3D12Module::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount) {

    ComPtr<IDXGISwapChain4> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    DXCall(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 32-bit RGBA format (8 bits per channel)
    // UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0)
    swapChainDesc.Stereo = FALSE; // Set to TRUE for stereoscopic 3D rendering (VR/3D Vision)
    swapChainDesc.SampleDesc = { 1, 0 }; // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel)
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // This buffer will be used as a render target
    swapChainDesc.BufferCount = bufferCount;
    // - 1 front buffer (displayed)
   // - 1 back buffer (rendering)
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // How to scale when window size doesn't match buffer size:
    // STRETCH = Stretch the image to fit the window
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern efficient swap method:
    // - FLIP: Uses page flipping (no copying)
   // - DISCARD: Discard previous back buffer contents
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
    swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Allow full-screen mode switches
   //DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING: Allow tearing in windowed mode (VSync off)

    ComPtr<IDXGISwapChain1> swapChain1;

    DXCall(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr, // fullscreen desc
        nullptr, // restrict output
        &swapChain1
    ));

    swapChain1.As(&dxgiSwapChain4);

    return dxgiSwapChain4;
}

ComPtr<ID3D12DescriptorHeap> D3D12Module::CreateDescriptorHeap(ComPtr<ID3D12Device2> device,D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors) {
    ComPtr<ID3D12DescriptorHeap> descriptorHeap;

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = numDescriptors;
    rtvHeapDesc.Type = type;
    DXCall(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&descriptorHeap)));

    return descriptorHeap;
}

void D3D12Module::UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    m_rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

   CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(descriptorHeap->GetCPUDescriptorHandleForHeapStart());

    for (UINT n = 0; n < FrameCount; n++)
    {
        DXCall(swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));

        device->CreateRenderTargetView(m_renderTargets[n].Get(),nullptr, rtvHandle);

        rtvHandle.Offset(1, m_rtvDescriptorSize);
    }
}

void D3D12Module::UpdateDepthView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap)
{
    //Resize screen dependt resources
    //Create a depth buffer
    depthBuffer = app->getResourcesModule()->CreateDepthBuffer(windowWidth, windowHeight);
    //Update the depth-stencil
    device->CreateDepthStencilView(depthBuffer.Get(), nullptr, descriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

bool D3D12Module::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}



