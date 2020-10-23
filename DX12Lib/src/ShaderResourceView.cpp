#include "DX12LibPCH.h"

#include <dx12lib/ShaderResourceView.h>

#include <dx12lib/Device.h>
#include <dx12lib/Resource.h>

using namespace dx12lib;

ShaderResourceView::ShaderResourceView( Device& device, const std::shared_ptr<Resource>& resource,
                                        const D3D12_SHADER_RESOURCE_VIEW_DESC* srv )
: m_Device( device )
, m_Resource( resource )
{
    assert( resource || srv );

    auto d3d12Resource = m_Resource ? m_Resource->GetD3D12Resource() : nullptr;
    auto d3d12Device   = m_Device.GetD3D12Device();

    m_Descriptor = m_Device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    d3d12Device->CreateShaderResourceView( d3d12Resource.Get(), srv, m_Descriptor.GetDescriptorHandle() );
}
