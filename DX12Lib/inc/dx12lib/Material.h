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
    explicit MaterialProperties( const DirectX::XMFLOAT4 diffuseColor  = { 1, 1, 1, 1 },
                                 const DirectX::XMFLOAT4 specularColor = { 1, 1, 1, 1 },
                                 const float             specularPower = 128.0f,
                                 const DirectX::XMFLOAT4 ambientColor  = { 0, 0, 0, 1 },
                                 const DirectX::XMFLOAT4 emissiveColor = { 0, 0, 0, 1 },
                                 const DirectX::XMFLOAT4 reflectance = { 0, 0, 0, 0 }, const float opacity = 1.0f,
                                 const float indexOfRefraction = 1.0f, const float bumpIntensity = 1.0f,
                                 const float alphaThreshold = 0.1f )
    : m_DiffuseColor( diffuseColor )
    , m_SpecularColor( specularColor )
    , m_EmissiveColor( emissiveColor )
    , m_AmbientColor( ambientColor )
    , m_Reflectance( reflectance )
    , m_Opacity( opacity )
    , m_SpecularPower( specularPower )
    , m_IndexOfRefraction( indexOfRefraction )
    , m_BumpIntensity( bumpIntensity )
    , m_AlphaThreshold( alphaThreshold )
    , m_HasAmbientTexture( false )
    , m_HasEmissiveTexture( false )
    , m_HasDiffuseTexture( false )
    , m_HasSpecularTexture( false )
    , m_HasSpecularPowerTexture( false )
    , m_HasNormalTexture( false )
    , m_HasBumpTexture( false )
    , m_HasOpacityTexture( false )
    , m_Padding( 0.0f, 0.0f, 0.0f )
    {}

    DirectX::XMFLOAT4 m_DiffuseColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_SpecularColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_EmissiveColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_AmbientColor;
    //-------------------------- ( 16 bytes )
    DirectX::XMFLOAT4 m_Reflectance;
    //-------------------------- ( 16 bytes )
    // If Opacity < 1, then the material is transparent.
    float m_Opacity;
    float m_SpecularPower;
    // For transparent materials, IOR > 0.
    float m_IndexOfRefraction;
    float m_BumpIntensity;  // When using bump textures (heightmaps) we need
                            // to scale the height values so the normals are visible.
    //-------------------------- ( 16 bytes )
    float    m_AlphaThreshold;  // Pixels with alpha < m_AlphaThreshold will be discarded.
    uint32_t m_HasAmbientTexture;
    uint32_t m_HasEmissiveTexture;
    uint32_t m_HasDiffuseTexture;
    //-------------------------- ( 16 bytes )
    uint32_t m_HasSpecularTexture;
    uint32_t m_HasSpecularPowerTexture;
    uint32_t m_HasNormalTexture;
    uint32_t m_HasBumpTexture;
    //-------------------------- ( 16 bytes )
    uint32_t          m_HasOpacityTexture;
    DirectX::XMFLOAT3 m_Padding;  // Pad to 16 byte boundary.
    //-------------------------- ( 16 bytes )
};  //--------------------------- ( 16 * 9 = 144 bytes )

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

    Material( const MaterialProperties& materialProperties = MaterialProperties() );
    Material( const Material& copy );

    ~Material() = default;

    const DirectX::XMFLOAT4& GetAmbientColor() const;
    void                     SetAmbientColor( const DirectX::XMFLOAT4& ambient );

    const DirectX::XMFLOAT4& GetDiffuseColor() const;
    void                     SetDiffuseColor( const DirectX::XMFLOAT4& diffuse );

    const DirectX::XMFLOAT4& GetEmissiveColor() const;
    void                     SetEmissiveColor( const DirectX::XMFLOAT4& emissive );

    const DirectX::XMFLOAT4& GetSpecularColor() const;
    void                     SetSpecularColor( const DirectX::XMFLOAT4& specular );

    float GetSpecularPower() const;
    void  SetSpecularPower( float specularPower );

    const DirectX::XMFLOAT4& GetReflectance() const;
    void                     SetReflectance( const DirectX::XMFLOAT4& reflectance );

    const float GetOpacity() const;
    void        SetOpacity( float opacity );

    float GetIndexOfRefraction() const;
    void  SetIndexOfRefraction( float indexOfRefraction );

    // When using bump maps, we can adjust the "intensity" of the normals generated
    // from the bump maps. We can even inverse the normals by using a negative intensity.
    // Default bump intensity is 1.0 and a value of 0 will remove the bump effect altogether.
    float GetBumpIntensity() const;
    void  SetBumpIntensity( float bumpIntensity );

    std::shared_ptr<Texture> GetTexture( TextureType ID ) const;
    void                     SetTexture( TextureType type, std::shared_ptr<Texture> texture );

    // This material defines a transparent material
    // if the opacity value is < 1, or there is an opacity map, or the diffuse texture has an alpha channel.
    bool IsTransparent() const;

    const MaterialProperties& GetMaterialProperties() const;

    // Define some interesting materials.
    static const Material Red;
    static const Material Green;
    static const Material Blue;
    static const Material Cyan;
    static const Material Magenta;
    static const Material Yellow;
    static const Material White;
    static const Material Black;
    static const Material Emerald;
    static const Material Jade;
    static const Material Obsidian;
    static const Material Pearl;
    static const Material Ruby;
    static const Material Turquoise;
    static const Material Brass;
    static const Material Bronze;
    static const Material Chrome;
    static const Material Copper;
    static const Material Gold;
    static const Material Silver;
    static const Material BlackPlastic;
    static const Material CyanPlastic;
    static const Material GreenPlastic;
    static const Material RedPlastic;
    static const Material WhitePlastic;
    static const Material YellowPlastic;
    static const Material BlackRubber;
    static const Material CyanRubber;
    static const Material GreenRubber;
    static const Material RedRubber;
    static const Material WhiteRubber;
    static const Material YellowRubber;

protected:
private:
    using TextureMap = std::map<TextureType, std::shared_ptr<Texture>>;
    // A unique pointer with a custom allocator/deallocator to ensure alignment.
    using MaterialPropertiesPtr = std::unique_ptr<MaterialProperties, void ( * )( MaterialProperties* )>;

    MaterialPropertiesPtr m_MaterialProperties;
    TextureMap            m_Textures;
};
}  // namespace dx12lib