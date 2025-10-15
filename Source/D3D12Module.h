#pragma once

#include "Module.h"

#include <dxgi1_6.h>
#include <cstdint>
#include <chrono>

class D3D12Module : public Module
{
public:
	bool init() override;
private:
	// Factory used to enumerate graphics adapters and create swap chains (DXGI)
	ComPtr<IDXGIFactory6> factory;
	// Enables the Direct3D debug layer for validation and debugging
	ComPtr<ID3D12Debug> debugInterface;
	// Represents a physical GPU (graphics adapter)
	ComPtr<IDXGIAdapter4> adapter;
	// The main Direct3D 12 device interface used to create resources and command objects
	ComPtr<ID3D12Device5> device;
	// Provides detailed debug and warning messages from the D3D12 runtime
	ComPtr<ID3D12InfoQueue> infoQueue;
};