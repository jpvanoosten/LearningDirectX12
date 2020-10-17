#include "DX12LibPCH.h"

#include <dx12lib/DepthStencilView.h>

#include <dx12lib/Device.h>
#include <dx12lib/Texture.h>

using namespace dx12lib;

DepthStencilView::DepthStencilView( const Texture& texture, const D3D12_DEPTH_STENCIL_VIEW_DESC* dsv )
: m_Texture( texture )
{
    auto& device            = texture.GetDevice();
    auto  d3d12Device       = device.GetD3D12Device();
    auto  d3d12Resource     = texture.GetD3D12Resource();
    auto  d3d12ResourceDesc = texture.GetD3D12ResourceDesc();

    // Resource must be created with the D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL flag.
    assert( ( d3d12ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL ) != 0 );

    m_Descriptor = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_DSV );

    d3d12Device->CreateDepthStencilView( d3d12Resource.Get(), dsv, m_Descriptor.GetDescriptorHandle() );
}