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

#include <DirectXCollision.h>  // For BoundingBox
#include <DirectXMath.h>       // For XMFLOAT3, XMFLOAT2

#include <d3d12.h>  // For D3D12_INPUT_LAYOUT_DESC, D3D12_INPUT_ELEMENT_DESC

#include <map>     // For std::map
#include <memory>  // For std::shared_ptr

namespace dx12lib
{

class CommandList;
class IndexBuffer;
class Material;
class VertexBuffer;
class Visitor;

class Mesh
{
public:
    using BufferMap = std::map<uint32_t, std::shared_ptr<VertexBuffer>>;

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
     * Get the number if indices in the index buffer.
     * If no index buffer is bound to the mesh, this function returns 0.
     */
    size_t GetIndexCount() const;

    /**
     * Get the number of vertices in the mesh.
     * If this mesh does not have a vertex buffer, the function returns 0.
     */
    size_t GetVertexCount() const;

    void                      SetMaterial( std::shared_ptr<Material> material );
    std::shared_ptr<Material> GetMaterial() const;

    /**
     * Set the AABB bounding volume for the geometry in this mesh.
     */
    void                        SetAABB( const DirectX::BoundingBox& aabb );
    const DirectX::BoundingBox& GetAABB() const;

    /**
     * Draw the mesh to a CommandList.
     *
     * @param commandList The command list to draw to.
     * @param instanceCount The number of instances to draw.
     * @param startInstance The offset added to the instance ID when reading from the instance buffers.
     */
    void Draw( CommandList& commandList, uint32_t instanceCount = 1, uint32_t startInstance = 0 );

    /**
     * Accept a visitor.
     */
    void Accept( Visitor& visitor );

private:
    BufferMap                    m_VertexBuffers;
    std::shared_ptr<IndexBuffer> m_IndexBuffer;
    std::shared_ptr<Material>    m_Material;
    D3D12_PRIMITIVE_TOPOLOGY     m_PrimitiveTopology;
    DirectX::BoundingBox         m_AABB;
};
}  // namespace dx12lib