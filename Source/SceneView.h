#pragma once
#include "D3D12Module.h"
#include "Window.h"

class D3D12Module;
class Window;

class SceneView
{
public:
    SceneView(D3D12Module* d3d12Module, ID3D12DescriptorHeap* imguiSrvHeap, UINT srvIndex);
	~SceneView();

	void Render();
    void UpdateSceneTexture();
private:
    D3D12Module* _d3d12Module;
    Window* _window;

    // SRV para la textura de la escena
    ComPtr<ID3D12Resource> _renderTarget;
    ComPtr<ID3D12DescriptorHeap> _imguiSrvHeap;
    UINT _srvIndex;
    ImTextureID _sceneTextureId{};
    ImVec2 _sceneSize = ImVec2(0, 0);

    void CreateImGuiSRV();
};

