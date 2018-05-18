/**
 * Structured buffer resource.
 */
#pragma once

#include "Buffer.h"

#include "ByteAddressBuffer.h"

class StructuredBuffer : public Buffer
{
public:
    StructuredBuffer( const std::wstring& name = L"" );
    StructuredBuffer( const D3D12_RESOURCE_DESC& resDesc, 
        size_t numElements, size_t elementSize,
        const std::wstring& name = L"");

    /**
    * Get the number of elements contained in this buffer.
    */
    virtual size_t GetNumElements() const
    {
        return m_NumElements;
    }

    /**
    * Get the size in bytes of each element in this buffer.
    */
    virtual size_t GetElementSize() const
    {
        return m_ElementSize;
    }

    /**
     * Create the views for the buffer resource.
     * Used by the CommandList when setting the buffer contents.
     */
    virtual void CreateViews( size_t numElements, size_t elementSize ) override;

    /**
     * Get the SRV for a resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const
    {
        return m_SRV.GetDescriptorHandle();
    }

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( uint32_t) const override
    {
        // Buffers don't have subresources.
        return m_UAV.GetDescriptorHandle();
    }
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t, uint32_t, uint32_t) const override
    {
        // Buffers don't have subresources.
        return m_UAV.GetDescriptorHandle();
    }


    const ByteAddressBuffer& GetCounterBuffer() const
    {
        return m_CounterBuffer;
    }

private:
    size_t m_NumElements;
    size_t m_ElementSize;

    DescriptorAllocation m_SRV;
    DescriptorAllocation m_UAV;

    // A buffer to store the internal counter for the structured buffer.
    ByteAddressBuffer m_CounterBuffer;
};