#include "DX12LibPCH.h"

#include <dx12lib/VertexBufferView.h>

#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

VertexBufferView::VertexBufferView( Device& device, std::shared_ptr<VertexBuffer> vertexBuffer )
: m_Device( device )
, m_VertexBuffer( vertexBuffer )
{
    assert( m_VertexBuffer );

    auto d3d12Resource = m_VertexBuffer->GetD3D12Resource();

    m_VertexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes    = ( vertexBuffer->GetNumVertices() * vertexBuffer->GetVertexStride() );
    m_VertexBufferView.StrideInBytes  = vertexBuffer->GetVertexStride();
}