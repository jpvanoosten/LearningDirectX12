#include "DX12LibPCH.h"

#include <dx12lib/IndexBuffer.h>

#include <cassert>

using namespace dx12lib;

IndexBuffer::IndexBuffer( Device& device, size_t numIndices, DXGI_FORMAT indexFormat )
: Buffer( device, CD3DX12_RESOURCE_DESC::Buffer( numIndices * ( indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4 ) ) )
, m_NumIndices( numIndices )
, m_IndexFormat( indexFormat )
, m_IndexBufferView {}
{
    assert( indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT );
    CreateIndexBufferView();
}

IndexBuffer::IndexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                   size_t numIndices, DXGI_FORMAT indexFormat )
: Buffer( device, resource )
, m_NumIndices( numIndices )
, m_IndexFormat( indexFormat )
, m_IndexBufferView {}
{
    assert( indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT );
    CreateIndexBufferView();
}

void IndexBuffer::CreateIndexBufferView()
{
    UINT bufferSize = m_NumIndices * ( m_IndexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4 );

    m_IndexBufferView.BufferLocation = m_d3d12Resource->GetGPUVirtualAddress();
    m_IndexBufferView.SizeInBytes    = bufferSize;
    m_IndexBufferView.Format         = m_IndexFormat;
}
