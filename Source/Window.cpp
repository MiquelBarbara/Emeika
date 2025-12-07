#include <Globals.h>
#include "Window.h"
#include "Application.h"
#include "ResourcesModule.h"
#include "CameraModule.h"
#include "D3D12Module.h"

Window::Window(HWND hWnd): _hwnd(hWnd)
{
    GetWindowSize(windowWidth, windowHeight);
    m_swapChain = CreateSwapChain(hWnd, app->GetD3D12Module()->GetCommandQueue()->GetD3D12CommandQueue(), windowWidth, windowHeight);
    m_currentBackBufferIndex = m_swapChain.Get()->GetCurrentBackBufferIndex();
    m_depthStencil = app->GetResourcesModule()->CreateDepthBuffer(windowWidth, windowHeight);
    CreateRenderTargetViews(app->GetD3D12Module()->GetDevice());

    m_viewport = D3D12_VIEWPORT{ 0.0, 0.0, float(windowWidth), float(windowHeight) , 0.0, 1.0 };
    m_scissorRect = D3D12_RECT { 0, 0, long(windowWidth), long(windowHeight) };
}

Window::~Window()
{
    // 1. Release render targets
    for (int i = 0; i < bufferCount; ++i)
        m_renderTargets[i].resource.Reset();

    // 2. Release depth stencil
    //m_depthStencil.resource.Reset();

    // 3. Release descriptor heaps
    //m_rtvHeap.Reset();
    //m_dsvHeap.Reset();

    // 4. Release swap chain
    m_swapChain.Reset();
}

ComPtr<SwapChain> Window::CreateSwapChain(HWND hWnd, ComPtr<ID3D12CommandQueue> commandQueue, uint32_t width, uint32_t height)
{
    ComPtr<SwapChain> dxgiSwapChain4;
    ComPtr<IDXGIFactory4> dxgiFactory4;
    UINT createFactoryFlags = 0;
#if defined(_DEBUG)
    createFactoryFlags = DXGI_CREATE_FACTORY_DEBUG;
#endif

    DXCall(CreateDXGIFactory2(createFactoryFlags, IID_PPV_ARGS(&dxgiFactory4)));

    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
    swapChainDesc.Width = width;
    swapChainDesc.Height = height;
    swapChainDesc.Format = DefaultFormat; // 32-bit RGBA format (8 bits per channel)
    // UNORM = Unsigned normalized integer (0-255 mapped to 0.0-1.0)
    swapChainDesc.Stereo = FALSE; // Set to TRUE for stereoscopic 3D rendering (VR/3D Vision)
    swapChainDesc.SampleDesc = { 1, 0 }; // Multisampling { Count, Quality } // Count=1: No multisampling (1 sample per pixel)
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // This buffer will be used as a render target
    swapChainDesc.BufferCount = bufferCount;
    // - 1 front buffer (displayed)
   // - 1 back buffer (rendering)
    swapChainDesc.Scaling = DXGI_SCALING_STRETCH; // How to scale when window size doesn't match buffer size:
    // STRETCH = Stretch the image to fit the window
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // Modern efficient swap method:
    // - FLIP: Uses page flipping (no copying)
   // - DISCARD: Discard previous back buffer contents
    swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED; // Alpha channel behavior for window blending UNSPECIFIED = Use default behavior
    swapChainDesc.Flags = CheckTearingSupport() ? DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING : 0;
    // DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH: Allow full-screen mode switches
   //DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING: Allow tearing in windowed mode (VSync off)

    ComPtr<IDXGISwapChain1> swapChain1;

    DXCall(dxgiFactory4->CreateSwapChainForHwnd(
        commandQueue.Get(),
        hWnd,
        &swapChainDesc,
        nullptr, // fullscreen desc
        nullptr, // restrict output
        &swapChain1
    ));

    swapChain1.As(&dxgiSwapChain4);

    return dxgiSwapChain4;
}

void Window::Present() const
{
    m_swapChain.Get()->Present(0, DXGI_PRESENT_ALLOW_TEARING);
    m_currentBackBufferIndex = m_swapChain.Get()->GetCurrentBackBufferIndex();
}

void Window::GetWindowSize(unsigned& width, unsigned& height) {
    RECT rect = {};
    GetClientRect(_hwnd, &rect);

    width = unsigned(rect.right - rect.left);
    height = unsigned(rect.bottom - rect.top);
}


void Window::Resize()
{
    unsigned width, height;
    GetWindowSize(width, height);

    if (width != windowWidth || height != windowHeight) {

        windowWidth = width;
        windowHeight = height;

        m_viewport = D3D12_VIEWPORT{ 0.0, 0.0, float(windowWidth), float(windowHeight) , 0.0, 1.0 };
        m_scissorRect = D3D12_RECT{ 0, 0, long(windowWidth), long(windowHeight) };

        app->GetCameraModule()->Resize();

        // Ensure GPU is finished with ALL pending work
        app->GetD3D12Module()->GetCommandQueue()->Flush();
        //_commandQueue->Flush();

        // Release the render targets
        for (UINT n = 0; n < bufferCount; n++)
        {
            m_renderTargets[n].resource.Reset();
        }

        // Resize the swap chain
        DXGI_SWAP_CHAIN_DESC swapChainDesc = {};

        DXCall(m_swapChain->GetDesc(&swapChainDesc));

        DXCall(m_swapChain->ResizeBuffers(bufferCount, width, height, swapChainDesc.BufferDesc.Format, swapChainDesc.Flags));

        m_currentBackBufferIndex = m_swapChain->GetCurrentBackBufferIndex();

        // Recreate the render target views
        CreateRenderTargetViews(app->GetD3D12Module()->GetDevice());
        m_depthStencil = app->GetResourcesModule()->CreateDepthBuffer(windowWidth, windowHeight);
    }
}


void Window::CreateRenderTargetViews(ComPtr<ID3D12Device2> device)
{    
    for (UINT n = 0; n < bufferCount; n++)
    {
        auto rtvHandle = app->GetDescriptorsModule()->GetRTV()->Allocate();
        m_renderTargets[n].rtv = rtvHandle;
        DXCall(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n].resource)));
        device->CreateRenderTargetView(m_renderTargets[n].resource.Get(), nullptr, rtvHandle.cpu);
    }
}


bool Window::CheckTearingSupport()
{
    BOOL allowTearing = FALSE;

    // Rather than create the DXGI 1.5 factory interface directly, we create the
    // DXGI 1.4 interface and query for the 1.5 interface. This is to enable the 
    // graphics debugging tools which will not support the 1.5 factory interface 
    // until a future update.
    ComPtr<IDXGIFactory4> factory4;
    if (SUCCEEDED(CreateDXGIFactory1(IID_PPV_ARGS(&factory4))))
    {
        ComPtr<IDXGIFactory5> factory5;
        if (SUCCEEDED(factory4.As(&factory5)))
        {
            if (FAILED(factory5->CheckFeatureSupport(
                DXGI_FEATURE_PRESENT_ALLOW_TEARING,
                &allowTearing, sizeof(allowTearing))))
            {
                allowTearing = FALSE;
            }
        }
    }

    return allowTearing == TRUE;
}
