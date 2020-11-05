#include "DX12LibPCH.h"

#include <dx12lib/Mesh.h>
#include <dx12lib/Visitor.h>

using namespace dx12lib;

D3D12_INPUT_ELEMENT_DESC Mesh::Vertex::InputElements[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT,
      D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
};

void Mesh::SetVertexBuffer( uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer )
{
    m_VertexBuffers[slotID] = vertexBuffer;
}

std::shared_ptr<VertexBuffer> Mesh::GetVertexBuffer( uint32_t slotID ) const
{
    auto iter         = m_VertexBuffers.find( slotID );
    auto vertexBuffer = iter != m_VertexBuffers.end() ? iter->second : nullptr;

    return vertexBuffer;
}

void Mesh::SetIndexBuffer( const std::shared_ptr<IndexBuffer>& indexBuffer )
{
    m_IndexBuffer = indexBuffer;
}

std::shared_ptr<IndexBuffer> Mesh::GetIndexBuffer()
{
    return m_IndexBuffer;
}

void Mesh::SetMaterial( std::shared_ptr<Material> material )
{
    m_Material = material;
}

std::shared_ptr<Material> Mesh::GetMaterial() const
{
    return m_Material;
}

void Mesh::Accept( Visitor& visitor )
{
    visitor.Visit( *this );
}
