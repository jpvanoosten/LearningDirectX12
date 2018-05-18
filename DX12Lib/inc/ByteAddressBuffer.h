/**
 * ByteAddressBuffer.
 * @see https://msdn.microsoft.com/en-us/library/ff471453(v=vs.85).aspx
 */
#pragma once

#include "Buffer.h"
#include "DescriptorAllocation.h"

#include <d3dx12.h>

class ByteAddressBuffer : public Buffer
{
public:
    ByteAddressBuffer( const std::wstring& name = L"" );
    ByteAddressBuffer( const D3D12_RESOURCE_DESC& resDesc, 
        size_t numElements, size_t elementSize,
        const std::wstring& name = L"");

    size_t GetBufferSize() const
    {
        return m_BufferSize;
    }

    /**
     * Create the views for the buffer resource.
     * Used by the CommandList when setting the buffer contents.
     */
    virtual void CreateViews( size_t numElements, size_t elementSize ) override;

    /**
     * Get the SRV for a resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const override
    {
        return m_SRV.GetDescriptorHandle();
    }

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( uint32_t subresource ) const override
    {
        // Buffers only have a single subresource.
        return m_UAV.GetDescriptorHandle();
    }

    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mipSlice, uint32_t arraySlice, uint32_t planeSlice) const override
    {
        // Buffers only have a single subresource.
        return m_UAV.GetDescriptorHandle();
    }
    
protected:

private:
    size_t m_BufferSize;

    DescriptorAllocation m_SRV;
    DescriptorAllocation m_UAV;
};