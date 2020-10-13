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

namespace dx12lib
{

class Device;

class Resource
{
public:
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

    /**
     * Get the SRV for a resource.
     *
     * @param srvDesc The description of the SRV to return. The default is nullptr
     * which returns the default SRV for the resource (the SRV that is created when no
     * description is provided.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE
        GetShaderResourceView( const D3D12_SHADER_RESOURCE_VIEW_DESC* srvDesc = nullptr ) const = 0;

    /**
     * Get the UAV for a (sub)resource.
     *
     * @param uavDesc The description of the UAV to return.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE
        GetUnorderedAccessView( const D3D12_UNORDERED_ACCESS_VIEW_DESC* uavDesc = nullptr ) const = 0;

    /**
     * Set the name of the resource. Useful for debugging purposes.
     * The name of the resource will persist if the underlying D3D12 resource is
     * replaced with SetD3D12Resource.
     */
    void                SetName( const std::wstring& name );
    const std::wstring& GetName() const
    {
        return m_ResourceName;
    }

    /**
     * Check if the resource format supports a specific feature.
     */
    bool CheckFormatSupport( D3D12_FORMAT_SUPPORT1 formatSupport ) const;
    bool CheckFormatSupport( D3D12_FORMAT_SUPPORT2 formatSupport ) const;

protected:
    friend class CommandList;

    // Resource creation should go through the device.
    Resource( std::shared_ptr<Device> device, const D3D12_RESOURCE_DESC& resourceDesc,
                       const D3D12_CLEAR_VALUE* clearValue = nullptr );
    Resource( std::shared_ptr<Device> device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
              const D3D12_CLEAR_VALUE* clearValue = nullptr );

    virtual ~Resource() = default;

    virtual void SetD3D12Resource( Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource );

    // The device that is used to create this resource.
    std::shared_ptr<Device> m_Device;

    // The underlying D3D12 resource.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
    D3D12_FEATURE_DATA_FORMAT_SUPPORT      m_FormatSupport;
    std::unique_ptr<D3D12_CLEAR_VALUE>     m_d3d12ClearValue;
    std::wstring                           m_ResourceName;

private:
    // Check the format support and populate the m_FormatSupport structure.
    void CheckFeatureSupport();
};
}  // namespace dx12lib