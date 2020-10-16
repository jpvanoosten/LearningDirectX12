#include "DX12LibPCH.h"

#include <dx12lib/ByteAddressBuffer.h>
#include <dx12lib/Device.h>

using namespace dx12lib;

ByteAddressBuffer::ByteAddressBuffer( Device& device, const D3D12_RESOURCE_DESC& resDesc )
: Buffer( device, resDesc )
{
    CreateViews();
}

ByteAddressBuffer::ByteAddressBuffer( Device& device, ComPtr<ID3D12Resource> resource )
: Buffer( device, resource )
{
    CreateViews();
}

void ByteAddressBuffer::CreateViews()
{
    if ( m_d3d12Resource )
    {
        auto d3d12Device = m_Device.GetD3D12Device();

        D3D12_RESOURCE_DESC resourceDesc = GetD3D12ResourceDesc();

        // Make sure buffer size is aligned to 4 bytes.
        m_BufferSize = Math::AlignUp( resourceDesc.Width, 4 );

        D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
        srvDesc.ViewDimension                   = D3D12_SRV_DIMENSION_BUFFER;
        srvDesc.Format                          = DXGI_FORMAT_R32_TYPELESS;
        srvDesc.Shader4ComponentMapping         = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srvDesc.Buffer.NumElements              = (UINT)m_BufferSize / 4;
        srvDesc.Buffer.Flags                    = D3D12_BUFFER_SRV_FLAG_RAW;

        m_SRV = m_Device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
        d3d12Device->CreateShaderResourceView( m_d3d12Resource.Get(), &srvDesc, m_SRV.GetDescriptorHandle() );

        if ( ( resourceDesc.Flags & D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS ) != 0 )
        {
            D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
            uavDesc.ViewDimension                    = D3D12_UAV_DIMENSION_BUFFER;
            uavDesc.Format                           = DXGI_FORMAT_R32_TYPELESS;
            uavDesc.Buffer.NumElements               = (UINT)m_BufferSize / 4;
            uavDesc.Buffer.Flags                     = D3D12_BUFFER_UAV_FLAG_RAW;

            m_UAV = m_Device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );
            d3d12Device->CreateUnorderedAccessView( m_d3d12Resource.Get(), nullptr, &uavDesc,
                                                    m_UAV.GetDescriptorHandle() );
        }
    }
}
