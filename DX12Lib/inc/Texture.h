/**
 * A wrapper for a DX12 Texture object.
 */
#include <Resource.h>

#include <d3d12.h>
#include <d3dx12.h>

#include <map>

class Texture : public Resource
{
public:
    Texture(const std::wstring& name = L"");
    virtual ~Texture();

    /**
     * Create SRV and UAVs for the resource.
     */
    virtual void CreateViews();

    /**
    * Get the SRV for a resource.
    *
    * @param dxgiFormat The required format of the resource. When accessing a
    * depth-stencil buffer as a shader resource view, the format will be different.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetShaderResourceView() const override;

    /**
    * Get the UAV for a (sub)resource.
    */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView(uint32_t subresource = 0) const override;

protected:

private:
    CD3DX12_CPU_DESCRIPTOR_HANDLE m_ShaderResourceView;
    // A UAV resource only references a single subresource at time.
    // UAVs are created when they are requested (if they haven't been created already)
    std::map<uint32_t, D3D12_CPU_DESCRIPTOR_HANDLE> m_UnorderedAccessViews;
};