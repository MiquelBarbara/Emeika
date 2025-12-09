#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>
#include "DebugDrawPass.h"
#include "CommandQueue.h"
#include "DescriptorsModule.h"
#include "Window.h"

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

	void ToggleDebugDraw();
	
	void CreateRootSignature();
	void CreatePipelineStateObject();
	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	ID3D12Device4* GetDevice() const { return m_device.Get(); }
	constexpr HWND GetWindowHandle() const { return _hwnd; }
	constexpr Window* GetWindow() const { return window; }
	CommandQueue* GetCommandQueue() const { return _commandQueue.get(); }
	ID3D12GraphicsCommandList4* GetCommandList() const { return m_commandList.Get(); }

	constexpr bool* GetShowDebugDrawBool() { return &_showDebugDrawPass; }
	void SetSampler(const int type) { _sampleType = static_cast<DescriptorsModule::SampleType>(type); }
	constexpr Matrix* GetModelMatrix() { return &model; }
private:

	// The DXGI factory used to create the swap chain and other DXGI objects
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device4> m_device;
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	//Synchronization values
	uint16_t m_frameIndex;
	uint16_t m_fenceValues[bufferCount];

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Resource> buffer;
	ComPtr<ID3D12Resource> depthBuffer;

	std::unique_ptr<DebugDrawPass> debugDrawPass;
	std::unique_ptr<CommandQueue> _commandQueue;
	Window* window;
	HWND _hwnd;

	Matrix model = Matrix::Identity;

	Texture texture{};

	bool _showDebugDrawPass = true;
	DescriptorsModule::SampleType _sampleType = DescriptorsModule::SampleType::POINT_CLAMP;
};