#pragma once
#include <Globals.h>

#include <cstdint>
#include <dxgi1_6.h>

constexpr static uint32_t bufferCount = 3;

using SwapChain = IDXGISwapChain4;

/// <summary>
/// Encapsulates all Direct3D 12 resources and operations associated with a
/// single application window, including:
///
/// • Swap chain creation and management (triple-buffered).
/// • Render target view (RTV) descriptor heap creation and RTV allocation.
/// • Depth-stencil buffer creation and DSV allocation.
/// • Handling window resizes and recreating size-dependent resources.
/// • Managing the current back buffer index used for rendering.
/// • Providing accessors for the active render target, viewport, and scissor.
/// </summary>
class Window
{
public:
	constexpr static DXGI_FORMAT DefaultFormat = DXGI_FORMAT_R8G8B8A8_UNORM;
	Window(HWND hWnd);
	~Window();
	ComPtr<SwapChain> CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width,  uint32_t height);
	void Present() const;
	void Resize();

	ComPtr<ID3D12DescriptorHeap> CreateDescriptorHeap(ComPtr<ID3D12Device2> device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t numDescriptors);
	void UpdateRenderTargetViews(ComPtr<ID3D12Device2> device, ComPtr<IDXGISwapChain4> swapChain, ComPtr<ID3D12DescriptorHeap> descriptorHeap);
	void UpdateDepthView(ComPtr<ID3D12Device2> device, ComPtr<ID3D12DescriptorHeap> descriptorHeap);

	bool CheckTearingSupport();
	void GetWindowSize(unsigned& width, unsigned& height);

	constexpr uint32_t Width() const { return (uint32_t)m_viewport.Width; }
	constexpr uint32_t Height() const { return (uint32_t)m_viewport.Height; }
	ID3D12Resource* GetCurrentRenderTarget() const { return m_renderTargets[m_currentBackBufferIndex].resource.Get(); }
	constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetCurrentRenderTargetView() const { return m_renderTargets[m_currentBackBufferIndex].rtvHandle; }
	constexpr D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const { return m_dsvHeap->GetCPUDescriptorHandleForHeapStart(); }
	constexpr const D3D12_VIEWPORT& GetViewport() const { return m_viewport; }
	constexpr const D3D12_RECT& GetScissorRect() const { return m_scissorRect; }
private:

	//This may be changed to a more general ResourceData struct if needed
	struct RenderTargetData
	{
		ComPtr<ID3D12Resource> resource{ nullptr };
		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{};
	};

	struct DepthStencilData
	{
		ComPtr<ID3D12Resource> resource{ nullptr };
		CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle{};
	};

	ComPtr<SwapChain> m_swapChain;
	// RTV
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize{ 0 };
	RenderTargetData m_renderTargets[bufferCount]{};
	// DSV
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	DepthStencilData m_depthStencil;

	mutable uint32_t m_currentBackBufferIndex{ 0 };
	UINT32 _flags{ 0 };

	// Viewport and ScissorRect
	D3D12_VIEWPORT m_viewport{};
	D3D12_RECT m_scissorRect{};
	unsigned int windowWidth{ 0 };
	unsigned int windowHeight{ 0 };
	HWND _hwnd{ nullptr };
};

