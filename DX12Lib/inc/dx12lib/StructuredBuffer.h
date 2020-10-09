#pragma once

/*
 *  Copyright(c) 2018 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions :
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file StructuredBuffer.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief Structured buffer resource.
 */


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
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView(const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr) const
    {
        return m_SRV.GetDescriptorHandle();
    }

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr) const override
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