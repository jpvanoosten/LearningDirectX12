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
    mesh.Draw( m_CommandList );
}