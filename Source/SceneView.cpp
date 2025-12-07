#include "Globals.h"
#include "SceneView.h"
#include "Application.h"
#include "D3D12Module.h"
#include <backends/imgui_impl_dx12.h>


SceneView::SceneView(D3D12Module* d3d12Module, ID3D12DescriptorHeap* imguiSrvHeap, UINT srvIndex)
    : _d3d12Module(d3d12Module), _imguiSrvHeap(imguiSrvHeap), _srvIndex(srvIndex)
{
    _window = _d3d12Module->GetWindow();
    CreateImGuiSRV();
}

SceneView::~SceneView()
{
    _d3d12Module = nullptr;
    _window = nullptr;
}

void SceneView::CreateImGuiSRV(){
    // Create the SRV for the render target
    UpdateSceneTexture();
}

void SceneView::UpdateSceneTexture()
{
    // Obtener el render target actual
    _renderTarget = _window->GetCurrentRenderTarget();

    // Volver a crear el SRV en el descriptor heap de ImGui
    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Format = Window::DefaultFormat;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;

    _d3d12Module->GetDevice()->CreateShaderResourceView(
        _renderTarget.Get(),
        &srvDesc,
        _imguiSrvHeap->GetCPUDescriptorHandleForHeapStart()
    );

    // Actualiza el ImTextureID si es necesario
    _sceneTextureId = (ImTextureID)_imguiSrvHeap->GetGPUDescriptorHandleForHeapStart().ptr;
}


void SceneView::Render()
{
    _sceneSize = ImVec2((float)_window->Width(), (float)_window->Height());

    ImGui::Begin("Scene");
    ImGui::Image(_sceneTextureId, _sceneSize);
    ImGui::End();
}
