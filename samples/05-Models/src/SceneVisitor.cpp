#include <SceneVisitor.h>

#include <Camera.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/Material.h>
#include <dx12lib/Mesh.h>
#include <dx12lib/SceneNode.h>

#include <DirectXMath.h>

using namespace dx12lib;
using namespace DirectX;

// Matrices to be sent to the vertex shader.
struct Matrices
{
    XMMATRIX ModelMatrix;
    XMMATRIX ModelViewMatrix;
    XMMATRIX InverseTransposeModelViewMatrix;
    XMMATRIX ModelViewProjectionMatrix;
};

// An enum for root signature parameters.
// I'm not using scoped enums to avoid the explicit cast that would be required
// to use these as root indices in the root signature.
enum RootParameters
{
    MatricesCB,         // ConstantBuffer<Matrices> MatCB : register(b0);
    MaterialCB,         // ConstantBuffer<Material> MaterialCB : register( b0, space1 );
    LightPropertiesCB,  // ConstantBuffer<LightProperties> LightPropertiesCB : register( b1 );
    PointLights,        // StructuredBuffer<PointLight> PointLights : register( t0 );
    SpotLights,         // StructuredBuffer<SpotLight> SpotLights : register( t1 );
    Textures,           // Texture2D DiffuseTexture : register( t2 );
    NumRootParameters
};


SceneVisitor::SceneVisitor( CommandList& commandList, const Camera& camera )
: m_CommandList( commandList )
, m_Camera( camera )
{}

void SceneVisitor::Visit( dx12lib::SceneNode& sceneNode )
{
    auto     model = sceneNode.GetWorldTransform();
    auto     view  = m_Camera.get_ViewMatrix();
    auto     projection = m_Camera.get_ProjectionMatrix();

    Matrices mat;
    mat.ModelMatrix     = model;
    mat.ModelViewMatrix = model * view;
    mat.ModelViewProjectionMatrix = model * view * projection;
    mat.InverseTransposeModelViewMatrix = XMMatrixTranspose( XMMatrixInverse( nullptr, mat.ModelViewMatrix ) );

    m_CommandList.SetGraphicsDynamicConstantBuffer( RootParameters::MatricesCB, mat );
}

void SceneVisitor::Visit( Mesh& mesh )
{
    auto material = mesh.GetMaterial();

    m_CommandList.SetShaderResourceView( RootParameters::Textures, 0, material->GetTexture(Material::TextureType::Diffuse),
                                        D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE );

    mesh.Draw( m_CommandList );
}