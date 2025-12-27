#include "Globals.h"
#include "D3D12Module.h"
#include "Application.h"
#include "ResourcesModule.h"
#include "CameraModule.h"
#include "DescriptorsModule.h"
#include "EditorModule.h"
#include "Transform.h"
#include <d3dcompiler.h>
#include "RingBuffer.h"

D3D12Module::D3D12Module(HWND hwnd) 
{
    _hwnd = hwnd;
}

D3D12Module::~D3D12Module()
{

}


bool D3D12Module::init()
{
    LoadPipeline();

    return true;
}

bool D3D12Module::postInit() {
    _swapChain = new SwapChain(_hwnd);
    debugDrawPass = std::make_unique<DebugDrawPass>(m_device.Get(), _commandQueue->GetD3D12CommandQueue().Get(), false);
    
    offscreenRenderTarget = app->GetResourcesModule()->CreateRenderTexture(offscreenTextureSize.x, offscreenTextureSize.y);
    offscreenDepthBuffer = app->GetResourcesModule()->CreateDepthBuffer(offscreenTextureSize.x, offscreenTextureSize.y);

    app->GetCameraModule()->SetAspectRatio(static_cast<float>(offscreenTextureSize.x), static_cast<float>(offscreenTextureSize.y));
    LoadAssets();

    ringBuffer = new RingBuffer(m_device.Get(), 10);

    return true;
}

void D3D12Module::preRender()
{
    m_frameIndex = _swapChain->GetCurrentBackBufferIndex();
    _commandQueue->WaitForFenceValue(m_fenceValues[m_frameIndex]);
    m_lastCompletedFenceValue = std::max(m_lastCompletedFenceValue, m_fenceValues[m_frameIndex]);

    ringBuffer->Free(m_lastCompletedFenceValue);

    // Reset command list and allocator
    m_commandList = _commandQueue->GetCommandList();

    // TODO: Decouple EditorModule to D3D12Module
    auto newSize = app->GetEditorModule()->GetSceneEditorSize();
    if (offscreenTextureSize.x != newSize.x || offscreenTextureSize.y != newSize.y) {
        _commandQueue->Flush();
        offscreenTextureSize = newSize;
        offscreenRenderTarget = app->GetResourcesModule()->CreateRenderTexture(newSize.x, newSize.y);
        offscreenDepthBuffer = app->GetResourcesModule()->CreateDepthBuffer(newSize.x, newSize.y);
        app->GetCameraModule()->Resize(newSize.x, newSize.y);
    }

    if (offscreenRenderTarget->GetResource()) {
        // Transition scene texture to render target
        TransitionResource(m_commandList, offscreenRenderTarget->GetResource(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE, D3D12_RESOURCE_STATE_RENDER_TARGET);

        // Render the scene to texture
        RenderScene(m_commandList.Get(), offscreenRenderTarget->RTV(0).cpu , offscreenDepthBuffer->DSV().cpu, offscreenTextureSize.x, offscreenTextureSize.y);

        // Transition back to shader resource state
        TransitionResource(m_commandList, offscreenRenderTarget->GetResource(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
    }

    TransitionResource(m_commandList, _swapChain->GetCurrentRenderTarget(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);
    RenderBackground(m_commandList.Get(), _swapChain->GetCurrentRenderTargetView().cpu, _swapChain->GetDepthStencilView(), _swapChain->GetViewport().Width, _swapChain->GetViewport().Height);
}

void D3D12Module::render()
{
    // Indicate that the back buffer will now be used to present.
    TransitionResource(m_commandList, _swapChain->GetCurrentRenderTarget(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
    // Execute the command list.
    m_fenceValues[m_frameIndex] = _commandQueue->ExecuteCommandList(m_commandList);
    // Present the frame and allow tearing
    _swapChain->Present();
}

void D3D12Module::postRender()
{

}

bool D3D12Module::cleanUp()
{
    for (int i = 0; i < _models.size(); i++) {
        _models[i]->~Model();
    }

    offscreenRenderTarget.reset();
    offscreenDepthBuffer.reset();


    // 2. Destroy debug pass
    debugDrawPass.reset();

    if (_swapChain) {
        _swapChain->~SwapChain();
        delete _swapChain;
        _swapChain = nullptr;
    }

    // 4. Reset pipeline / root signature
    m_pipelineState.Reset();
    m_rootSignature.Reset();

    // 5. Reset command list
    m_commandList.Reset();

    // 7. Reset command queue
    _commandQueue.reset();

    // 8. Reset device
    m_device.Reset();

    // 9. Reset DXGI factory
    m_dxgiFactory.Reset();

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
    DXCall(m_dxgiFactory->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(&m_adapter)));
    D3D12CreateDevice(m_adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device));

#ifdef  _DEBUG
    //Enable debugging layer. Requires Graphics Tools optional feature in Windows 10 SDK
    {
        ComPtr<ID3D12InfoQueue> info_queue;
        DXCall(m_device->QueryInterface(IID_PPV_ARGS(&info_queue)));

        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
        info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
        //info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, TRUE);
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
    CD3DX12_ROOT_PARAMETER rootParameters[5] = {};
    CD3DX12_DESCRIPTOR_RANGE srvRange, sampRange;

    srvRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0, 0); // 1 range of 1 SRV descriptor
    sampRange.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SAMPLER, DescriptorsModule::SampleType::COUNT, 0);

    rootParameters[0].InitAsConstants((sizeof(Matrix) / sizeof(UINT32)), 0, 0, D3D12_SHADER_VISIBILITY_VERTEX);
    rootParameters[1].InitAsConstantBufferView(1, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[2].InitAsConstantBufferView(2, 0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[3].InitAsDescriptorTable(1, &srvRange, D3D12_SHADER_VISIBILITY_PIXEL); // The descriptor table
    rootParameters[4].InitAsDescriptorTable(1, &sampRange, D3D12_SHADER_VISIBILITY_PIXEL);

    rootSignatureDesc.Init(5, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

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
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA,0},
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0}
    };
     
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = m_rootSignature.Get();
    psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShaderBlob.Get());
    psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShaderBlob.Get());
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.RasterizerState.FrontCounterClockwise = TRUE;
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

    duck = Emeika::Model();
    duck.Load("Assets/Geometry/Duck/Duck.gltf", "Assets/Geometry/Duck/");
    duck.GetTransform().SetScale(Vector3(0.01f, 0.01f, 0.01f));
    _models.push_back(&duck);
}

void D3D12Module::RenderBackground(ID3D12GraphicsCommandList4* commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float width, float height)
{
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);

    commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

    // Setup viewport & scissor
    D3D12_VIEWPORT offscreenViewport = { 0,0, width, height, 0.0f, 1.0f };
    D3D12_RECT offscreenScissorRect = { 0, 0, static_cast<LONG>(width), static_cast<LONG>(height) };

    commandList->RSSetViewports(1, &offscreenViewport);
    commandList->RSSetScissorRects(1, &offscreenScissorRect);
}

