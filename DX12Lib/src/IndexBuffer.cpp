#include "DX12LibPCH.h"

#include <dx12lib/IndexBuffer.h>

#include <cassert>

using namespace dx12lib;

IndexBuffer::IndexBuffer( Device& device, size_t numIndicies, DXGI_FORMAT indexFormat )
: Buffer( device, CD3DX12_RESOURCE_DESC::Buffer( numIndicies * ( indexFormat == DXGI_FORMAT_R16_UINT ? 2 : 4 ) ) )
, m_NumIndicies( numIndicies )
, m_IndexFormat( indexFormat )
{
    assert( indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT );
}

IndexBuffer::IndexBuffer( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
                                   size_t numIndicies, DXGI_FORMAT indexFormat )
: Buffer( device, resource )
, m_NumIndicies( numIndicies )
, m_IndexFormat( indexFormat )
{
    assert( indexFormat == DXGI_FORMAT_R16_UINT || indexFormat == DXGI_FORMAT_R32_UINT );
}

IndexBuffer::~IndexBuffer() {}
