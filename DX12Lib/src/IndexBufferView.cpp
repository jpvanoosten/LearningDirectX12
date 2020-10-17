#include "DX12LibPCH.h"

#include <dx12lib/IndexBufferView.h>

#include <dx12lib/IndexBuffer.h>

using namespace dx12lib;

IndexBufferView::IndexBufferView( const IndexBuffer& indexBuffer )
: m_IndexBuffer( indexBuffer )
{
    auto d3d12Resource = indexBuffer.GetD3D12Resource();

    DXGI_FORMAT indexFormat = indexBuffer.GetIndexFormat();
    UINT        elementSize = ( indexFormat == DXGI_FORMAT_R16_UINT ) ? 2 : 4;

    m_IndexBufferView.BufferLocation = d3d12Resource->GetGPUVirtualAddress();
    m_IndexBufferView.Format         = indexBuffer.GetIndexFormat();
    m_IndexBufferView.SizeInBytes    = indexBuffer.GetNumIndicies() * elementSize;
}