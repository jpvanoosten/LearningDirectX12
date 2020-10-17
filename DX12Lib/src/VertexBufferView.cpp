#include "DX12LibPCH.h"

#include <dx12lib/VertexBufferView.h>

#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

VertexBufferView::VertexBufferView(const VertexBuffer& vertexBuffer)
    : m_VertexBuffer(vertexBuffer)
{
    auto d3d12Resource = vertexBuffer.GetD3D12Resource();

    m_VertexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = ( vertexBuffer.GetNumVertices() * vertexBuffer.GetVertexStride() );
    m_VertexBufferView.StrideInBytes = vertexBuffer.GetVertexStride();
}