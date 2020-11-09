#include <SceneVisitor.h>

#include <Camera.h>

#include <dx12lib/CommandList.h>
#include <dx12lib/IndexBuffer.h>
#include <dx12lib/Mesh.h>

using namespace dx12lib;

SceneVisitor::SceneVisitor( CommandList& commandList, const Camera& camera )
: m_CommandList( commandList )
, m_Camera(camera)
{}

void SceneVisitor::Visit(dx12lib::SceneNode& sceneNode) {

}


void SceneVisitor::Visit( Mesh& mesh )
{
    mesh.Draw( m_CommandList );
}