#include <DX12LibPCH.h>

#include <Texture.h>

#include <Application.h>

Texture::Texture(const std::wstring& name)
    : Resource(name)
{
    m_ShaderResourceView = Application::Get().AllocateDescriptors(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

Texture::~Texture()
{}

void Texture::CreateViews()
{
    if (m_d3d12Resource)
    {
        auto device = Application::Get().GetDevice();
        device->CreateShaderResourceView(m_d3d12Resource.Get(), nullptr, m_ShaderResourceView);
    }
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetShaderResourceView() const
{
    return m_ShaderResourceView;
}

D3D12_CPU_DESCRIPTOR_HANDLE Texture::GetUnorderedAccessView(uint32_t subresource) const
{
    throw std::exception("Texture::GetUnorderedAccessView is not implemented.");
}
