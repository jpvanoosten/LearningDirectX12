#include "DX12LibPCH.h"

#include <dx12lib/ConstantBuffer.h>
#include <dx12lib/Device.h>
#include <dx12lib/d3dx12.h>

using namespace dx12lib;

ConstantBuffer::ConstantBuffer( std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& resourceDesc )
: Buffer( device, resourceDesc )
, m_SizeInBytes( resourceDesc.Width )
{
    CreateViews( 1, m_SizeInBytes );
}

ConstantBuffer::~ConstantBuffer() {}

void ConstantBuffer::CreateViews( size_t numElements, size_t elementSize )
{
    m_SizeInBytes    = numElements * elementSize;
    auto d3d12Device = m_Device->GetD3D12Device();

    m_ConstantBufferView = m_Device->AllocateDescriptors( D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV );

    D3D12_CONSTANT_BUFFER_VIEW_DESC d3d12ConstantBufferViewDesc;
    d3d12ConstantBufferViewDesc.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    d3d12ConstantBufferViewDesc.SizeInBytes    = static_cast<UINT>( Math::AlignUp( m_SizeInBytes, 16 ) );

    d3d12Device->CreateConstantBufferView( &d3d12ConstantBufferViewDesc, m_ConstantBufferView.GetDescriptorHandle() );
}

D3D12_CPU_DESCRIPTOR_HANDLE
ConstantBuffer::GetShaderResourceView( const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc ) const
{
    throw std::exception( "ConstantBuffer::GetShaderResourceView should not be called." );
}

D3D12_CPU_DESCRIPTOR_HANDLE
ConstantBuffer::GetUnorderedAccessView( const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc ) const
{
    throw std::exception( "ConstantBuffer::GetUnorderedAccessView should not be called." );
}
