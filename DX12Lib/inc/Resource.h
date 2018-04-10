/**
 * A wrapper for a DX12 resource.
 * This provides a base class for all other resource types (Buffers & Textures).
 */

#include <d3d12.h>
#include <wrl.h>

class Resource
{
public:
    Resource();
    Resource(const D3D12_RESOURCE_DESC& d3d12ResourceDesc);
    Resource(Microsoft::WRL::ComPtr<ID3D12Resource> d3d12Resource);
    virtual ~Resource();

    // Get access to the underlying D3D12 resource
    Microsoft::WRL::ComPtr<ID3D12Resource> GetD3D12Resource() const
    {
        return m_d3d12Resource;
    }



protected:

private:
    // The underlying D3D12 resource.
    Microsoft::WRL::ComPtr<ID3D12Resource> m_d3d12Resource;
};