/**
 * Index buffer resource.
 */
#pragma once

#include "Buffer.h"

class IndexBuffer : public Buffer
{
public:
    IndexBuffer( const std::wstring& name = L"");
    virtual ~IndexBuffer();

    // Inherited from Buffer
    virtual void CreateViews(size_t numElements, size_t elementSize) override;

    size_t GetNumIndicies() const
    {
        return m_NumIndicies;
    }

    DXGI_FORMAT GetIndexFormat() const
    {
        return m_IndexFormat;
    }

    /**
     * Get the index buffer view for biding to the Input Assembler stage.
     */
    D3D12_INDEX_BUFFER_VIEW GetIndexBufferView() const
    {
        return m_IndexBufferView;
    }

    /**
    * Get the SRV for a resource.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const override;

    /**
    * Get the UAV for a (sub)resource.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t subresource) const override;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mipSlice, uint32_t arraySlice, uint32_t planeSlice) const override;

protected:

private:
    size_t m_NumIndicies;
    DXGI_FORMAT m_IndexFormat;

    D3D12_INDEX_BUFFER_VIEW m_IndexBufferView;
};