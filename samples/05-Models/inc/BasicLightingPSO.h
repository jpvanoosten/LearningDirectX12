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
 *  @file BasicLightingPSO.h
 *  @date November 12, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief Basic lighting effect.
 */

#include "EffectPSO.h"
#include "Light.h"

#include <DirectXMath.h>

#include <vector>

namespace dx12lib
{
class CommandList;
class Material;
class ShaderResourceView;
class Texture;
}  // namespace dx12lib

class BasicLightingPSO : public EffectPSO
{
public:
    // Light properties for the pixel shader.
    struct LightProperties
    {
        uint32_t NumPointLights;
        uint32_t NumSpotLights;
        uint32_t NumDirectionalLights;
    };

    // Transformation matrices for the vertex shader.
    struct alignas( 16 ) Matrices
    {
        DirectX::XMMATRIX ModelMatrix;
        DirectX::XMMATRIX ModelViewMatrix;
        DirectX::XMMATRIX InverseTransposeModelViewMatrix;
        DirectX::XMMATRIX ModelViewProjectionMatrix;
    };

    // An enum for root signature parameters.
    // I'm not using scoped enums to avoid the explicit cast that would be required
    // to use these as root indices in the root signature.
    enum RootParameters
    {
        // Vertex shader parameter
        MatricesCB,  // ConstantBuffer<Matrices> MatCB : register(b0);

        // Pixel shader parameters
        MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
        LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );

        PointLights,  // StructuredBuffer<PointLight> PointLights : register( t0 );
        SpotLights,   // StructuredBuffer<SpotLight> SpotLights : register( t1 );

        Textures,  // Texture2D AmbientTexture       : register( t2 );
                   // Texture2D EmissiveTexture : register( t3 );
                   // Texture2D DiffuseTexture : register( t4 );
                   // Texture2D SpecularTexture : register( t5 );
                   // Texture2D SpecularPowerTexture : register( t6 );
                   // Texture2D NormalTexture : register( t7 );
                   // Texture2D BumpTexture : register( t8 );
                   // Texture2D OpacityTexture : register( t9 );
        NumRootParameters
    };

    BasicLightingPSO( std::shared_ptr<dx12lib::Device> device, bool enableLigting, bool enableDecal );
    virtual ~BasicLightingPSO();

    const std::vector<PointLight>& GetPointLights() const
    {
        return m_PointLights;
    }
    void SetPointLights( const std::vector<PointLight>& pointLights )
    {
        m_PointLights = pointLights;
        m_DirtyFlags |= DF_PointLights;
    }

    const std::vector<SpotLight>& GetSpotLights() const
    {
        return m_SpotLights;
    }
    void SetSpotLights( const std::vector<SpotLight>& spotLights )
    {
        m_SpotLights = spotLights;
        m_DirtyFlags |= DF_SpotLights;
    }

    const std::vector<DirectionalLight>& GetDirectionalLights() const
    {
        return m_DirectionalLights;
    }
    void SetDirectionalLights( const std::vector<DirectionalLight>& directionalLights )
    {
        m_DirectionalLights = directionalLights;
        m_DirtyFlags |= DF_DirectionalLights;
    }

    const std::shared_ptr<dx12lib::Material>& GetMaterial() const
    {
        return m_Material;
    }
    void SetMaterial( const std::shared_ptr<dx12lib::Material>& material )
    {
        m_Material = material;
        m_DirtyFlags |= DF_Material;
    }

    // Set matrices.
    void XM_CALLCONV SetWorldMatrix( DirectX::FXMMATRIX worldMatrix )
    {
        m_pAlignedMVP->World = worldMatrix;
        m_DirtyFlags |= DF_Matrices;
    }
    DirectX::XMMATRIX GetWorldMatrix() const
    {
        return m_pAlignedMVP->World;
    }

    void XM_CALLCONV SetViewMatrix( DirectX::FXMMATRIX viewMatrix )
    {
        m_pAlignedMVP->View = viewMatrix;
        m_DirtyFlags |= DF_Matrices;
    }
    DirectX::XMMATRIX GetViewMatrix() const
    {
        return m_pAlignedMVP->View;
    }

    void XM_CALLCONV SetProjectionMatrix( DirectX::FXMMATRIX projectionMatrix )
    {
        m_pAlignedMVP->Projection = projectionMatrix;
        m_DirtyFlags |= DF_Matrices;
    }
    DirectX::XMMATRIX GetProjectionMatrix() const
    {
        return m_pAlignedMVP->Projection;
    }

    // Apply this effect to the rendering pipeline.
    virtual void Apply( dx12lib::CommandList& commandList ) override;

private:
    enum DirtyFlags
    {
        DF_None                = 0,
        DF_PointLights         = ( 1 << 0 ),
        DF_SpotLights          = ( 1 << 1 ),
        DF_DirectionalLights   = ( 1 << 2 ),
        DF_Material            = ( 1 << 3 ),
        DF_Matrices            = ( 1 << 4 ),
        DF_RootSignature       = ( 1 << 5 ),
        DF_PipelineStateObject = ( 1 << 6 ),
        DF_All = DF_PointLights | DF_SpotLights | DF_DirectionalLights | DF_Material | DF_Matrices | DF_RootSignature |
                 DF_PipelineStateObject
    };

    struct alignas( 16 ) MVP
    {
        DirectX::XMMATRIX World;
        DirectX::XMMATRIX View;
        DirectX::XMMATRIX Projection;
    };

    // Helper function to bind a texture to the rendering pipeline.
    inline void BindTexture( dx12lib::CommandList& commandList, uint32_t offset,
                             const std::shared_ptr<dx12lib::Texture>& texture );

    std::vector<PointLight>       m_PointLights;
    std::vector<SpotLight>        m_SpotLights;
    std::vector<DirectionalLight> m_DirectionalLights;

    // The material to apply during rendering.
    std::shared_ptr<dx12lib::Material> m_Material;

    // An SRV used pad unused texture slots.
    std::shared_ptr<dx12lib::ShaderResourceView> m_DefaultSRV;

    // Matrices
    MVP* m_pAlignedMVP;
    // If the command list changes, all parameters need to be rebound.
    dx12lib::CommandList* m_pPreviousCommandList;

    // Which properties need to be bound to the
    uint32_t m_DirtyFlags;

    bool m_EnableLighting;
    bool m_EnableDecal;
};
