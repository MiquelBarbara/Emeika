#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>

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
	void CreateSwapChain();
	void CreateDescriptorHeap();
	void CreateCommandAllocators();
	void CreateRTVs();
	void CreateRootSignature();
	void CreatePipelineStateObject();
	void CreateCommandList();


	void GetWindowSize(unsigned& width, unsigned& height);
	void Resize();
	void Flush();

	ID3D12Device5* GetDevice() { return m_device.Get(); }
	HWND GetWindowHandle() { return _hwnd; }
	ID3D12GraphicsCommandList* GetCommandList() { return m_commandList.Get(); }
	ID3D12CommandQueue* GetCommandQueue() { return m_commandQueue.Get(); }
	ID3D12CommandAllocator* GetCommandAllocator() { return m_commandAllocators->Get(); }

	//Fence related functions
	bool IsFenceComplete(UINT16 fenceValue);
	UINT64 Signal();
	void WaitForFence();
	ComPtr<ID3D12Fence> CreateFence(ComPtr<ID3D12Device> device);
private:
	static const UINT FrameCount = 2;

	// The DXGI factory used to create the swap chain and other DXGI objects
	ComPtr<IDXGIFactory6> m_dxgiFactory;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device5> m_device;
	//Provides methods for submitting command lists, synchronizing command list execution, 
	// instrumenting the command queue, and updating resource tile mappings.
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	//Encapsulates a list of graphics commands for rendering. 
	// Includes APIs for instrumenting the command list execution, and for setting and clearing the pipeline state.
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	//The swap chain interface manages the buffers that are used for displaying frames to the screen.
	ComPtr<IDXGISwapChain4> m_swapChain;
	//Render target views (RTVs) are descriptors that allow the pipeline to access render targets.
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	// Command allocators can create command lists and manage the memory that the command lists use to store their commands.
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	//Descriptor heap for render target views
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize;

	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;

	// Synchronization objects.
	UINT m_frameIndex = 0;
	HANDLE m_fenceEvent = nullptr;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount] = {};
	UINT64 m_currentFenceValue = 0;

	HWND _hwnd = nullptr;
	LONG windowWidth = 0;
	LONG windowHeight = 0;

	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> buffer;

};