void D3D12Module::RenderScene(ID3D12GraphicsCommandList4* commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float width, float height)
{
    // Clear + draw
    RenderBackground(commandList, rtvHandle, dsvHandle, width, height);

    // Bind root signature (must be set before any draw calls)
    commandList->SetPipelineState(m_pipelineState.Get());
    commandList->SetGraphicsRootSignature(m_rootSignature.Get());

    //Set input assembler
    ID3D12DescriptorHeap* descriptorHeaps[] = { app->GetDescriptorsModule()->GetSRV()->GetHeap(), app->GetDescriptorsModule()->GetSamplers()->GetHeap() };
    commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

    //Assign composed MVP matrix
    auto camera = app->GetCameraModule();
    SceneData sceneData;
    sceneData.lightDirection = light.direction;
    sceneData.lightColor = light.color;
    sceneData.ambientColor = light.ambientColor;
    sceneData.view = camera->GetPosition();

    sceneData.lightDirection.Normalize();


    commandList->SetGraphicsRootConstantBufferView(1, ringBuffer->Allocate(&sceneData, sizeof(SceneData), GetCurrentFrame()));
    commandList->SetGraphicsRootDescriptorTable(4, app->GetDescriptorsModule()->GetSamplers()->GetGPUHandle(_sampleType));

    //Load all models
    for (int i = 0; i < _models.size(); ++i) {
        std::vector<Emeika::Mesh*> _meshes = _models[i]->GetMeshes();
        std::vector<Emeika::Material*> _materials = _models[i]->GetMaterials();

        Matrix mvp = (_models[i]->GetWorldMatrix() * camera->GetViewMatrix() * camera->GetProjectionMatrix()).Transpose();
        commandList->SetGraphicsRoot32BitConstants(0, sizeof(XMMATRIX) / sizeof(UINT32), &mvp, 0);

        for (int j = 0; j < _meshes.size(); ++j) {
            int32_t materialIndex = _meshes[j]->GetMaterialIndex();

            // Check if material index is valid
            if (materialIndex >= 0 && materialIndex < _materials.size()) {

                ModelData modelData;
                modelData.model = _models[i]->GetWorldMatrix().Transpose();
                modelData.material = _materials[materialIndex]->GetMaterial();
                modelData.normalMat = _models[i]->GetNormalMatrix();

                commandList->SetGraphicsRootConstantBufferView(2, ringBuffer->Allocate(&modelData, sizeof(ModelData), GetCurrentFrame()));
                commandList->SetGraphicsRootDescriptorTable(3, _materials[materialIndex]->GetTexture()->SRV().gpu);

                _meshes[j]->Draw(commandList);
            }
            else {
                // Handle case with no material or invalid material index
                LOG("Warning: Mesh %d has invalid material index %d", j, materialIndex);
                continue;
            }
        }
    }

    if (_showDebugDrawPass) {
        dd::xzSquareGrid(-10.0f, 10.f, 0.0f, 1.0f, dd::colors::LightGray);
        dd::axisTriad(ddConvert(Matrix::Identity), 0.1f, 1.0f);
        debugDrawPass->record(commandList, width, height, camera->GetViewMatrix(), camera->GetProjectionMatrix());
    }
}

void D3D12Module::ToggleDebugDraw()
{
    _showDebugDrawPass = !_showDebugDrawPass;
}



