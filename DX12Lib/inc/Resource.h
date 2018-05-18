/**
 * A wrapper for a DX12 resource.
 * This provides a base class for all other resource types (Buffers & Textures).
 */
#pragma once

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
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const = 0;

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t subresource ) const = 0;
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t mipSlice, uint32_t arraySlice, uint32_t planeSlice) const = 0;

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