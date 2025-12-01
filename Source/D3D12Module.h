#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>
#include "DebugDrawPass.h"
#include "CommandQueue.h"
#include "Window.h"


class D3D12Module : public Module
{
public:
	D3D12Module(HWND hwnd);
	~D3D12Module();

	bool init() override;
	bool postInit();
	void preRender() override;
	void render() override;
	void postRender() override;
	bool cleanUp() override;

	void LoadPipeline();
	void LoadAssets();
	
	void CreateRootSignature();
	void CreatePipelineStateObject();
	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	ID3D12Device4* GetDevice() { return m_device.Get(); }
	HWND GetWindowHandle() const { return _hwnd; }
	Window* GetWindow() { return window; }
	CommandQueue* GetCommandQueue() { return _commandQueue.get(); }
	ID3D12GraphicsCommandList4* GetCommandList() { return m_commandList.Get(); }
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

	Matrix model;

};