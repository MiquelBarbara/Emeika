#include "Globals.h"
#include "Application.h"
#include "D3D12Module.h"
#include "ResourcesModule.h"
#include "Resources.h"
#include "Globals.h"

Texture::Texture(TextureInitInfo info)
{
    auto device = app->GetD3D12Module()->GetDevice();

    D3D12_CLEAR_VALUE* const clear_value
    {
        (info.desc &&
            (info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ||
            info.desc->Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL))
            ? &info.clearValue : nullptr
    };

    if (info.resource) {
        _resource = info.resource;
    }
    else if (info.heap) {
        assert(!info.resource);
        DXCall(device->CreatePlacedResource(
            info.heap,
            info.allocInfo.Offset,
            info.desc,
            info.initialState,
            clear_value,
            IID_PPV_ARGS(&_resource)
        ));
    }
    else if (info.desc) {
        assert(!info.heap && !info.resource);
        CD3DX12_HEAP_PROPERTIES heap = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
        DXCall(device->CreateCommittedResource(
            &heap,
            D3D12_HEAP_FLAG_NONE,
            info.desc,
            info.initialState,
            clear_value,
            IID_PPV_ARGS(&_resource)
        ));
    }

    assert(_resource);
    _srv = app->GetDescriptorsModule()->GetSRV()->Allocate();
    device->CreateShaderResourceView(_resource.Get(), info.srvDesc, _srv.cpu);
}

void Texture::Release()
{
    app->GetDescriptorsModule()->DefferDescriptorRelease((Handle)_srv.index);
	app->GetResourcesModule()->DefferResourceRelease(_resource);
}

RenderTexture::RenderTexture(TextureInitInfo info): _texture(info)
{
    assert(info.desc);
    _mipCount = GetResource()->GetDesc().MipLevels;
    assert(_mipCount && _mipCount <= Texture::maxMips);

    D3D12_RENDER_TARGET_VIEW_DESC desc{};
    desc.Format = info.desc->Format;
    desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
    desc.Texture2D.MipSlice = 0;

    auto device = app->GetD3D12Module()->GetDevice();

    for (int i{ 0 }; i < _mipCount; i++) {
        _rtv[i] = app->GetDescriptorsModule()->GetOffscreenRTV()->Allocate();
        device->CreateRenderTargetView(GetResource(), &desc, _rtv[i].cpu);
        ++desc.Texture2D.MipSlice;
    }
}


void RenderTexture::Release()
{
    for (uint32_t i = 0; i < _mipCount; ++i) {
        app->GetDescriptorsModule()->GetOffscreenRTV()->Free(_rtv[i].index);
    }
    _mipCount = 0;
}


DepthBuffer::DepthBuffer(TextureInitInfo info)
{
    const DXGI_FORMAT format{ info.desc->Format };

    D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc{};
    if (info.desc->Format == DXGI_FORMAT_D32_FLOAT) {
        info.desc->Format = DXGI_FORMAT_R32_TYPELESS;
        srv_desc.Format = DXGI_FORMAT_R32_FLOAT;
    }

    srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srv_desc.Texture2D.MipLevels = 1;
    srv_desc.Texture2D.MostDetailedMip = 0;
    srv_desc.Texture2D.PlaneSlice = 0;
    srv_desc.Texture2D.ResourceMinLODClamp = 0.0f;

    assert(!info.srvDesc && !info.resource);
    info.srvDesc = &srv_desc;
    _texture = Texture{ info };

    D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc{};
    dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
    dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
    dsv_desc.Format = format;
    dsv_desc.Texture2D.MipSlice = 0;

    _dsv = app->GetDescriptorsModule()->GetDSV()->Allocate();

    auto device = app->GetD3D12Module()->GetDevice();

    device->CreateDepthStencilView(GetResource(), &dsv_desc, _dsv.cpu);
}

void DepthBuffer::Release()
{
    app->GetDescriptorsModule()->GetDSV()->Free(_dsv.index);
}
