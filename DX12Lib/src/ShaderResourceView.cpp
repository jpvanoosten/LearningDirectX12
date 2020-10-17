#include "DX12LibPCH.h"

#include <dx12lib/ShaderResourceView.h>

#include <dx12lib/Device.h>
#include <dx12lib/Resource.h>

using namespace dx12lib;

ShaderResourceView::ShaderResourceView( const Resource& resource, const D3D12_SHADER_RESOURCE_VIEW_DESC* srv )
: m_Resource( resource )
{
    auto& device        = m_Resource.GetDevice();
    auto  d3d12Resource = m_Resource.GetD3D12Resource();
    auto  d3d12Device   = device.GetD3D12Device();

    m_Descriptor = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    d3d12Device->CreateShaderResourceView( d3d12Resource.Get(), srv, m_Descriptor.GetDescriptorHandle() );
}
