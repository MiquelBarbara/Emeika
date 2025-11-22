#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>
#include "DebugDrawPass.h"
#include "CommandQueue.h"


class D3D12Module : public Module
{
public:
	D3D12Module(HWND hwnd);

	bool init() override;
	bool postInit();
	void preRender() override;
	void render() override;
	void postRender() override;
	bool cleanUp() override;

	void LoadPipeline();
	void LoadAssets();
	ComPtr<IDXGISwapChain4> CreateSwapChain(HWND hWnd,ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height, uint32_t bufferCount);
	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device,ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
	void UpdateDepthView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
	void CreateRootSignature();
	void CreatePipelineStateObject();
	bool CheckTearingSupport();
	void TransitionResource(ComPtr<ID3D12GraphicsCommandList> commandList, ComPtr<ID3D12Resource> resource, D3D12_RESOURCE_STATES beforeState, D3D12_RESOURCE_STATES afterState);

	void GetWindowSize(unsigned& width, unsigned& height);
	void Resize();

	ID3D12Device4* GetDevice() { return m_device.Get(); }
	HWND GetWindowHandle() { return _hwnd; }
	CommandQueue* GetCommandQueue() { return _commandQueue.get(); }
	ID3D12GraphicsCommandList4* GetCommandList() { return m_commandList.Get(); }
private:
	static const UINT FrameCount = 2;

	// The DXGI factory used to create the swap chain and other DXGI objects
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device4> m_device;
	//Encapsulates a list of graphics commands for rendering. 
	//The swap chain interface manages the buffers that are used for displaying frames to the screen.
	ComPtr<IDXGISwapChain4> m_swapChain;
	//Render target views (RTVs) are descriptors that allow the pipeline to access render targets.
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	//Descriptor heap for render target views
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12GraphicsCommandList4> m_commandList;
	UINT m_rtvDescriptorSize;
	UINT m_dsvDescriptorSize;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	HWND _hwnd = nullptr;
	unsigned int windowWidth = 0;
	unsigned int windowHeight = 0;

	//Synchronization values
	uint16_t m_frameIndex;
	uint16_t m_fenceValues[FrameCount];


	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;

	ComPtr<ID3D12Resource> buffer;
	ComPtr<ID3D12Resource> depthBuffer;

	std::unique_ptr<DebugDrawPass> debugDrawPass;
	std::unique_ptr<CommandQueue> _commandQueue;
	Matrix model;
	Matrix view;
	Matrix proj;

};