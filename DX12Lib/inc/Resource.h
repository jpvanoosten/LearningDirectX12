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
    Resource( const std::wstring& name = L"" );
    virtual ~Resource();

    // Get access to the underlying D3D12 resource
    Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const
    {
        return m_d3d12Resource;
    }

    // Replace the D3D12 resource
    // Should only be called by the CommandList.
    virtual void SetD3D12Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource);

    /**
     * Get the SRV for a resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const = 0;

    /**
     * Get the UAV for a (sub)resource.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( uint32_t subresource = 0 ) const = 0;

    /**
     * Set the name of the resource. Useful for debugging purposes.
     * The name of the resource will persist if the underlying D3D12 resource is
     * replaced with SetD3D12Resource.
     */
    void SetName(const std::wstring& name);

protected:
    // The underlying D3D12 resource.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;

private:
    std::wstring m_ResourceName;
};