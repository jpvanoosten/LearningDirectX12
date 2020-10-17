#include "DX12LibPCH.h"

#include <dx12lib/ConstantBufferView.h>

#include <dx12lib/ConstantBuffer.h>
#include <dx12lib/Device.h>

using namespace dx12lib;

ConstantBufferView::ConstantBufferView( const ConstantBuffer&                  constantBuffer,
                                        const D3D12_CONSTANT_BUFFER_VIEW_DESC* _cbv )
: m_ConstantBuffer( constantBuffer )
{
    auto& device        = m_ConstantBuffer.GetDevice();
    auto  d3d12Device   = device.GetD3D12Device();
    auto  d3d12Resource = m_ConstantBuffer.GetD3D12Resource();

    m_Descriptor = device.AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    D3D12_CONSTANT_BUFFER_VIEW_DESC cbv;
    if ( _cbv )
    {
        cbv = *_cbv;
    }
    else
    {
        cbv.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
        cbv.SizeInBytes =
            Math::AlignUp( m_ConstantBuffer.GetSizeInBytes(),
                           D3D12_CONSTANT_BUFFER_DATA_PLACEMENT_ALIGNMENT );  // Constant buffers must be aligned for
                                                                              // hardware requirements.
    }

    d3d12Device->CreateConstantBufferView( &cbv, m_Descriptor.GetDescriptorHandle() );
}
