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

#include <DirectXMath.h> // For XMFLOAT3, XMFLOAT2

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

    struct alignas(16) Vertex
    {
        DirectX::XMFLOAT3 Position;
        DirectX::XMFLOAT3 Normal;
        DirectX::XMFLOAT3 Tangent;
        DirectX::XMFLOAT3 BiTangent;
        DirectX::XMFLOAT3 TexCoord;

        static const int               InputElementCount = 5;
        static D3D12_INPUT_ELEMENT_DESC InputElements[InputElementCount];
    };

    Mesh()  = default;
    ~Mesh() = default;

    void                          SetVertexBuffer( uint32_t slotID, const std::shared_ptr<VertexBuffer>& vertexBuffer );
    std::shared_ptr<VertexBuffer> GetVertexBuffer( uint32_t slotID ) const;
    const BufferMap&              GetVertexBuffers() const
    {
        return m_VertexBuffers;
    }

    void                         SetIndexBuffer( const std::shared_ptr<IndexBuffer>& indexBuffer );
    std::shared_ptr<IndexBuffer> GetIndexBuffer();

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
};
}  // namespace dx12lib