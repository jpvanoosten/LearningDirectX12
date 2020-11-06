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
 *  @file Mesh.h
 *  @date October 24, 2018
 *  @author Jeremiah van Oosten
 *
 *  @brief A mesh class encapsulates the index and vertex buffers for a geometric primitive.
 */

#include <DirectXMath.h>  // For XMFLOAT3, XMFLOAT2

#include <d3d12.h> // For D3D12_INPUT_LAYOUT_DESC, D3D12_INPUT_ELEMENT_DESC

#include <map>     // For std::map
#include <memory>  // For std::shared_ptr

namespace dx12lib
{

class IndexBuffer;
class Material;
class VertexBuffer;
class Visitor;

class Mesh
{
public:
    using BufferMap = std::map<uint32_t, std::shared_ptr<VertexBuffer>>;

    struct alignas( 16 ) Vertex
    {
        Vertex() = default;

        explicit Vertex( const DirectX::XMFLOAT3& position,
                         const DirectX::XMFLOAT3& normal,
                         const DirectX::XMFLOAT3& texCoord,
                         const DirectX::XMFLOAT3& tangent   = { 0, 0, 0 },
                         const DirectX::XMFLOAT3& biTangent = { 0, 0, 0 } )
        : Position( position )
        , Normal( normal )
        , Tangent( tangent )
        , BiTangent( biTangent )
        , TexCoord( texCoord )
        {}

        explicit Vertex( DirectX::FXMVECTOR  position,
                         DirectX::FXMVECTOR normal,
                         DirectX::FXMVECTOR texCoord,
                         DirectX::GXMVECTOR tangent   = { 0, 0, 0, 0 },
                         DirectX::HXMVECTOR biTangent = { 0, 0, 0, 0 } )
        {
            DirectX::XMStoreFloat3( &( this->Position ), position );
            DirectX::XMStoreFloat3( &( this->Normal ), normal );
            DirectX::XMStoreFloat3( &( this->Tangent ), tangent );
            DirectX::XMStoreFloat3( &( this->BiTangent ), biTangent );
            DirectX::XMStoreFloat3( &( this->TexCoord ), texCoord );
        }


        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tangent;
        DirectX::XMFLOAT3 BiTangent;
        DirectX::XMFLOAT3 TexCoord;

        static const D3D12_INPUT_LAYOUT_DESC InputLayout;
    private:
        static const int                InputElementCount = 5;
        static const D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

    Mesh();
    ~Mesh() = default;

    void                     SetPrimitiveTopology( D3D12_PRIMITIVE_TOPOLOGY primitiveToplogy );
    D3D12_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const;

    void                          SetVertexBuffer( uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer );
    std::shared_ptr<VertexBuffer> GetVertexBuffer( uint32_t slotID ) const;
    const BufferMap&              GetVertexBuffers() const
    {
        return m_VertexBuffers;
    }

    void                         SetIndexBuffer( const std::shared_ptr<IndexBuffer>& indexBuffer );
    std::shared_ptr<IndexBuffer> GetIndexBuffer();

    /**
     * Get the number if indicies in the index buffer.
     * If no index buffer is bound to the mesh, this function returns 0.
     */
    size_t GetIndexCount() const;

    /**
     * Get the number of verticies in the mesh.
     * If this mesh does not have a vertex buffer, the function returns 0.
     */
    size_t GetVertexCount() const;

    void                      SetMaterial( std::shared_ptr<Material> material );
    std::shared_ptr<Material> GetMaterial() const;

    /**
     * Accept a visitor.
     */
    void Accept( Visitor& visitor );

private:
    BufferMap                    m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    std::shared_ptr<Material>    m_Material;
    D3D12_PRIMITIVE_TOPOLOGY     m_PrimitiveTopology;
    size_t                       m_VertexCount;
};
}  // namespace dx12lib