#include "DX12LibPCH.h"

#include <dx12lib/Material.h>
#include <dx12lib/Texture.h>

using namespace dx12lib;

// Material properties must be 16-byte aligned.
// In order to ensure alignment, the matierial properties is allocated in aligned memory.
static MaterialProperties* NewMaterialProperties()
{
    MaterialProperties* materialProperties = (MaterialProperties*)_aligned_malloc( sizeof( MaterialProperties ), 16 );
    *materialProperties                    = MaterialProperties();  // Default construct material properties.

    return materialProperties;
}

// Aligned memory must be deleted using the _aligned_free method.
static void DeleteMaterialProperties( MaterialProperties* p )
{
    _aligned_free( p );
}

Material::Material()
: m_MaterialProperties( NewMaterialProperties(), &DeleteMaterialProperties )
{}

const DirectX::XMFLOAT4& Material::GetGlobalAmbientColor() const
{
    return m_MaterialProperties->m_GlobalAmbient;
}

void Material::SetGlobalAmbientColor( const DirectX::XMFLOAT4& globalAmbient )
{
    m_MaterialProperties->m_GlobalAmbient = globalAmbient;
}

const DirectX::XMFLOAT4& Material::GetAmbientColor() const
{
    return m_MaterialProperties->m_AmbientColor;
}

void Material::SetAmbientColor( const DirectX::XMFLOAT4& ambient )
{
    m_MaterialProperties->m_AmbientColor = ambient;
}

const DirectX::XMFLOAT4& Material::GetDiffuseColor() const
{
    return m_MaterialProperties->m_DiffuseColor;
}

void Material::SetDiffuseColor( const DirectX::XMFLOAT4& diffuse )
{
    m_MaterialProperties->m_DiffuseColor = diffuse;
}

const DirectX::XMFLOAT4& Material::GetEmissiveColor() const
{
    return m_MaterialProperties->m_EmissiveColor;
}

void Material::SetEmissiveColor( const DirectX::XMFLOAT4& emissive )
{
    m_MaterialProperties->m_EmissiveColor = emissive;
}

const DirectX::XMFLOAT4& Material::GetSpecularColor() const
{
    return m_MaterialProperties->m_SpecularColor;
}

void Material::SetSpecularColor( const DirectX::XMFLOAT4& specular )
{
    m_MaterialProperties->m_SpecularColor = specular;
}

float Material::GetSpecularPower() const
{
    return m_MaterialProperties->m_SpecularPower;
}

void Material::SetSpecularPower( float specularPower )
{
    m_MaterialProperties->m_SpecularPower = specularPower;
}

const DirectX::XMFLOAT4& Material::GetReflectance() const
{
    return m_MaterialProperties->m_Reflectance;
}

void Material::SetReflectance( const DirectX::XMFLOAT4& reflectance )
{
    m_MaterialProperties->m_Reflectance = reflectance;
}

const float Material::GetOpacity() const
{
    return m_MaterialProperties->m_Opacity;
}

void Material::SetOpacity( float opacity )
{
    m_MaterialProperties->m_Opacity = opacity;
}

float Material::GetIndexOfRefraction() const
{
    return m_MaterialProperties->m_IndexOfRefraction;
}

void Material::SetIndexOfRefraction( float indexOfRefraction )
{
    m_MaterialProperties->m_IndexOfRefraction = indexOfRefraction;
}

float Material::GetBumpIntensity() const
{
    return m_MaterialProperties->m_BumpIntensity;
}

void Material::SetBumpIntensity( float bumpIntensity )
{
    m_MaterialProperties->m_BumpIntensity = bumpIntensity;
}

std::shared_ptr<Texture> Material::GetTexture( TextureType ID ) const
{
    TextureMap::const_iterator iter = m_Textures.find( ID );
    if ( iter != m_Textures.end() )
    {
        return iter->second;
    }

    return nullptr;
}

void Material::SetTexture( TextureType type, std::shared_ptr<Texture> texture )
{
    m_Textures[type] = texture;

    switch ( type )
    {
    case TextureType::Ambient:
    {
        m_MaterialProperties->m_HasAmbientTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Emissive:
    {
        m_MaterialProperties->m_HasEmissiveTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Diffuse:
    {
        m_MaterialProperties->m_HasDiffuseTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Specular:
    {
        m_MaterialProperties->m_HasSpecularTexture = ( texture != nullptr );
    }
    break;
    case TextureType::SpecularPower:
    {
        m_MaterialProperties->m_HasSpecularPowerTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Normal:
    {
        m_MaterialProperties->m_HasNormalTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Bump:
    {
        m_MaterialProperties->m_HasBumpTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Opacity:
    {
        m_MaterialProperties->m_HasOpacityTexture = ( texture != nullptr );
    }
    break;
    }
}

bool Material::IsTransparent() const
{
    return ( m_MaterialProperties->m_Opacity < 1.0f || m_MaterialProperties->m_HasOpacityTexture ||
             ( m_MaterialProperties->m_HasDiffuseTexture && GetTexture( TextureType::Diffuse )->HasAlpha() ) ||
             m_MaterialProperties->m_AlphaThreshold <= 0.0f );  // Objects with an alpha threshold > 0 should be drawn in the opaque pass.
}

const MaterialProperties& Material::GetMaterialProperties() const
{
    return *m_MaterialProperties;
}
