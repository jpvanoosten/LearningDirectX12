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
 *  @file Material.h
 *  @date October 30, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Material class for scene loading.
 */

#include <DirectXMath.h>  // For vector types.

#include <map>     // For std::map
#include <memory>  // For std::unique_ptr and std::shared_ptr

namespace dx12lib
{
class Texture;

struct alignas( 16 ) MaterialProperties
{
    // The Material properties must be aligned to a 16-byte boundary.
    // To guarantee alignment, the MaterialProperties structure will be allocated in aligned memory.
    MaterialProperties()
    : m_GlobalAmbient( 0.5f, 0.5f, 0.5f, 1.0f )
    , m_AmbientColor( 0, 0, 0, 1 )
    , m_EmissiveColor( 0, 0, 0, 1 )
    , m_DiffuseColor( 1, 1, 1, 1 )
    , m_SpecularColor( 0, 0, 0, 1 )
    , m_Reflectance( 0, 0, 0, 0 )
    , m_Opacity( 1.0f )
    , m_SpecularPower( -1.0f )
    , m_IndexOfRefraction( -1.0f )
    , m_HasAmbientTexture( false )
    , m_HasEmissiveTexture( false )
    , m_HasDiffuseTexture( false )
    , m_HasSpecularTexture( false )
    , m_HasSpecularPowerTexture( false )
    , m_HasNormalTexture( false )
    , m_HasBumpTexture( false )
    , m_HasOpacityTexture( false )
    , m_BumpIntensity( 5.0f )
    , m_SpecularScale( 128.0f )
    , m_AlphaThreshold( 0.1f )
    , m_Padding( 0.0f, 0.0f )
    {}

    DirectX::XMFLOAT4 m_GlobalAmbient;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_AmbientColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_EmissiveColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_DiffuseColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_SpecularColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_Reflectance;
    //-------------------------- ( 16 bytes )
    // If Opacity < 1, then the material is transparent.
    float m_Opacity;
    float m_SpecularPower;
    // For transparent materials, IOR > 0.
    float    m_IndexOfRefraction;
    uint32_t m_HasAmbientTexture;
    //-------------------------- ( 16 bytes )
    uint32_t m_HasEmissiveTexture;
    uint32_t m_HasDiffuseTexture;
    uint32_t m_HasSpecularTexture;
    uint32_t m_HasSpecularPowerTexture;
    //-------------------------- ( 16 bytes )
    uint32_t m_HasNormalTexture;
    uint32_t m_HasBumpTexture;
    uint32_t m_HasOpacityTexture;
    float    m_BumpIntensity;            // When using bump textures (heightmaps) we need
                                         // to scale the height values so the normals are visible.
                                         //-------------------------- ( 16 bytes )
    float m_SpecularScale;               // When reading specular power from a texture,
                                         // we need to scale it into the correct range.
    float             m_AlphaThreshold;  // Pixels with alpha < m_AlphaThreshold will be discarded.
    DirectX::XMFLOAT2 m_Padding;         // Pad to 16 byte boundary.
                                         //-------------------------- ( 16 bytes )
};                                       //--------------------------- ( 16 * 10 = 160 bytes )

class Material
{
public:
    // These are the texture slots that can be bound to the material.
    enum class TextureType
    {
        Ambient,
        Emissive,
        Diffuse,
        Specular,
        SpecularPower,
        Normal,
        Bump,
        Opacity,
        NumTypes,
    };

    Material();

protected:
private:
    using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;

    std::unique_ptr<MaterialProperties> m_MaterialProperties;

    TextureMap m_Textures;
};
}  // namespace dx12lib