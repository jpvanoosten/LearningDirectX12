#include "DX12LibPCH.h"

#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

VertexBuffer::VertexBuffer( std::shared_ptr<Device> device, size_t numVertices, size_t vertexStride )
    : Buffer(device, CD3DX12_RESOURCE_DESC::Buffer( numVertices * vertexStride ) )
    , m_NumVertices(numVertices)
    , m_VertexStride(vertexStride)
    , m_VertexBufferView({})
{
    CreateViews();
}

VertexBuffer::~VertexBuffer()
{}

void VertexBuffer::CreateViews()
{
    m_VertexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(m_NumVertices * m_VertexStride);
    m_VertexBufferView.StrideInBytes = static_cast<UINT>(m_VertexStride);
}

D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc) const
{
    throw std::exception("VertexBuffer::GetShaderResourceView should not be called.");
}

D3D12_CPU_DESCRIPTOR_HANDLE VertexBuffer::GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc) const
{
    throw std::exception("VertexBuffer::GetUnorderedAccessView should not be called.");
}
