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
 *  @file SceneVisitor.h
 *  @date November 6, 2020
 *  @author Jeremiah van Oosten
 *
 *  @brief A scene visitor is used to render the meshes in a scene. It uses the Visitor design pattern to iterate the 
 * nodes of a scene.
 */

#include <dx12lib/Visitor.h>

class Camera;
class EffectPSO;

namespace dx12lib
{
class CommandList;
}

class SceneVisitor : public dx12lib::Visitor
{
public:
    /**
     * Constructor for the SceneVisitor.
     * @param commandList The CommandList that is used to render the meshes in the scene.
     * @param camera The camera that is used to render the scene. This is required for setting up the MVP matrix.
     * @param pso The Pipeline state object to use for rendering the geometry in the scene.
     * @param transparent Whether to draw transparent geometry during this pass.
     */
    SceneVisitor( dx12lib::CommandList& commandList, const Camera& camera, EffectPSO& pso, bool transparent );

    // For this sample, we don't need to do anything when visiting the Scene.
    virtual void Visit( dx12lib::Scene& scene ) override;
    // For this sample, we need to set the MVP matrix of the scene node.
    virtual void Visit( dx12lib::SceneNode& sceneNode ) override;
    // When visiting a mesh, the mesh must be rendered.
    virtual void Visit( dx12lib::Mesh& mesh ) override;

private:
    dx12lib::CommandList& m_CommandList;
    const Camera&         m_Camera;
    EffectPSO&     m_LightingPSO;
    bool                  m_TransparentPass;
};