#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>
#include "DebugDrawPass.h"
#include "CommandQueue.h"
#include "DescriptorsModule.h"
#include "SwapChain.h"
#include "Model.h"
#include "Material.h"


struct ModelData {
	Matrix model;
	Matrix normalMat;
	Emeika::Material::BDRFPhongMaterialData material;
};

struct SceneData {
	Vector3 lightDirection;
	float pad0;
	Vector3 lightColor;
	float pad1;
	Vector3 ambientColor;
	float pad2;
	Vector3 view;
	float pad3;
};

struct Light {
	Vector3 direction = Vector3::One * (-0.7f);
	Vector3 color = Vector3(1.0f, 1.0f, 1.0f);
	Vector3 ambientColor = Vector3::One * (0.1f);
};


class RingBuffer;

// -----------------------------------------------------------------------------
// D3D12Module
// -----------------------------------------------------------------------------
class D3D12Module : public Module
{
public:
	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;
	bool postInit() override;
	void preRender() override;
	void render() override;
	void postRender() override;
	bool cleanUp() override;

	void LoadPipeline();
	void LoadAssets();
	void RenderBackground(ID3D12GraphicsCommandList4* commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float width, float height);
	void RenderScene(ID3D12GraphicsCommandList4* commandList, D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle, D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle, float width, float height);
	void ToggleDebugDraw();
	
	void CreateRootSignature();
	void CreatePipelineStateObject();
	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	ID3D12Device4* GetDevice() const { return m_device.Get(); }
	constexpr HWND GetWindowHandle() const { return _hwnd; }
	constexpr SwapChain* GetSwapChain() const { return _swapChain; }
	CommandQueue* GetCommandQueue() const { return _commandQueue.get(); }
	ID3D12GraphicsCommandList4* GetCommandList() const { return m_commandList.Get(); }
	RenderTexture* GetOffscreenRenderTarget() { return offscreenRenderTarget.get(); }

	constexpr bool* GetShowDebugDrawBool() { return &_showDebugDrawPass; }
	void SetSampler(const int type) { _sampleType = static_cast<DescriptorsModule::SampleType>(type); }
	IDXGIAdapter4* GetAdapter() const { return m_adapter.Get(); }
	Emeika::Model* GetDuck() { return &duck; }

	uint64_t GetCurrentFrame() const { return m_fenceValues[m_frameIndex]; }
	uint64_t GetLastCompletedFrame() const { return m_lastCompletedFenceValue; }
private:
	ComPtr<IDXGIAdapter4> m_adapter;
	// The DXGI factory used to create the swap chain and other DXGI objects
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device4> m_device;
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	//Synchronization values
	uint64_t m_frameIndex;
	uint64_t m_fenceValues[bufferCount];
	uint64_t m_lastCompletedFenceValue;

	std::unique_ptr<DebugDrawPass> debugDrawPass;
	std::unique_ptr<CommandQueue> _commandQueue;
	SwapChain* _swapChain;
	HWND _hwnd;

	Matrix model = Matrix::Identity;

	bool _showDebugDrawPass = true;
	DescriptorsModule::SampleType _sampleType = DescriptorsModule::SampleType::POINT_CLAMP;

	//Scene Editor Offscreen Render Target
	std::unique_ptr<RenderTexture> offscreenRenderTarget{};
	std::unique_ptr<DepthBuffer> offscreenDepthBuffer{};
	ImVec2 offscreenTextureSize = ImVec2(800, 600);

	std::vector<Emeika::Model*> _models;
	Emeika::Model duck;
	RingBuffer* ringBuffer;
	Light light;

	const static int _numRootParameters = 4;
};