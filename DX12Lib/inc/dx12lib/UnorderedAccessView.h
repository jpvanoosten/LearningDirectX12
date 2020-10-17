#pragma once

/*
 *  Copyright(c) 2020 Jeremiah van Oosten
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
 *  @file UnorderedAccessView.h
 *  @date October 17, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Wrapper for an Unordered Access View (UAV)
 */

#include <dx12lib/DescriptorAllocation.h>

#include <d3d12.h>  // For D3D12_UNORDERED_ACCESS_VIEW_DESC and D3D12_CPU_DESCRIPTOR_HANDLE

namespace dx12lib
{

class Resource;

class UnorderedAccessView
{
public:
    UnorderedAccessView( const Resource& resource, const Resource* pCounterResource = nullptr,
                         const D3D12_UNORDERED_ACCESS_VIEW_DESC* uav = nullptr );

    const Resource& GetResource() const
    {
        return m_Resource;
    }

    const Resource* GetCounterResource() const
    {
        return m_pCounterResource;
    }

    D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const
    {
        return m_Descriptor.GetDescriptorHandle();
    }

private:
    const Resource&      m_Resource;
    const Resource*      m_pCounterResource;
    DescriptorAllocation m_Descriptor;
};
}  // namespace dx12lib
