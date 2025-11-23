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
    window = new Window(_hwnd);
    debugDrawPass = std::make_unique<DebugDrawPass>(m_device.Get(), _commandQueue->GetD3D12CommandQueue().Get(), false);

    return true;
}

bool D3D12Module::postInit() {
    LoadAssets();
    return true;
}

void D3D12Module::preRender()
{
    //m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
    _commandQueue->WaitForFenceValue(m_fenceValues[m_frameIndex]);

    // Reset command list and allocator
    m_commandList = _commandQueue->GetCommandList();

    // Transition back buffer to render target
    TransitionResource(m_commandList, window->GetCurrentRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

    //CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
    D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = window->GetCurrentRenderTargetView();
    //CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
    D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = window->GetDepthStencilView();

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

    m_commandList->RSSetViewports(1, &window->GetViewport());
    m_commandList->RSSetScissorRects(1, &window->GetScissorRect());

    m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    //Assign composed MVP matrix
    Matrix mvp = (model * view * proj).Transpose();
    m_commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0);

    m_commandList->DrawInstanced(3, 1, 0, 0);

    dd::xzSquareGrid(-10.0f, 10.f, 0.0f, 1.0f, dd::colors::LightGray);
    dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
    debugDrawPass->record(m_commandList.Get(), window->Width(), window->Height(), view, proj);

}


void D3D12Module::render()
{
    // Indicate that the back buffer will now be used to present.
    TransitionResource(m_commandList, window->GetCurrentRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);

    // Execute the command list.
    m_fenceValues[m_frameIndex] = _commandQueue->ExecuteCommandList(m_commandList);

    // Present the frame and allow tearing
    window->Present();
}

void D3D12Module::postRender()
{
    // Move swap chain index to next frame
    // m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
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
        window->GetWindowSize(width, height);
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

        float aspect = float(window->Width()) / float(window->Height());
        float fov = XM_PIDIV4;

        proj = Matrix::CreatePerspectiveFieldOfView(fov, aspect, 1.0f, 1000.f);
    }
}



