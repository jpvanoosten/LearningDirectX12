#include "DX12LibPCH.h"

#include <dx12lib/Material.h>
#include <dx12lib/Texture.h>

using namespace dx12lib;

// Material properties must be 16-byte aligned.
// In order to ensure alignment, the matierial properties is allocated in aligned memory.
static MaterialProperties* NewMaterialProperties( const MaterialProperties& props )
{
    MaterialProperties* materialProperties = (MaterialProperties*)_aligned_malloc( sizeof( MaterialProperties ), 16 );
    *materialProperties                    = props;

    return materialProperties;
}

// Aligned memory must be deleted using the _aligned_free method.
static void DeleteMaterialProperties( MaterialProperties* p )
{
    _aligned_free( p );
}

Material::Material( const MaterialProperties& materialProperties )
: m_MaterialProperties( NewMaterialProperties( materialProperties ), &DeleteMaterialProperties )
{}

Material::Material(const Material& copy) 
: m_MaterialProperties( NewMaterialProperties( *copy.m_MaterialProperties ), &DeleteMaterialProperties )
, m_Textures(copy.m_Textures)
{}

const DirectX::XMFLOAT4& Material::GetAmbientColor() const
{
    return m_MaterialProperties->Ambient;
}

void Material::SetAmbientColor( const DirectX::XMFLOAT4& ambient )
{
    m_MaterialProperties->Ambient = ambient;
}

const DirectX::XMFLOAT4& Material::GetDiffuseColor() const
{
    return m_MaterialProperties->Diffuse;
}

void Material::SetDiffuseColor( const DirectX::XMFLOAT4& diffuse )
{
    m_MaterialProperties->Diffuse = diffuse;
}

const DirectX::XMFLOAT4& Material::GetEmissiveColor() const
{
    return m_MaterialProperties->Emissive;
}

void Material::SetEmissiveColor( const DirectX::XMFLOAT4& emissive )
{
    m_MaterialProperties->Emissive = emissive;
}

const DirectX::XMFLOAT4& Material::GetSpecularColor() const
{
    return m_MaterialProperties->Specular;
}

void Material::SetSpecularColor( const DirectX::XMFLOAT4& specular )
{
    m_MaterialProperties->Specular = specular;
}

float Material::GetSpecularPower() const
{
    return m_MaterialProperties->SpecularPower;
}

void Material::SetSpecularPower( float specularPower )
{
    m_MaterialProperties->SpecularPower = specularPower;
}

const DirectX::XMFLOAT4& Material::GetReflectance() const
{
    return m_MaterialProperties->Reflectance;
}

void Material::SetReflectance( const DirectX::XMFLOAT4& reflectance )
{
    m_MaterialProperties->Reflectance = reflectance;
}

const float Material::GetOpacity() const
{
    return m_MaterialProperties->Opacity;
}

void Material::SetOpacity( float opacity )
{
    m_MaterialProperties->Opacity = opacity;
}

float Material::GetIndexOfRefraction() const
{
    return m_MaterialProperties->IndexOfRefraction;
}

void Material::SetIndexOfRefraction( float indexOfRefraction )
{
    m_MaterialProperties->IndexOfRefraction = indexOfRefraction;
}

float Material::GetBumpIntensity() const
{
    return m_MaterialProperties->BumpIntensity;
}

