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
 *  @file Resource.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief A wrapper for a DX12 resource. This provides a base class for all 
 *  other resource types (Buffers & Textures).
 */

#include <d3d12.h>
#include <wrl.h>

#include <string>

class Resource
{
public:
    Resource(const std::wstring& name = L"");
    Resource(const D3D12_RESOURCE_DESC& resourceDesc, 
        const D3D12_CLEAR_VALUE* clearValue = nullptr,
        const std::wstring& name = L"");
    Resource(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");
    Resource(const Resource& copy);
    Resource(Resource&& copy);

    Resource& operator=( const Resource& other);
    Resource& operator=(Resource&& other);

    virtual ~Resource();

    /**
     * Check to see if the underlying resource is valid.
     */
    bool IsValid() const
    {
        return ( m_d3d12Resource != nullptr );
    }

    // Get access to the underlying D3D12 resource
    Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const
    {
        return m_d3d12Resource;
    }

    D3D12_RESOURCE_DESC GetD3D12ResourceDesc() const
    {
        D3D12_RESOURCE_DESC resDesc = {};
        if ( m_d3d12Resource )
        {
            resDesc = m_d3d12Resource->GetDesc();
        }

        return resDesc;
    }

    // Replace the D3D12 resource
    // Should only be called by the CommandList.
    virtual void SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource, 
        const D3D12_CLEAR_VALUE* clearValue = nullptr );

    /**
     * Get the SRV for a resource.
     * 
     * @param srvDesc The description of the SRV to return. The default is nullptr 
     * which returns the default SRV for the resource (the SRV that is created when no 
     * description is provided.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView( const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr ) const = 0;

    /**
     * Get the UAV for a (sub)resource.
     * 
     * @param uavDesc The description of the UAV to return.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr ) const = 0;

    /**
     * Set the name of the resource. Useful for debugging purposes.
     * The name of the resource will persist if the underlying D3D12 resource is
     * replaced with SetD3D12Resource.
     */
    void SetName(const std::wstring& name);

    /**
     * Release the underlying resource.
     * This is useful for swap chain resizing.
     */
    virtual void Reset();

protected:
    // The underlying D3D12 resource.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
    std::unique_ptr<D3D12_CLEAR_VALUE> m_d3d12ClearValue;
    std::wstring m_ResourceName;
};