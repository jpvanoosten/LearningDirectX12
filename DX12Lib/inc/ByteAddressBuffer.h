/**
 * ByteAddressBuffer.
 * @see https://msdn.microsoft.com/en-us/library/ff471453(v=vs.85).aspx
 */
#pragma once

#include "Buffer.h"

class ByteAddressBuffer : public Buffer
{
public:
    ByteAddressBuffer( const std::wstring& name = L"" );

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
        return m_SRV;
    }

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( uint32_t subresource = 0 ) const override
    {
        return m_UAV;
    }


protected:

private:
    size_t m_BufferSize;

    D3D12_CPU_DESCRIPTOR_HANDLE m_SRV;
    D3D12_CPU_DESCRIPTOR_HANDLE m_UAV;
};