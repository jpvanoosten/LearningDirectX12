#include "DX12LibPCH.h"

#include <dx12lib/VertexBuffer.h>

using namespace dx12lib;

VertexBuffer::VertexBuffer( Device& device, size_t numVertices, size_t vertexStride )
    : Buffer(device, CD3DX12_RESOURCE_DESC::Buffer( numVertices * vertexStride ) )
    , m_NumVertices(numVertices)
    , m_VertexStride(vertexStride)
{}

VertexBuffer::VertexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource, size_t numVertices,
    size_t vertexStride)
    : Buffer(device, resource)
    , m_NumVertices(numVertices)
    , m_VertexStride(vertexStride)
{
}

VertexBuffer::~VertexBuffer()
{}