void Material::SetBumpIntensity( float bumpIntensity )
{
    m_MaterialProperties->BumpIntensity = bumpIntensity;
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
        m_MaterialProperties->HasAmbientTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Emissive:
    {
        m_MaterialProperties->HasEmissiveTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Diffuse:
    {
        m_MaterialProperties->HasDiffuseTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Specular:
    {
        m_MaterialProperties->HasSpecularTexture = ( texture != nullptr );
    }
    break;
    case TextureType::SpecularPower:
    {
        m_MaterialProperties->HasSpecularPowerTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Normal:
    {
        m_MaterialProperties->HasNormalTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Bump:
    {
        m_MaterialProperties->HasBumpTexture = ( texture != nullptr );
    }
    break;
    case TextureType::Opacity:
    {
        m_MaterialProperties->HasOpacityTexture = ( texture != nullptr );
    }
    break;
    }
}

bool Material::IsTransparent() const
{
    return ( m_MaterialProperties->Opacity < 1.0f || m_MaterialProperties->HasOpacityTexture ||
             ( m_MaterialProperties->HasDiffuseTexture && GetTexture( TextureType::Diffuse )->HasAlpha() ) ||
             m_MaterialProperties->AlphaThreshold <= 0.0f );  // Objects with an alpha threshold > 0 should be drawn in the opaque pass.
}

const MaterialProperties& Material::GetMaterialProperties() const
{
    return *m_MaterialProperties;
}

// clang-format off
const MaterialProperties Material::Zero = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 0.0f, 0.0f, 0.0f, 1.0f },
    0.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::Red = {
    { 1.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.1f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::Green = {
    { 0.0f, 1.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.0f, 0.1f, 0.0f, 1.0f }
};

const MaterialProperties Material::Blue = {
    { 0.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.0f, 0.0f, 0.1f, 1.0f }
};

const MaterialProperties Material::Cyan = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.0f, 0.1f, 0.1f, 1.0f }
};

const MaterialProperties Material::Magenta = {
    { 1.0f, 0.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.1f, 0.0f, 0.1f, 1.0f }
};

const MaterialProperties Material::Yellow = {
    { 0.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.0f, 0.1f, 0.1f, 1.0f }
};

const MaterialProperties Material::White = {
    { 1.0f, 1.0f, 1.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.1f, 0.1f, 0.1f, 1.0f }
};

const MaterialProperties Material::Black = {
    { 0.0f, 0.0f, 0.0f, 1.0f },
    { 1.0f, 1.0f, 1.0f, 1.0f },
    128.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::Emerald = {
    { 0.07568f, 0.61424f, 0.07568f, 1.0f },
    { 0.633f, 0.727811f, 0.633f, 1.0f },
    76.8f, 
    { 0.0215f, 0.1745f, 0.0215f, 1.0f }
};

const MaterialProperties Material::Jade = {
    { 0.54f, 0.89f, 0.63f, 1.0f },
    { 0.316228f, 0.316228f, 0.316228f, 1.0f },
    12.8f, 
    { 0.135f, 0.2225f, 0.1575f, 1.0f }
};

const MaterialProperties Material::Obsidian = {
    { 0.18275f, 0.17f, 0.22525f, 1.0f },
    { 0.332741f, 0.328634f, 0.346435f, 1.0f },
    38.4f, 
    { 0.05375f, 0.05f, 0.06625f, 1.0f }
};

const MaterialProperties Material::Pearl = {
    { 1.0f, 0.829f, 0.829f, 1.0f },
    { 0.296648f, 0.296648f, 0.296648f, 1.0f },
    11.264f, 
    { 0.25f, 0.20725f, 0.20725f, 1.0f }
};

const MaterialProperties Material::Ruby = {
    { 0.61424f, 0.04136f, 0.04136f, 1.0f },
    { 0.727811f, 0.626959f, 0.626959f, 1.0f },
    76.8f, 
    { 0.1745f, 0.01175f, 0.01175f, 1.0f }
};

const MaterialProperties Material::Turquoise = {
    { 0.396f, 0.74151f, 0.69102f, 1.0f },
    { 0.297254f, 0.30829f, 0.306678f, 1.0f },
    12.8f, 
    { 0.1f, 0.18725f, 0.1745f, 1.0f }
};

const MaterialProperties Material::Brass = {
    { 0.780392f, 0.568627f, 0.113725f, 1.0f },
    { 0.992157f, 0.941176f, 0.807843f, 1.0f },
    27.9f, 
    { 0.329412f, 0.223529f, 0.027451f, 1.0f }
};

const MaterialProperties Material::Bronze = {
    { 0.714f, 0.4284f, 0.18144f, 1.0f },
    { 0.393548f, 0.271906f, 0.166721f, 1.0f },
    25.6f, 
    { 0.2125f, 0.1275f, 0.054f, 1.0f }
};

const MaterialProperties Material::Chrome = {
    { 0.4f, 0.4f, 0.4f, 1.0f },
    { 0.774597f, 0.774597f, 0.774597f, 1.0f },
    76.8f, 
    { 0.25f, 0.25f, 0.25f, 1.0f }
};

const MaterialProperties Material::Copper = {
    { 0.7038f, 0.27048f, 0.0828f, 1.0f },
    { 0.256777f, 0.137622f, 0.086014f, 1.0f },
    12.8f, 
    { 0.19125f, 0.0735f, 0.0225f, 1.0f }
};

const MaterialProperties Material::Gold = {
    { 0.75164f, 0.60648f, 0.22648f, 1.0f },
    { 0.628281f, 0.555802f, 0.366065f, 1.0f },
    51.2f, 
    { 0.24725f, 0.1995f, 0.0745f, 1.0f }
};

const MaterialProperties Material::Silver = {
    { 0.50754f, 0.50754f, 0.50754f, 1.0f },
    { 0.508273f, 0.508273f, 0.508273f, 1.0f },
    51.2f, 
    { 0.19225f, 0.19225f, 0.19225f, 1.0f }
};

const MaterialProperties Material::BlackPlastic = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    { 0.5f, 0.5f, 0.5f, 1.0f },
    32.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::CyanPlastic = {
    { 0.0f, 0.50980392f, 0.50980392f, 1.0f },
    { 0.50196078f, 0.50196078f, 0.50196078f, 1.0f },
    32.0f, 
    { 0.0f, 0.1f, 0.06f, 1.0f }
};

const MaterialProperties Material::GreenPlastic = {
    { 0.1f, 0.35f, 0.1f, 1.0f },
    { 0.45f, 0.55f, 0.45f, 1.0f },
    32.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::RedPlastic = {
    { 0.5f, 0.0f, 0.0f, 1.0f },
    { 0.7f, 0.6f, 0.6f, 1.0f },
    32.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::WhitePlastic = {
    { 0.55f, 0.55f, 0.55f, 1.0f },
    { 0.7f, 0.7f, 0.7f, 1.0f },
    32.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::YellowPlastic = {
    { 0.5f, 0.5f, 0.0f, 1.0f },
    { 0.6f, 0.6f, 0.5f, 1.0f },
    32.0f, 
    { 0.0f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::BlackRubber = {
    { 0.01f, 0.01f, 0.01f, 1.0f },
    { 0.4f, 0.4f, 0.4f, 1.0f },
    10.0f, 
    { 0.02f, 0.02f, 0.02f, 1.0f }
};

const MaterialProperties Material::CyanRubber = {
    { 0.4f, 0.5f, 0.5f, 1.0f },
    { 0.04f, 0.7f, 0.7f, 1.0f },
    10.0f, 
    { 0.0f, 0.05f, 0.05f, 1.0f }
};

const MaterialProperties Material::GreenRubber = {
    { 0.4f, 0.5f, 0.4f, 1.0f },
    { 0.04f, 0.7f, 0.04f, 1.0f },
    10.0f, 
    { 0.0f, 0.05f, 0.0f, 1.0f }
};

const MaterialProperties Material::RedRubber = {
    { 0.5f, 0.4f, 0.4f, 1.0f },
    { 0.7f, 0.04f, 0.04f, 1.0f },
    10.0f, 
    { 0.05f, 0.0f, 0.0f, 1.0f }
};

const MaterialProperties Material::WhiteRubber = {
    { 0.5f, 0.5f, 0.5f, 1.0f },
    { 0.7f, 0.7f, 0.7f, 1.0f },
    10.0f, 
    { 0.05f, 0.05f, 0.05f, 1.0f }
};

const MaterialProperties Material::YellowRubber = {
    { 0.5f, 0.5f, 0.4f, 1.0f },
    { 0.7f, 0.7f, 0.04f, 1.0f },
    10.0f, 
    { 0.05f, 0.05f, 0.0f, 1.0f }
};
// clang-format on