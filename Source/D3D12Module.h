#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>

class D3D12Module : public Module
{
public:
	bool init() override;
	void preRender() override;
	void render() override;
	void postRender() override;
	bool cleanUp() override;
	void loadPipeline();
	void populateCommandList();
	void waitForFence(UINT64& fenceValue);
private:
	static const UINT FrameCount = 2;

	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device5> m_device;
	//Provides methods for submitting command lists, synchronizing command list execution, 
	// instrumenting the command queue, and updating resource tile mappings.
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	//Encapsulates a list of graphics commands for rendering. 
	// Includes APIs for instrumenting the command list execution, and for setting and clearing the pipeline state.
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<IDXGISwapChain4> m_swapChain;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];
	ComPtr<ID3D12CommandAllocator> m_commandAllocators[FrameCount];
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize;

	// Synchronization objects.
	UINT m_frameIndex = 0;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValues[FrameCount] = {};
};