#include "DX12LibPCH.h"

#include <dx12lib/UnorderedAccessView.h>

#include <dx12lib/Device.h>
#include <dx12lib/Resource.h>

using namespace dx12lib;

UnorderedAccessView::UnorderedAccessView( const Resource& resource, const Resource* pCounterResource,
                                          const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav )
: m_Resource( resource )
, m_pCounterResource(pCounterResource)
{
    auto& device               = resource.GetDevice();
    auto  d3d12Device          = device.GetD3D12Device();
    auto  d3d12Resource        = resource.GetD3D12Resource();
    auto  d3d12CounterResource = pCounterResource ? pCounterResource->GetD3D12Resource() : nullptr;
    auto  d3d12ResourceDesc    = resource.GetD3D12ResourceDesc();

    // Resource must be created with the D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS flag.
    assert( ( d3d12ResourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) != 0 );

    m_Descriptor = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    d3d12Device->CreateUnorderedAccessView( d3d12Resource.Get(), d3d12CounterResource.Get(), uav,
                                            m_Descriptor.GetDescriptorHandle() );
}
