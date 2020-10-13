#include "DX12LibPCH.h"

#include <dx12lib/StructuredBuffer.h>

#include <dx12lib/Device.h>
#include <dx12lib/ResourceStateTracker.h>
#include <dx12lib/d3dx12.h>

using namespace dx12lib;

StructuredBuffer::StructuredBuffer( std::shared_ptr<Device> device,
                                    size_t numElements, size_t elementSize )
: Buffer( device, CD3DX12_RESOURCE_DESC::Buffer( numElements * elementSize, D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) )
, m_NumElements( numElements )
, m_ElementSize( elementSize )
{
    m_CounterBuffer = device->CreateByteAddressBuffer( 4 );

    CreateViews();
}

StructuredBuffer::StructuredBuffer(std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numElements,
    size_t elementSize)
    : Buffer(device, resource)
    , m_NumElements(numElements)
    , m_ElementSize(elementSize)
{
    m_CounterBuffer = device->CreateByteAddressBuffer( 4 );

    CreateViews();
}


void StructuredBuffer::CreateViews()
{
    auto d3d12Device = m_Device->GetD3D12Device();

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
    srvDesc.Format                          = DXGI_FORMAT_UNKNOWN;
    srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.Buffer.NumElements              = static_cast<UINT>( m_NumElements );
    srvDesc.Buffer.StructureByteStride      = static_cast<UINT>( m_ElementSize );
    srvDesc.Buffer.Flags                    = D3D12_BUFFER_SRV_FLAG_NONE;

    m_SRV = m_Device->AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
    d3d12Device->CreateShaderResourceView( m_d3d12Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle() );

    D3D12_RESOURCE_DESC desc = GetD3D12ResourceDesc();
    if ( desc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS )
    {
        D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
        uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
        uavDesc.Format                           = DXGI_FORMAT_UNKNOWN;
        uavDesc.Buffer.CounterOffsetInBytes      = 0;
        uavDesc.Buffer.NumElements               = static_cast<UINT>( m_NumElements );
        uavDesc.Buffer.StructureByteStride       = static_cast<UINT>( m_ElementSize );
        uavDesc.Buffer.Flags                     = D3D12_BUFFER_UAV_FLAG_NONE;

        m_UAV = m_Device->AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
        d3d12Device->CreateUnorderedAccessView( m_d3d12Resource.Get(), m_CounterBuffer->GetD3D12Resource().Get(),
                                                &uavDesc, m_UAV.GetDescriptorHandle() );
    }
}
