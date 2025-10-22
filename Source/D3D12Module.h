#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>

class D3D12Module : public Module
{
public:
	bool init() override;
	bool createSwapChain();
private:
	// Factory used to enumerate graphics adapters and create swap chains (DXGI)
	ComPtr<IDXGIFactory6> m_factory;
	// Enables the Direct3D debug layer for validation and debugging
	ComPtr<ID3D12Debug> m_debugInterface;
	// Represents a physical GPU (graphics adapter)
	ComPtr<IDXGIAdapter4> m_adapter;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device5> m_device;
	// Provides detailed debug and warning messages from the D3D12 runtime
	ComPtr<ID3D12InfoQueue> m_infoQueue;
	//Provides methods for submitting command lists, synchronizing command list execution, 
	// instrumenting the command queue, and updating resource tile mappings.
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	//Encapsulates a list of graphics commands for rendering. 
	// Includes APIs for instrumenting the command list execution, and for setting and clearing the pipeline state.
	ComPtr<ID3D12GraphicsCommandList> m_commandList;

	ComPtr<ID3D12CommandAllocator> m_commandAllocator;

	ComPtr< IDXGISwapChain1> m_swapChain;
};