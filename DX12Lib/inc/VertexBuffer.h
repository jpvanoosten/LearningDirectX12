/**
 * Vertex buffer resource.
 */
#pragma once

#include "Buffer.h"

class VertexBuffer : public Buffer
{
public:
    VertexBuffer(const std::wstring& name = L"");
    virtual ~VertexBuffer();

    // Inherited from Buffer
    virtual void CreateViews(size_t numElements, size_t elementSize) override;

    /**
     * Get the vertex buffer view for binding to the Input Assembler stage.
     */
    D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView() const
    {
        return m_VertexBufferView;
    }

    size_t GetNumVertices() const
    {
        return m_NumVertices;
    }

    size_t GetVertexStride() const
    {
        return m_VertexStride;
    }

    /**
    * Get the SRV for a resource.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const override;

    /**
    * Get the UAV for a (sub)resource.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t subresource = 0) const override;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mipSlice, uint32_t arraySlice = 0, uint32_t planeSlice = 0) const override;

protected:

private:
    size_t m_NumVertices;
    size_t m_VertexStride;

    D3D12_VERTEX_BUFFER_VIEW m_VertexBufferView;
};