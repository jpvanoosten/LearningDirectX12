/**
 * A wrapper for a DX12 Texture object.
 */
#pragma once

#include "Resource.h"
#include "DescriptorAllocation.h"

#include "d3dx12.h"

#include <map>

class Texture : public Resource
{
public:
    Texture(const std::wstring& name = L"");
    Texture( const D3D12_RESOURCE_DESC& resourceDesc, 
        const D3D12_CLEAR_VALUE* clearValue = nullptr,
        D3D12_RESOURCE_STATES initialState = D3D12_RESOURCE_STATE_COMMON, 
        const std::wstring& name = L"" );
    Texture(Microsoft::WRL::ComPtr<ID3D12Resource> resource, const std::wstring& name = L"");
    Texture(const Texture& copy);
    Texture( Texture&& copy );

    Texture& operator=(const Texture& other);
    Texture& operator=(Texture&& other);

    virtual ~Texture();

    /**
     * Resize the texture.
     */
    void Resize(uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1 );

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
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetUnorderedAccessView( uint32_t mipSlice, uint32_t arraySlice, uint32_t planeSlice ) const override;

    /**
     * Get the RTV for the texture.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;
     
    /**
     * Get the DSV for the texture.
     */
    virtual D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

    static bool CheckSRVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
    {
        return ( ( formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE ) != 0 ||
            ( formatSupport & D3D12_FORMAT_SUPPORT1_SHADER_LOAD ) != 0 );
    }

    static bool CheckRTVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
    {
        return ( ( formatSupport & D3D12_FORMAT_SUPPORT1_RENDER_TARGET ) != 0 );
    }

    static bool CheckUAVSupport(D3D12_FORMAT_SUPPORT1 formatSupport )
    {
        return ( ( formatSupport & D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW) != 0 );
    }

    static bool CheckDSVSupport(D3D12_FORMAT_SUPPORT1 formatSupport)
    {
        return ( ( formatSupport & D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL ) != 0 );
    }

    static bool IsUAVCompatibleFormat(DXGI_FORMAT format);
    static bool IsSRGBFormat(DXGI_FORMAT format);
    static bool IsBGRFormat(DXGI_FORMAT format);
    static bool IsDepthFormat(DXGI_FORMAT format);

    // Return a typeless format from the given format.
    static DXGI_FORMAT GetTypelessFormat(DXGI_FORMAT format);

protected:

private:
    DescriptorAllocation m_ShaderResourceView;
    DescriptorAllocation m_UnorderedAccessViews;
    DescriptorAllocation m_RenderTargetView;
    DescriptorAllocation m_DepthStencilView;
};