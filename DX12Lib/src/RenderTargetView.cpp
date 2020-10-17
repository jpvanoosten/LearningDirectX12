#include "DX12LibPCH.h"

#include <dx12lib/RenderTargetView.h>

#include <dx12lib/Device.h>
#include <dx12lib/Texture.h>

using namespace dx12lib;

RenderTargetView::RenderTargetView(const Texture& texture, const D3D12_RENDER_TARGET_VIEW_DESC* rtv)
: m_Texture(texture)
{
    auto& device = texture.GetDevice();
    auto d3d12Device = device.GetD3D12Device();
    auto d3d12Resource = texture.GetD3D12Resource();
    auto d3d12ResourceDesc = texture.GetD3D12ResourceDesc();

    // Resource must be created with D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET flag.
    assert( ( d3d12ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET ) != 0 );

    m_Descriptor = device.AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

    d3d12Device->CreateRenderTargetView(d3d12Resource.Get(), rtv, m_Descriptor.GetDescriptorHandle() );
}