#include "DX12LibPCH.h"

#include <dx12lib/IndexBufferView.h>

#include <dx12lib/IndexBuffer.h>

using namespace dx12lib;

IndexBufferView::IndexBufferView( Device& device, std::shared_ptr<IndexBuffer> indexBuffer )
: m_Device( device )
, m_IndexBuffer( indexBuffer )
{
    assert( indexBuffer );

    auto d3d12Resource = indexBuffer->GetD3D12Resource();

    DXGI_FORMAT indexFormat = indexBuffer->GetIndexFormat();
    UINT        numIndicies = indexBuffer->GetNumIndicies();
    UINT        elementSize = ( indexFormat == DXGI_FORMAT_R16_UINT ) ? 2 : 4;

    m_IndexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
    m_IndexBufferView.Format         = indexFormat;
    m_IndexBufferView.SizeInBytes    = numIndicies * elementSize;
}