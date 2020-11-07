#include <SceneVisitor.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/Mesh.h>

using namespace dx12lib;

SceneVisitor::SceneVisitor( CommandList& commandList )
: m_CommandList( commandList )
{}

void SceneVisitor::Visit( Mesh& mesh )
{

    m_CommandList.SetPrimitiveTopology( mesh.GetPrimitiveTopology() );

    const auto& vertexBuffers = mesh.GetVertexBuffers();
    for ( auto vertexBuffer: vertexBuffers )
    {
        m_CommandList.SetVertexBuffer( vertexBuffer.first, vertexBuffer.second );
    }

    auto indexBuffer = mesh.GetIndexBuffer();
    if ( indexBuffer )
    {
        m_CommandList.SetIndexBuffer( indexBuffer );
        m_CommandList.DrawIndexed( indexBuffer->GetNumIndicies() );
    }
    else
    {
        uint32_t vertexCount = static_cast<uint32_t>( mesh.GetVertexCount() );
        if (vertexCount > 0) {
            m_CommandList.Draw( vertexCount );
        }
    }
}