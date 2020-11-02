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
 *  @file Texture.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief A wrapper for a DX12 Texture object.
 */

#include "DescriptorAllocation.h"
#include "Resource.h"
#include "TextureUsage.h"

#include "d3dx12.h"

#include <mutex>
#include <unordered_map>

namespace dx12lib
{

class Device;

class Texture : public Resource
{
public:
    TextureUsage GetTextureUsage() const
    {
        return m_TextureUsage;
    }

    void SetTextureUsage( TextureUsage textureUsage )
    {
        m_TextureUsage = textureUsage;
    }

    /**
     * Resize the texture.
     */
    void Resize( uint32_t width, uint32_t height, uint32_t depthOrArraySize = 1 );

    /**
     * Get the RTV for the texture.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE GetRenderTargetView() const;

    /**
     * Get the DSV for the texture.
     */
    D3D12_CPU_DESCRIPTOR_HANDLE GetDepthStencilView() const;

    bool CheckSRVSupport() const
    {
        return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_SHADER_SAMPLE );
    }

    bool CheckRTVSupport() const
    {
        return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_RENDER_TARGET );
    }

    bool CheckUAVSupport() const
    {
        return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_TYPED_UNORDERED_ACCESS_VIEW ) &&
               CheckFormatSupport( D3D12_FORMAT_SUPPORT2_UAV_TYPED_LOAD ) &&
               CheckFormatSupport( D3D12_FORMAT_SUPPORT2_UAV_TYPED_STORE );
    }

    bool CheckDSVSupport() const
    {
        return CheckFormatSupport( D3D12_FORMAT_SUPPORT1_DEPTH_STENCIL );
    }

    /**
     * Check to see if the image format has an alpha channel.
     */
    bool HasAlpha() const;

    /**
     * Check the number of bits per pixel.
     */
    size_t BitsPerPixel() const;

    static bool IsUAVCompatibleFormat( DXGI_FORMAT format );
    static bool IsSRGBFormat( DXGI_FORMAT format );
    static bool IsBGRFormat( DXGI_FORMAT format );
    static bool IsDepthFormat( DXGI_FORMAT format );

    // Return a typeless format from the given format.
    static DXGI_FORMAT GetTypelessFormat( DXGI_FORMAT format );
    // Return an sRGB format in the same format family.
    static DXGI_FORMAT GetSRGBFormat( DXGI_FORMAT format );
    static DXGI_FORMAT GetUAVCompatableFormat( DXGI_FORMAT format );

protected:
    Texture( Device& device, const D3D12_RESOURCE_DESC& resourceDesc, TextureUsage texturUsage = TextureUsage::Albedo,
             const D3D12_CLEAR_VALUE* clearValue = nullptr );
    Texture( Device& device, Microsoft::WRL::ComPtr<ID3D12Resource> resource,
             TextureUsage textureUsage = TextureUsage::Albedo, const D3D12_CLEAR_VALUE* clearValue = nullptr );
    virtual ~Texture();

    /**
     * Create SRV and UAVs for the resource.
     */
    void CreateViews();

private:
    TextureUsage m_TextureUsage;

    DescriptorAllocation m_RenderTargetView;
    DescriptorAllocation m_DepthStencilView;
};
}  // namespace dx12lib