#pragma once
#include <Globals.h>
#include "Resources.h"
#include <cstdint>
#include <dxgi1_6.h>

constexpr static uint32_t bufferCount = 3;

using SwapChain = IDXGISwapChain4;

//
// -------------------------------------------------------------------------------------------------
// Window
// -------------------------------------------------------------------------------------------------
// Encapsulates a Win32 window and its associated DirectX 12 rendering resources:
// - Swap chain (triple-buffered)
// - Render targets for each back buffer
// - Depth buffer
// - Viewport and scissor rect
//
class Window
{
public:
	constexpr static DXGI_FORMAT DefaultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	Window(HWND hWnd);
	~Window();
	ComPtr<SwapChain> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width,  uint32_t height);
	void Present() const;
	void Resize();
	void CreateRenderTargetViews(ComPtr<ID3D12Device2> device);
	bool CheckTearingSupport() const;
	void GetWindowSize(unsigned& width, unsigned& height);

	constexpr uint32_t Width() const { return (uint32_t)m_viewport.Width; }
	constexpr uint32_t Height() const { return (uint32_t)m_viewport.Height; }
	ID3D12Resource* GetCurrentRenderTarget() const { return m_renderTargets[m_currentBackBufferIndex].resource.Get(); }
	constexpr DescriptorHandle GetCurrentRenderTargetView() const { return m_renderTargets[m_currentBackBufferIndex].rtv; }
	constexpr DescriptorHandle GetDepthStencilView() const { return m_depthStencil._dsv; }
	constexpr const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
	constexpr const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
private:

	ComPtr<SwapChain> m_swapChain;
	RenderTarget m_renderTargets[bufferCount]{};
	DepthBuffer m_depthStencil;

	mutable uint32_t m_currentBackBufferIndex{ 0 };
	UINT32 _flags{ 0 };

	// Viewport and ScissorRect
	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};
	unsigned int windowWidth{ 0 };
	unsigned int windowHeight{ 0 };
	HWND _hwnd{ nullptr };
};

