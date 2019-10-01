#pragma once

/*
 *  Copyright(c) 2019 Jeremiah van Oosten
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
 *  @file SceneNode.h
 *  @date May 13, 2019
 *  @author Jeremiah van Oosten
 *
 *  @brief A node in a scene graph.
 */

#include <memory>
#include <string>
#include <map>
#include <vector>

#include <DirectXMath.h>

class Mesh;
class CommandList;

class SceneNode : public std::enable_shared_from_this<SceneNode>
{
public:
    explicit SceneNode(const DirectX::XMMATRIX& localTransform = DirectX::XMMatrixIdentity());
    virtual ~SceneNode();

    /**
     * Assign a name to the scene node so it can be searched for later.
     */
    const std::string& GetName() const;
    void SetName(const std::string& name);

    /**
     * Get the scene nodes local (relative to its parent's transform).
     */
    DirectX::XMMATRIX GetLocalTransform() const;
    void SetLocalTransform(const DirectX::XMMATRIX& localTransform);

    /**
     * Get the inverse of the local transform.
     */
    DirectX::XMMATRIX GetInverseLocalTransform() const;

    /**
     * Get the scene node's world transform (concatenated with its parents
     * world transform).
     */
    DirectX::XMMATRIX GetWorldTransform() const;
    void SetWorldTransform(const DirectX::XMMATRIX& worldTransform);

    /**
     * Get the inverse of the world transform (concatenated with its parent's
     * world transform).
     */
    DirectX::XMMATRIX GetInverseWorldTransform() const;

    /**
     * Add a child node to this scene node.
     * NOTE: Circular references are not checked.
     * A scene node "owns" its children. If the root node
     * is deleted, all of its children are deleted if nothing
     * else is referencing them.
     */
    void AddChild(std::shared_ptr<SceneNode> childNode);
    void RemoveChild(std::shared_ptr<SceneNode> childNode);
    void SetParent(std::weak_ptr<SceneNode> parentNode);

    /**
     * Add a mesh to this scene node.
     */
    void AddMesh(std::shared_ptr<Mesh> mesh);
    void RemoveMesh(std::shared_ptr<Mesh> mesh);

    /**
     * Render this node and all nodes in the scene graph.
     */
    void Render(CommandList& commandList);


protected:
    DirectX::XMMATRIX GetParentWorldTransform() const;

private:
    using NodePtr = std::shared_ptr<SceneNode>;
    using NodeList = std::vector<NodePtr>;
    using NodeNameMap = std::multimap<std::string, NodePtr>;
    using MeshList = std::vector<std::shared_ptr<Mesh>>;

    std::string m_Name;

    // This data must be aligned to a 16-byte boundary. 
    // The only way to guarantee this, is to allocate this
    // structure in aligned memory.
    struct alignas(16) AlignedData
    {
        DirectX::XMMATRIX m_LocalTransform;
        DirectX::XMMATRIX m_InverseTransform;
    } *m_AlignedData;

    std::weak_ptr<SceneNode> m_ParentNode;
    NodeList m_Children;
    NodeNameMap m_ChildrenByName;
    MeshList m_Meshes;
};