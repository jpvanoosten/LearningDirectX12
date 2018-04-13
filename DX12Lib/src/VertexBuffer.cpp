#include <DX12LibPCH.h>

#include <VertexBuffer.h>

VertexBuffer::VertexBuffer(const std::wstring& name)
    : Buffer(name)
    , m_NumVertices(0)
    , m_VertexStride(0)
    , m_VertexBufferView({})
{}

VertexBuffer::~VertexBuffer()
{}

void VertexBuffer::CreateViews(size_t numElements, size_t elementSize)
{
    m_NumVertices = numElements;
    m_VertexStride = elementSize;

    m_VertexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_VertexBufferView.SizeInBytes = static_cast<UINT>(m_NumVertices * m_VertexStride);
    m_VertexBufferView.StrideInBytes = static_cast<UINT>(m_VertexStride);
}
